#include "edbus_private.h"
#include "edbus_private_types.h"
#include <dbus/dbus.h>

/* TODO: mempool of EDBus_Signal_Handler */

#define EDBUS_SIGNAL_HANDLER_CHECK(handler)                        \
  do                                                               \
    {                                                              \
       EINA_SAFETY_ON_NULL_RETURN(handler);                        \
       if (!EINA_MAGIC_CHECK(handler, EDBUS_SIGNAL_HANDLER_MAGIC)) \
         {                                                         \
            EINA_MAGIC_FAIL(handler, EDBUS_SIGNAL_HANDLER_MAGIC);  \
            return;                                                \
         }                                                         \
    }                                                              \
  while (0)

#define EDBUS_SIGNAL_HANDLER_CHECK_RETVAL(handler, retval)         \
  do                                                               \
    {                                                              \
       EINA_SAFETY_ON_NULL_RETURN_VAL(handler, retval);            \
       if (!EINA_MAGIC_CHECK(handler, EDBUS_SIGNAL_HANDLER_MAGIC)) \
         {                                                         \
            EINA_MAGIC_FAIL(handler, EDBUS_SIGNAL_HANDLER_MAGIC);  \
            return retval;                                         \
         }                                                         \
    }                                                              \
  while (0)

static void _edbus_signal_handler_del(EDBus_Signal_Handler *handler);
static void _edbus_signal_handler_clean(EDBus_Signal_Handler *handler);

Eina_Bool
edbus_signal_handler_init(void)
{
   return EINA_TRUE;
}

void
edbus_signal_handler_shutdown(void)
{
}

static void
_match_append(Eina_Strbuf *match, const char *key, const char *value)
{
   if (!value) return;

   if ((eina_strbuf_length_get(match) + strlen(",=''") + strlen(key) + strlen(value))
       >= DBUS_MAXIMUM_MATCH_RULE_LENGTH)
     {
        ERR("cannot add match %s='%s' to %s: too long!", key, value,
            eina_strbuf_string_get(match));
        return;
     }

   eina_strbuf_append_printf(match, ",%s='%s'", key, value);
}

static int
_sort_arg(const void *d1, const void *d2)
{
   const Signal_Argument *arg1, *arg2;
   arg1 = d1;
   arg2 = d2;
   return arg1->index - arg2->index;
}

#define ARGX "arg"
EAPI Eina_Bool
edbus_signal_handler_match_extra_vset(EDBus_Signal_Handler *sh, va_list ap)
{
   const char *key = NULL, *read;
   DBusError err;

   EDBUS_SIGNAL_HANDLER_CHECK_RETVAL(sh, EINA_FALSE);

   dbus_error_init(&err);
   dbus_bus_remove_match(sh->conn->dbus_conn,
                         eina_strbuf_string_get(sh->match), &err);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(dbus_error_is_set(&err), EINA_FALSE);

   for (read = va_arg(ap, char *); read; read = va_arg(ap, char *))
     {
        Signal_Argument *arg;

        if (!key)
          {
             key = read;
             continue;
          }
        arg = calloc(1, sizeof(Signal_Argument));
        EINA_SAFETY_ON_NULL_GOTO(arg, error);
        if (!strncmp(key, ARGX, strlen(ARGX)))
          {
             int id = atoi(key + strlen(ARGX));
             arg->index = (unsigned short) id;
             arg->value = eina_stringshare_add(read);
             sh->args = eina_inlist_sorted_state_insert(sh->args,
                                                        EINA_INLIST_GET(arg),
                                                        _sort_arg,
                                                        sh->state_args);
             _match_append(sh->match, key, read);
          }
        else
          {
             ERR("%s not supported", key);
             free(arg);
          }
        key = NULL;
     }

   dbus_error_init(&err);
   dbus_bus_add_match(sh->conn->dbus_conn,
                      eina_strbuf_string_get(sh->match), &err);
   if (!dbus_error_is_set(&err))
     return EINA_TRUE;

   ERR("Error setting new match.");
   return EINA_FALSE;

error:
   dbus_error_init(&err);
   dbus_bus_add_match(sh->conn->dbus_conn,
                      eina_strbuf_string_get(sh->match), &err);
   if (dbus_error_is_set(&err))
     ERR("Error setting partial extra arguments.");
   return EINA_FALSE;
}

