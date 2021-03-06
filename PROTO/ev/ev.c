/*
 * Copyright 2011-2012 Mike Blumenkrantz <michael.blumenkrantz@gmail.com>
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

#include <Eio.h>
#include <Elementary.h>

#ifndef __UNUSED__
# define __UNUSED__ __attribute__((unused))
#endif

#define DBG(...)            EINA_LOG_DOM_DBG(_ev_log_dom, __VA_ARGS__)
#define INF(...)            EINA_LOG_DOM_INFO(_ev_log_dom, __VA_ARGS__)
#define WRN(...)            EINA_LOG_DOM_WARN(_ev_log_dom, __VA_ARGS__)
#define ERR(...)            EINA_LOG_DOM_ERR(_ev_log_dom, __VA_ARGS__)
#define CRI(...)            EINA_LOG_DOM_CRIT(_ev_log_dom, __VA_ARGS__)

static int _ev_log_dom = -1;
static Elm_Genlist_Item_Class itc;
static Evas_Object *img, *list = NULL;

static void stat_done(void *data, Eio_File *handler, const Eina_Stat *st);

static void
dummy()
{}

static void
_close(void *data __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   ecore_main_loop_quit();
}

static void
_del(void *data, Evas_Object *obj __UNUSED__)
{
   eina_stringshare_del(data);
}

static char *
_text(void *data, Evas_Object *obj __UNUSED__, const char *part __UNUSED__)
{
   return strdup(data);
}

static void
_title(Evas_Object *win)
{
   Evas_Coord ww, wh;
   char buf[8192];
   const char *f, *s;

   evas_object_geometry_get(img, NULL, NULL, &ww, &wh);
   elm_image_file_get(img, &f, NULL);
   s = ecore_file_file_get(f);
   snprintf(buf, sizeof(buf), "%s (%ux%u)", s, ww, wh);
   elm_win_title_set(win, buf);
}

static void
_pick(void *data __UNUSED__, Evas_Object *obj __UNUSED__, Elm_Object_Item *ev)
{
   const char *file, *f, *p;
   Evas_Object *win, *ic;
   Evas_Coord w, h;

   DBG("pick");
   elm_image_file_get(img, &f, &p);
   file = elm_object_item_data_get(ev);
   if (f && (!strcmp(file, f))) return;

   if (!evas_object_image_extension_can_load_get(file))
     {
        ERR("Image loader for %s not detected", file);
        return;
     }
   elm_image_file_set(img, file, NULL);
   win = elm_object_parent_widget_get(img);
   ic = elm_image_object_get(img);
   evas_object_image_size_get(ic, &w, &h);
   evas_object_resize(win, w, h);
   _title(win);
}

static void
_key(void *data __UNUSED__, Evas *e __UNUSED__, Evas_Object *obj, Evas_Event_Key_Down *key)
{
   Elm_Object_Item *it;
   if ((!strcmp(key->keyname, "space")) || (!strcmp(key->keyname, "Right")) ||
       ((!strcmp(key->keyname, "KP_Right")) && (!key->string)))
     {
        it = elm_genlist_selected_item_get(list);
        DBG("current: %p", it);
        it = elm_genlist_item_next_get(it);
        if (!it) it = elm_genlist_first_item_get(list);
        DBG("next: %p", it);
        elm_genlist_item_selected_set(it, EINA_TRUE);
        elm_genlist_item_bring_in(it, ELM_GENLIST_ITEM_SCROLLTO_IN);
        _pick(NULL, NULL, it);
     }
   else if ((!strcmp(key->keyname, "Left")) ||
           ((!strcmp(key->keyname, "KP_Left")) && (!key->string)))
     {
        it = elm_genlist_selected_item_get(list);
        DBG("current: %p", it);
        it = elm_genlist_item_prev_get(it);
        if (!it) it = elm_genlist_last_item_get(list);
        DBG("next: %p", it);
        elm_genlist_item_selected_set(it, EINA_TRUE);
        elm_genlist_item_bring_in(it, ELM_GENLIST_ITEM_SCROLLTO_IN);
        _pick(NULL, NULL, it);
     }
   else if ((!strcmp(key->keyname, "Return")) || (!strcmp(key->keyname, "KP_Enter")))
     {
        if (obj == elm_object_parent_widget_get(img)) return;
        it = elm_genlist_selected_item_get(list);
        if (!it) return;
        elm_genlist_item_bring_in(it, ELM_GENLIST_ITEM_SCROLLTO_IN);
        _pick(NULL, NULL, it);
     }
   else if (key->keyname[0] == 'q')
     ecore_main_loop_quit();
}

static void
_show(void *data, Evas_Object *obj __UNUSED__, void *ev __UNUSED__)
{
   DBG("show");
   if (evas_object_visible_get(data))
     evas_object_hide(data);
   else
     evas_object_show(data);
}

static int
add_sort(void *a, void *b)
{
   const char *x, *y;

   x = elm_object_item_data_get(a);
   x = ecore_file_file_get(x);
   y = elm_object_item_data_get(b);
   y = ecore_file_file_get(y);
   return strcmp(x, y);
}

static void
add_single(const char *file)
{
   static Eina_Bool first = EINA_TRUE;

   DBG("NEW: %s", file);
   elm_genlist_item_sorted_insert(list, &itc, file, NULL, ELM_GENLIST_ITEM_NONE, (Eina_Compare_Cb)add_sort, NULL, NULL /*data*/);
   elm_genlist_item_selected_set(elm_genlist_first_item_get(list), EINA_TRUE);
   if (first)
     {
        _pick(NULL, NULL, elm_genlist_selected_item_get(list));
        first = EINA_FALSE;
     }
}

