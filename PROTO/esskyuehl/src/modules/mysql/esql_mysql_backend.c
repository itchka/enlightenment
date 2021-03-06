/*
 * Copyright 2011, 2012 Mike Blumenkrantz <michael.blumenkrantz@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "esql_module.h"
#include "mysac/mysac.h"
#include <unistd.h>

#define ESQL_MYSAC_SWITCH_RET(X) \
   switch (X) \
     { \
      case 0: \
        return 0; \
      case MYERR_WANT_READ: \
        return ECORE_FD_READ; \
      case MYERR_WANT_WRITE: \
        return ECORE_FD_WRITE; \
      default: \
        break; \
     } \
   return ECORE_FD_ERROR

static const char *esql_mysac_error_get(Esql *e);
static void esql_mysac_disconnect(Esql *e);
static int esql_mysac_fd_get(Esql *e);
static int esql_mysac_connect(Esql *e);
static void esql_mysac_database_set(Esql *e, const char *database_name);
static int esql_mysac_io(Esql *e);
static void esql_mysac_setup(Esql *e, const char *addr, const char *user, const char *passwd);
static void esql_mysac_query(Esql *e, const char *query, unsigned int len __UNUSED__);
static void esql_mysac_res_free(Esql_Res *res);
static void esql_mysac_res(Esql_Res *res);
static char *esql_mysac_escape(Esql *e, unsigned int *len, const char *fmt, va_list args);
static void esql_mysac_row_init(Esql_Row *r, MYSAC_ROW *row);
static void esql_mysac_free(Esql *e);


static void
esql_module_setup_cb(MYSAC_RES *re, int col, Eina_Value_Struct_Member *m)
{
   m->name = eina_stringshare_add(re->cols[col].name);

   switch (re->cols[col].type)
     {
      case MYSQL_TYPE_TIME:
      case MYSQL_TYPE_DOUBLE:
        m->type = EINA_VALUE_TYPE_DOUBLE;
        break;

      case MYSQL_TYPE_YEAR:
      case MYSQL_TYPE_TIMESTAMP:
      case MYSQL_TYPE_DATETIME:
      case MYSQL_TYPE_DATE:
        m->type = EINA_VALUE_TYPE_TIMESTAMP;
        break;

      case MYSQL_TYPE_STRING:
      case MYSQL_TYPE_VARCHAR:
      case MYSQL_TYPE_VAR_STRING:
        m->type = EINA_VALUE_TYPE_STRING;
        break;

      case MYSQL_TYPE_TINY:
        m->type = EINA_VALUE_TYPE_CHAR;
        break;

      case MYSQL_TYPE_SHORT:
        m->type = EINA_VALUE_TYPE_SHORT;
        break;

      case MYSQL_TYPE_LONG:
      case MYSQL_TYPE_INT24:
        m->type = EINA_VALUE_TYPE_LONG;
        break;

      case MYSQL_TYPE_DECIMAL:
      case MYSQL_TYPE_NEWDECIMAL:
      case MYSQL_TYPE_LONGLONG:
        m->type = EINA_VALUE_TYPE_INT64;
        break;

      case MYSQL_TYPE_FLOAT:
        m->type = EINA_VALUE_TYPE_FLOAT;
        break;

      default:
        m->type = EINA_VALUE_TYPE_BLOB;
     }
}

static const char *
esql_mysac_error_get(Esql *e)
{
   switch (mysac_errno(e->backend.db))
     {
      case MYERR_WANT_READ:
      case MYERR_WANT_WRITE:
      case 0:
        return NULL;
      default:
        break;
     }
   return mysac_advance_error(e->backend.db);
}

static void
esql_mysac_disconnect(Esql *e)
{
   MYSAC *m;
   char *buf;
   const char *addr;
   const char *login;
   const char *password;
   size_t size;

   /* mysac is very complicated :/ */
   m = e->backend.db;
   if (m->fd >= 0) close(m->fd);
   buf = m->buf;
   size = m->bufsize;
   addr = m->addr;
   login = m->login;
   password = m->password;
   /* avoid allocating new mem */
   memset(m, 0, sizeof(MYSAC));
   memset(buf, 0, size);
   m->buf = buf;
   m->bufsize = size;
   m->free_it = 1;
   m->qst = MYSAC_START;
   mysac_setup(m, addr, login, password, e->database, 0);
}