EAPI Eina_Bool
edbus_signal_handler_match_extra_set(EDBus_Signal_Handler *sh, ...)
{
   Eina_Bool ret;
   va_list ap;

   va_start(ap, sh);
   ret = edbus_signal_handler_match_extra_vset(sh, ap);
   va_end(ap);
   return ret;
}

static void _on_handler_of_conn_free(void *data, const void *dead_pointer);

static void
_on_connection_free(void *data, const void *dead_pointer)
{
   EDBus_Signal_Handler *sh = data;
   edbus_signal_handler_free_cb_del(sh, _on_handler_of_conn_free, sh->conn);
   edbus_signal_handler_del(sh);
}

static void
_on_handler_of_conn_free(void *data, const void *dead_pointer)
{
   EDBus_Connection *conn = data;
   edbus_connection_free_cb_del(conn, _on_connection_free, dead_pointer);
}

EAPI EDBus_Signal_Handler *
edbus_signal_handler_add(EDBus_Connection *conn, const char *sender, const char *path, const char *interface, const char *member, EDBus_Signal_Cb cb, const void *cb_data)
{
   EDBus_Signal_Handler *sh;
   sh = _edbus_signal_handler_add(conn, sender, path, interface, member, cb, cb_data);
   EINA_SAFETY_ON_NULL_RETURN_VAL(sh, NULL);
   edbus_connection_free_cb_add(conn, _on_connection_free, sh);
   edbus_signal_handler_free_cb_add(sh, _on_handler_of_conn_free, conn);
   return sh;
}

EDBus_Signal_Handler *
_edbus_signal_handler_add(EDBus_Connection *conn, const char *sender, const char *path, const char *interface, const char *member, EDBus_Signal_Cb cb, const void *cb_data)
{
   EDBus_Signal_Handler *sh;
   Eina_Strbuf *match;
   DBusError err;

   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cb, NULL);
   DBG("conn=%p, sender=%s, path=%s, interface=%s, member=%s, cb=%p %p",
       conn, sender, path, interface, member, cb, cb_data);

   sh = calloc(1, sizeof(EDBus_Signal_Handler));
   EINA_SAFETY_ON_NULL_RETURN_VAL(sh, NULL);

   match = eina_strbuf_new();
   EINA_SAFETY_ON_NULL_GOTO(match, cleanup_create_strbuf);
   eina_strbuf_append(match, "type='signal'");
   _match_append(match, "sender", sender);
   _match_append(match, "path", path);
   _match_append(match, "interface", interface);
   _match_append(match, "member", member);

   dbus_error_init(&err);
   dbus_bus_add_match(conn->dbus_conn, eina_strbuf_string_get(match), &err);
   if (dbus_error_is_set(&err)) goto cleanup;

   if (sender)
     {
        sh->bus = edbus_connection_name_get(conn, sender);
        if (!sh->bus) goto cleanup;
        edbus_connection_name_ref(sh->bus);
     }

   sh->cb = cb;
   sh->cb_data = cb_data;
   sh->conn = conn;
   sh->interface = eina_stringshare_add(interface);
   sh->member = eina_stringshare_add(member);
   sh->path = eina_stringshare_add(path);
   sh->sender = eina_stringshare_add(sender);
   sh->match = match;
   sh->refcount = 1;
   sh->dangling = EINA_FALSE;
   sh->state_args = eina_inlist_sorted_state_new();
   EINA_MAGIC_SET(sh, EDBUS_SIGNAL_HANDLER_MAGIC);

   edbus_connection_signal_handler_add(conn, sh);
   return sh;

cleanup:
   eina_strbuf_free(match);
cleanup_create_strbuf:
   free(sh);

   return NULL;
}

static void
_edbus_signal_handler_clean(EDBus_Signal_Handler *handler)
{
   DBusError err;

   if (handler->dangling) return;

   dbus_error_init(&err);
   dbus_bus_remove_match(handler->conn->dbus_conn,
                         eina_strbuf_string_get(handler->match), &err);
   handler->dangling = EINA_TRUE;
}