static void
ls_done(void *data __UNUSED__, Eio_File *handler __UNUSED__, const char *file)
{
   eio_file_direct_stat(file, stat_done, dummy, eina_stringshare_add(file));
}

static Eina_Bool
ls_filter(void *data __UNUSED__, Eio_File *handler __UNUSED__, const char *file)
{
   struct stat st;
   if (stat(file, &st)) return EINA_FALSE;
   return S_ISREG(st.st_mode);
}

static void
add_dir(const char *dir)
{
   eio_file_ls(dir, ls_filter, ls_done, dummy, dummy, NULL);
}

static void
stat_done(void *data, Eio_File *handler __UNUSED__, const Eina_Stat *st)
{
   if (S_ISREG(st->mode))
     {
        add_single(data);
        return;
     }
   if (S_ISDIR(st->mode))
     add_dir(data);
   else
     ERR("wtf is this %s", (char*)data);
   eina_stringshare_del(data);
}

static void
load_stuff(int argc, char **argv)
{
   int x;

   for (x = 1; x < argc; x++)
     eio_file_direct_stat(argv[x], stat_done, dummy, eina_stringshare_add(argv[x]));
}

int
main(int argc, char *argv[])
{
   Evas_Object *listwin, *win, *bg, *box;

   itc.item_style     = "default";
   itc.func.text_get = _text;
   itc.func.content_get  = NULL;
   itc.func.state_get = NULL;
   itc.func.del       = _del;

   elm_init(argc, argv);
   ecore_app_args_set(argc, (const char**)argv);

   _ev_log_dom = eina_log_domain_register("ev", EINA_COLOR_HIGH EINA_COLOR_CYAN);
   if (argc == 1)
     {
        printf("Usage: %s filename filename directory fileglob*\n", argv[0]);
        exit(1);
     }
   /* eina_log_domain_level_set("ev", EINA_LOG_LEVEL_DBG); */

   win = elm_win_add(NULL, NULL, ELM_WIN_BASIC);
   elm_win_autodel_set(win, EINA_TRUE);

   bg = elm_bg_add(win);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   img = elm_icon_add(win);
   evas_object_size_hint_weight_set(img, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(img, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_win_resize_object_add(win, img);
   elm_image_animated_set(img, EINA_TRUE);
   elm_image_aspect_fixed_set(img, EINA_TRUE);
   elm_image_fill_outside_set(img, EINA_FALSE);
   evas_object_show(img);
   evas_object_show(win);

   elm_win_screen_constrain_set(win, EINA_TRUE);

   listwin = elm_win_add(NULL, "ev", ELM_WIN_BASIC);
   elm_win_autodel_set(listwin, EINA_TRUE);

   bg = elm_bg_add(listwin);
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(listwin, bg);
   evas_object_show(bg);

   box = elm_box_add(listwin);
   evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(listwin, box);
   evas_object_show(box);

   list = elm_genlist_add(listwin);
   evas_object_size_hint_weight_set(list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(list, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(box, list);
   evas_object_show(list);
   evas_object_smart_callback_add(list, "clicked,double", (Evas_Smart_Cb)_pick, NULL);
   evas_object_event_callback_add(win, EVAS_CALLBACK_KEY_DOWN, (Evas_Object_Event_Cb)_key, NULL);
   1 | evas_object_key_grab(win, "Left", 0, 0, 1);
   1 | evas_object_key_grab(win, "Right", 0, 0, 1);
   1 | evas_object_key_grab(win, "KP_Left", 0, 0, 1);
   1 | evas_object_key_grab(win, "KP_Right", 0, 0, 1);
   1 | evas_object_key_grab(win, "space", 0, 0, 1);
   1 | evas_object_key_grab(win, "q", 0, 0, 1);
   1 | evas_object_key_grab(listwin, "space", 0, 0, 1);
   1 | evas_object_key_grab(listwin, "q", 0, 0, 1);
   evas_object_event_callback_add(listwin, EVAS_CALLBACK_KEY_DOWN, (Evas_Object_Event_Cb)_key, NULL);
   evas_object_smart_callback_add(img, "clicked", (Evas_Smart_Cb)_show, listwin);
   evas_object_smart_callback_add(win, "delete,request", _close, NULL);

   load_stuff(argc, argv);

   evas_object_resize(listwin, 450, 350);

   elm_run();
   elm_shutdown();

   return 0;
}