static int
esql_mysac_fd_get(Esql *e)
{
   return mysac_get_fd(e->backend.db);
}

static int
esql_mysac_connect(Esql *e)
{
   ESQL_MYSAC_SWITCH_RET(mysac_connect(e->backend.db));
}

static void
esql_mysac_database_set(Esql *e, const char *database_name)
{
   mysac_set_database(e->backend.db, database_name);
}

static int
esql_mysac_io(Esql *e)
{
   ESQL_MYSAC_SWITCH_RET(mysac_io(e->backend.db));
}

static void
esql_mysac_setup(Esql *e, const char *addr, const char *user, const char *passwd)
{
   mysac_setup(e->backend.db, eina_stringshare_add(addr), eina_stringshare_add(user), eina_stringshare_add(passwd), e->database, 0);
}

static void
esql_mysac_query(Esql *e, const char *query, unsigned int len __UNUSED__)
{
   MYSAC_RES *res;

   res = mysac_new_res(2048, 1);
   EINA_SAFETY_ON_NULL_RETURN(res);
   mysac_s_set_query(e->backend.db, res, query);
}

static void
esql_mysac_res_free(Esql_Res *res)
{
   mysac_free_res(res->backend.res);
}

static void
esql_mysac_res(Esql_Res *res)
{
   MYSAC_ROW *row;
   MYSAC_RES *re;
   MYSAC *m;
   Esql_Row *r;

   re = res->backend.res = mysac_get_res(res->e->backend.db);
   if (!re) return;
   m = res->e->backend.db;
   res->desc = esql_module_desc_get(re->nb_cols, (Esql_Module_Setup_Cb)esql_module_setup_cb, res);
   mysac_first_row(re);
   row = mysac_fetch_row(re);
   if (!row) /* must be insert/update/etc */
     {
        res->affected = m->affected_rows;
        res->id = m->insert_id;
        return;
     }
   res->row_count = mysac_num_rows(re);
   do
     {
        r = esql_row_calloc(1);
        EINA_SAFETY_ON_NULL_RETURN(r);
        r->res = res;
        esql_mysac_row_init(r, row);
        res->rows = eina_inlist_append(res->rows, EINA_INLIST_GET(r));
     } while ((row = mysac_fetch_row(re)));
}

static char *
esql_mysac_escape(Esql *e, unsigned int *len, const char *fmt, va_list args)
{
   MYSAC *m;
   Eina_Bool backslashes = EINA_TRUE;
   char *ret;

   m = e->backend.db;
   if (m->status & 512) /* SERVER_STATUS_NO_BACKSLASH_ESCAPES */
     backslashes = EINA_FALSE;
   ret = esql_query_escape(backslashes, len, fmt, args);
   if (*len > m->bufsize - 5) /* mysac is dumb and uses a user-allocated buffer, so we have to manually resize it */
     {
        char *tmp;

        tmp = realloc(m->buf, (*len) * 2);
        if (!tmp) /* we're so fucked */
          {
             free(ret);
             ERR("Alloc! We're in trouble!");
             return NULL;
          }
        m->buf = tmp;
        m->bufsize = (*len) * 2;
     }
   return ret;
}