static void
_edbus_signal_handler_del(EDBus_Signal_Handler *handler)
{
   Eina_Inlist *list;
   Signal_Argument *arg;
   DBG("handler %p, refcount=%d, conn=%p %s",
       handler, handler->refcount, handler->conn, handler->sender);
   edbus_cbs_free_dispatch(&(handler->cbs_free), handler);
   edbus_connection_signal_handler_del(handler->conn, handler);
   EINA_MAGIC_SET(handler, EINA_MAGIC_NONE);

   /* after cbs_free dispatch these shouldn't exit, error if they do */

   eina_stringshare_del(handler->sender);
   eina_stringshare_del(handler->path);
   eina_stringshare_del(handler->interface);
   eina_stringshare_del(handler->member);
   eina_strbuf_free(handler->match);
   EINA_INLIST_FOREACH_SAFE(handler->args, list, arg)
     {
        eina_stringshare_del(arg->value);
        free(arg);
     }
   eina_inlist_sorted_state_free(handler->state_args);

   if (handler->bus)
     edbus_connection_name_unref(handler->conn, handler->bus);
   free(handler);
}

EAPI EDBus_Signal_Handler *
edbus_signal_handler_ref(EDBus_Signal_Handler *handler)
{
   EDBUS_SIGNAL_HANDLER_CHECK_RETVAL(handler, NULL);
   DBG("handler=%p, pre-refcount=%d, match=%s",
       handler, handler->refcount, eina_strbuf_string_get(handler->match));
   handler->refcount++;
   return handler;
}

EAPI void
edbus_signal_handler_unref(EDBus_Signal_Handler *handler)
{
   EDBUS_SIGNAL_HANDLER_CHECK(handler);
   DBG("handler=%p, pre-refcount=%d, match=%s",
       handler, handler->refcount, eina_strbuf_string_get(handler->match));
   handler->refcount--;
   if (handler->refcount > 0) return;

   _edbus_signal_handler_clean(handler);
   _edbus_signal_handler_del(handler);
}

EAPI void
edbus_signal_handler_del(EDBus_Signal_Handler *handler)
{
   EDBUS_SIGNAL_HANDLER_CHECK(handler);
   _edbus_signal_handler_clean(handler);
   edbus_signal_handler_unref(handler);
}

EAPI void
edbus_signal_handler_free_cb_add(EDBus_Signal_Handler *handler, EDBus_Free_Cb cb, const void *data)
{
   EDBUS_SIGNAL_HANDLER_CHECK(handler);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   handler->cbs_free = edbus_cbs_free_add(handler->cbs_free, cb, data);
}

EAPI void
edbus_signal_handler_free_cb_del(EDBus_Signal_Handler *handler, EDBus_Free_Cb cb, const void *data)
{
   EDBUS_SIGNAL_HANDLER_CHECK(handler);
   EINA_SAFETY_ON_NULL_RETURN(cb);
   handler->cbs_free = edbus_cbs_free_del(handler->cbs_free, cb, data);
}

EAPI const char *
edbus_signal_handler_sender_get(const EDBus_Signal_Handler *handler)
{
   EDBUS_SIGNAL_HANDLER_CHECK_RETVAL(handler, NULL);
   return handler->sender;
}

EAPI const char *
edbus_signal_handler_path_get(const EDBus_Signal_Handler *handler)
{
   EDBUS_SIGNAL_HANDLER_CHECK_RETVAL(handler, NULL);
   return handler->path;
}

EAPI const char *
edbus_signal_handler_interface_get(const EDBus_Signal_Handler *handler)
{
   EDBUS_SIGNAL_HANDLER_CHECK_RETVAL(handler, NULL);
   return handler->interface;
}

EAPI const char *
edbus_signal_handler_member_get(const EDBus_Signal_Handler *handler)
{
   EDBUS_SIGNAL_HANDLER_CHECK_RETVAL(handler, NULL);
   return handler->member;
}

EAPI const char *
edbus_signal_handler_match_get(const EDBus_Signal_Handler *handler)
{
   EDBUS_SIGNAL_HANDLER_CHECK_RETVAL(handler, NULL);
   return eina_strbuf_string_get(handler->match);
}

EAPI EDBus_Connection *
edbus_signal_handler_connection_get(const EDBus_Signal_Handler *handler)
{
   EDBUS_SIGNAL_HANDLER_CHECK_RETVAL(handler, NULL);
   return handler->conn;
}