static void
esql_mysac_row_init(Esql_Row *r, MYSAC_ROW *row)
{
   MYSAC_RES *res;
   MYSAC_ROWS *rows;
   struct mysac_list_head *l;
   Eina_Value *sval;
   unsigned int i, cols;

   res = r->res->backend.res;
   rows = res->cr;
   l = res->data.next;
   cols = res->nb_cols;

   sval = &(r->value);
   eina_value_struct_setup(sval, r->res->desc);
   for (i = 0; i < cols; i++, l = l->next, rows = mysac_container_of(l, MYSAC_ROWS, link))
     {
        Eina_Value val;
        const Eina_Value_Struct_Member *m = r->res->desc->members + i;

        switch (res->cols[i].type)
          {
           case MYSQL_TYPE_TIME:
             eina_value_setup(&val, EINA_VALUE_TYPE_DOUBLE);
             eina_value_set(&val, (double)row[i].tv.tv_sec + (double)((double)row[i].tv.tv_usec / (double) 1000000));
             break;

           case MYSQL_TYPE_YEAR:
           case MYSQL_TYPE_TIMESTAMP:
           case MYSQL_TYPE_DATETIME:
           case MYSQL_TYPE_DATE:
             eina_value_setup(&val, EINA_VALUE_TYPE_TIMESTAMP);
             eina_value_set(&val, (long)mktime(row[i].tm));
             break;

           case MYSQL_TYPE_STRING:
           case MYSQL_TYPE_VARCHAR:
           case MYSQL_TYPE_VAR_STRING:
             eina_value_setup(&val, EINA_VALUE_TYPE_STRING);
             eina_value_set(&val, row[i].string);
             //cell->len = rows->lengths[i];
             break;

           case MYSQL_TYPE_TINY_BLOB:
           case MYSQL_TYPE_MEDIUM_BLOB:
           case MYSQL_TYPE_LONG_BLOB:
           case MYSQL_TYPE_BLOB:
             {
                Eina_Value_Blob blob;

                blob.ops = NULL;
                blob.memory = row[i].string;
                blob.size = rows->lengths[i];;
                eina_value_setup(&val, EINA_VALUE_TYPE_BLOB);
                eina_value_set(&val, &blob);
                break;
             }

           case MYSQL_TYPE_TINY:
             eina_value_setup(&val, EINA_VALUE_TYPE_CHAR);
             eina_value_set(&val, row[i].stiny);
             break;

           case MYSQL_TYPE_SHORT:
             eina_value_setup(&val, EINA_VALUE_TYPE_SHORT);
             eina_value_set(&val, row[i].ssmall);
             break;

           case MYSQL_TYPE_LONG:
           case MYSQL_TYPE_INT24:
             eina_value_setup(&val, EINA_VALUE_TYPE_LONG);
             eina_value_set(&val, row[i].sint);
             break;

           case MYSQL_TYPE_DECIMAL:
           case MYSQL_TYPE_NEWDECIMAL:
           case MYSQL_TYPE_LONGLONG:
             eina_value_setup(&val, EINA_VALUE_TYPE_INT64);
             eina_value_set(&val, row[i].sbigint);
             break;

           case MYSQL_TYPE_FLOAT:
             eina_value_setup(&val, EINA_VALUE_TYPE_FLOAT);
             eina_value_set(&val, row[i].mfloat);
             break;

           case MYSQL_TYPE_DOUBLE:
             eina_value_setup(&val, EINA_VALUE_TYPE_DOUBLE);
             eina_value_set(&val, row[i].mdouble);
             break;

           default:
             ERR("FIXME: Got unknown column type %u!", res->cols[i].type);
             break;
          }
        eina_value_struct_member_value_set(sval, m, &val);
        eina_value_flush(&val);
     }
}

static void
esql_mysac_free(Esql *e)
{
   MYSAC *m;

   esql_mysac_disconnect(e);
   e->backend.free = NULL;
   m = e->backend.db;
   eina_stringshare_del(m->addr);
   eina_stringshare_del(m->login);
   eina_stringshare_del(m->password);
   free(m->buf);
   free(m);
}

static void
esql_mysac_init(Esql *e)
{
   INFO("Esql type for %p set to MySQL", e);
   e->type = ESQL_TYPE_MYSQL;
   e->backend.connect = esql_mysac_connect;
   e->backend.disconnect = esql_mysac_disconnect;
   e->backend.error_get = esql_mysac_error_get;
   e->backend.setup = esql_mysac_setup;
   e->backend.database_set = esql_mysac_database_set;
   e->backend.io = esql_mysac_io;
   e->backend.fd_get = esql_mysac_fd_get;
   e->backend.escape = esql_mysac_escape;
   e->backend.query = esql_mysac_query;
   e->backend.res = esql_mysac_res;
   e->backend.res_free = esql_mysac_res_free;
   e->backend.free = esql_mysac_free;

   e->backend.db = mysac_new(4096);
   EINA_SAFETY_ON_NULL_RETURN(e->backend.db);
}

EAPI Esql_Type
esql_module_init(Esql *e)
{
   if (e) esql_mysac_init(e);
   return ESQL_TYPE_MYSQL;
}
