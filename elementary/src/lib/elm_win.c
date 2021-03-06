#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"

EAPI Eo_Op ELM_OBJ_WIN_BASE_ID = EO_NOOP;

#define MY_CLASS ELM_OBJ_WIN_CLASS

#define MY_CLASS_NAME "elm_win"

static const Elm_Win_Trap *trap = NULL;

#define TRAP(sd, name, ...)                                             \
  do                                                                    \
    {                                                                   \
       if ((!trap) || (!trap->name) ||                                  \
           ((trap->name) &&                                             \
            (trap->name(sd->trap_data, sd->obj, ## __VA_ARGS__)))) \
         ecore_evas_##name(sd->ee, ##__VA_ARGS__);                      \
    }                                                                   \
  while (0)

#define ELM_WIN_DATA_GET(o, sd) \
  Elm_Win_Smart_Data * sd = eo_data_get(o, MY_CLASS)

#define ELM_WIN_DATA_GET_OR_RETURN(o, ptr)           \
  ELM_WIN_DATA_GET(o, ptr);                          \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return;                                       \
    }

#define ELM_WIN_DATA_GET_OR_RETURN_VAL(o, ptr, val)  \
  ELM_WIN_DATA_GET(o, ptr);                          \
  if (!ptr)                                          \
    {                                                \
       CRITICAL("No widget data for object %p (%s)", \
                o, evas_object_type_get(o));         \
       return val;                                   \
    }

#define ELM_WIN_CHECK(obj)                                             \
  if (!obj || !eo_isa(obj, MY_CLASS)) \
    return

#define ENGINE_GET() (_elm_preferred_engine ? _elm_preferred_engine : (_elm_config->engine ? _elm_config->engine : ""))
#define ENGINE_COMPARE(name) (!strcmp(ENGINE_GET(), name))
#define EE_ENGINE_COMPARE(ee, name) (!strcmp(ecore_evas_engine_name_get(ee), name))

typedef struct _Elm_Win_Smart_Data Elm_Win_Smart_Data;

struct _Elm_Win_Smart_Data
{
   Ecore_Evas           *ee;
   Evas                 *evas;
   Evas_Object          *parent; /* parent *window* object*/
   Evas_Object          *img_obj, *frame_obj;
   Evas_Coord           fx, fy, fw, fh;
   Eina_List            *resize_objs; /* a window may have
                                       * *multiple* resize
                                       * objects */
   Evas_Object          *obj; /* The object itself */
#ifdef HAVE_ELEMENTARY_X
   struct
   {
      Ecore_X_Window       xwin;
      Ecore_Event_Handler *client_message_handler;
      Ecore_Event_Handler *property_handler;
   } x;
#endif
#ifdef HAVE_ELEMENTARY_WAYLAND
   struct
   {
      Ecore_Wl_Window *win;
   } wl;
#endif

   Ecore_Job                     *deferred_resize_job;
   Ecore_Job                     *deferred_child_eval_job;

   Elm_Win_Type                   type;
   Elm_Win_Keyboard_Mode          kbdmode;
   Elm_Win_Indicator_Mode         indmode;
   Elm_Win_Indicator_Opacity_Mode ind_o_mode;
   struct
   {
      const char  *info;
      Ecore_Timer *timer;
      int          repeat_count;
      int          shot_counter;
   } shot;
   int                            resize_location;
   int                           *autodel_clear, rot;
   int                            show_count;
   struct
   {
      int x, y;
   } screen;
   struct
   {
      Ecore_Evas  *ee;
      Evas        *evas;
      Evas_Object *obj, *hot_obj;
      int          hot_x, hot_y;
   } pointer;
   struct
   {
      Evas_Object *top;

      struct
      {
         Evas_Object *target;
         Eina_Bool    visible : 1;
         Eina_Bool    handled : 1;
      } cur, prev;

      const char  *style;
      Ecore_Job   *reconf_job;

      Eina_Bool    enabled : 1;
      Eina_Bool    changed_theme : 1;
      Eina_Bool    top_animate : 1;
      Eina_Bool    geometry_changed : 1;
   } focus_highlight;

   Evas_Object *icon;
   const char  *title;
   const char  *icon_name;
   const char  *role;

   struct
   {
      const char  *name;
      const char **available_list;
      unsigned int count;
   } profile;

   void *trap_data;

   double       aspect;
   int          size_base_w, size_base_h;
   int          size_step_w, size_step_h;
   int          norender;
   Eina_Bool    urgent : 1;
   Eina_Bool    modal : 1;
   Eina_Bool    demand_attention : 1;
   Eina_Bool    autodel : 1;
   Eina_Bool    constrain : 1;
   Eina_Bool    resizing : 1;
   Eina_Bool    iconified : 1;
   Eina_Bool    withdrawn : 1;
   Eina_Bool    sticky : 1;
   Eina_Bool    fullscreen : 1;
   Eina_Bool    maximized : 1;
   Eina_Bool    skip_focus : 1;
   Eina_Bool    floating : 1;
};

static const char SIG_DELETE_REQUEST[] = "delete,request";
static const char SIG_FOCUS_OUT[] = "focus,out";
static const char SIG_FOCUS_IN[] = "focus,in";
static const char SIG_MOVED[] = "moved";
static const char SIG_WITHDRAWN[] = "withdrawn";
static const char SIG_ICONIFIED[] = "iconified";
static const char SIG_NORMAL[] = "normal";
static const char SIG_STICK[] = "stick";
static const char SIG_UNSTICK[] = "unstick";
static const char SIG_FULLSCREEN[] = "fullscreen";
static const char SIG_UNFULLSCREEN[] = "unfullscreen";
static const char SIG_MAXIMIZED[] = "maximized";
static const char SIG_UNMAXIMIZED[] = "unmaximized";
static const char SIG_IOERR[] = "ioerr";
static const char SIG_INDICATOR_PROP_CHANGED[] = "indicator,prop,changed";
static const char SIG_ROTATION_CHANGED[] = "rotation,changed";
static const char SIG_PROFILE_CHANGED[] = "profile,changed";

static const Evas_Smart_Cb_Description _smart_callbacks[] = {
   {SIG_DELETE_REQUEST, ""},
   {SIG_FOCUS_OUT, ""},
   {SIG_FOCUS_IN, ""},
   {SIG_MOVED, ""},
   {SIG_WITHDRAWN, ""},
   {SIG_ICONIFIED, ""},
   {SIG_NORMAL, ""},
   {SIG_STICK, ""},
   {SIG_UNSTICK, ""},
   {SIG_FULLSCREEN, ""},
   {SIG_UNFULLSCREEN, ""},
   {SIG_MAXIMIZED, ""},
   {SIG_UNMAXIMIZED, ""},
   {SIG_IOERR, ""},
   {SIG_INDICATOR_PROP_CHANGED, ""},
   {SIG_ROTATION_CHANGED, ""},
   {SIG_PROFILE_CHANGED, ""},
   {NULL, NULL}
};

Eina_List *_elm_win_list = NULL;
int _elm_win_deferred_free = 0;

static int _elm_win_count = 0;
static int _elm_win_count_shown = 0;
static int _elm_win_count_iconified = 0;
static int _elm_win_count_withdrawn = 0;

static Eina_Bool _elm_win_auto_throttled = EINA_FALSE;

static Ecore_Job *_elm_win_state_eval_job = NULL;

static void
_elm_win_state_eval(void *data __UNUSED__)
{
   Eina_List *l;
   Evas_Object *obj;

   _elm_win_state_eval_job = NULL;

   if (_elm_config->auto_norender_withdrawn)
     {
        EINA_LIST_FOREACH(_elm_win_list, l, obj)
          {
             if ((elm_win_withdrawn_get(obj)) ||
                 ((elm_win_iconified_get(obj) &&
                   (_elm_config->auto_norender_iconified_same_as_withdrawn))))
               {
                  if (!evas_object_data_get(obj, "__win_auto_norender"))
                    {
                       Evas *evas = evas_object_evas_get(obj);

                       elm_win_norender_push(obj);
                       evas_object_data_set(obj, "__win_auto_norender", obj);

                       if (_elm_config->auto_flush_withdrawn)
                         {
                            edje_file_cache_flush();
                            edje_collection_cache_flush();
                            evas_image_cache_flush(evas);
                            evas_font_cache_flush(evas);
                         }
                       if (_elm_config->auto_dump_withdrawn)
                         {
                            evas_render_dump(evas);
                         }
                    }
               }
             else
               {
                  if (evas_object_data_get(obj, "__win_auto_norender"))
                    {
                       elm_win_norender_pop(obj);
                       evas_object_data_del(obj, "__win_auto_norender");
                    }
               }
          }
     }
   if (_elm_config->auto_throttle)
     {
        if (_elm_win_count == 0)
          {
             if (_elm_win_auto_throttled)
               {
                  ecore_throttle_adjust(-_elm_config->auto_throttle_amount);
                  _elm_win_auto_throttled = EINA_FALSE;
               }
          }
        else
          {
             if ((_elm_win_count_iconified + _elm_win_count_withdrawn) >=
                 _elm_win_count_shown)
               {
                  if (!_elm_win_auto_throttled)
                    {
                       ecore_throttle_adjust(_elm_config->auto_throttle_amount);
                       _elm_win_auto_throttled = EINA_TRUE;
                    }
               }
             else
               {
                  if (_elm_win_auto_throttled)
                    {
                       ecore_throttle_adjust(-_elm_config->auto_throttle_amount);
                       _elm_win_auto_throttled = EINA_FALSE;
                    }
               }
          }
     }
}

static void
_elm_win_state_eval_queue(void)
{
   if (_elm_win_state_eval_job) ecore_job_del(_elm_win_state_eval_job);
   _elm_win_state_eval_job = ecore_job_add(_elm_win_state_eval, NULL);
}

// example shot spec (wait 0.1 sec then save as my-window.png):
// ELM_ENGINE="shot:delay=0.1:file=my-window.png"

static double
_shot_delay_get(Elm_Win_Smart_Data *sd)
{
   char *p, *pd;
   char *d = strdup(sd->shot.info);

   if (!d) return 0.5;
   for (p = (char *)sd->shot.info; *p; p++)
     {
        if (!strncmp(p, "delay=", 6))
          {
             double v;

             for (pd = d, p += 6; (*p) && (*p != ':'); p++, pd++)
               {
                  *pd = *p;
               }
             *pd = 0;
             v = _elm_atof(d);
             free(d);
             return v;
          }
     }
   free(d);

   return 0.5;
}

static char *
_shot_file_get(Elm_Win_Smart_Data *sd)
{
   char *p;
   char *tmp = strdup(sd->shot.info);
   char *repname = NULL;

   if (!tmp) return NULL;

   for (p = (char *)sd->shot.info; *p; p++)
     {
        if (!strncmp(p, "file=", 5))
          {
             strcpy(tmp, p + 5);
             if (!sd->shot.repeat_count) return tmp;
             else
               {
                  char *dotptr = strrchr(tmp, '.');
                  if (dotptr)
                    {
                       size_t size = sizeof(char) * (strlen(tmp) + 16);
                       repname = malloc(size);
                       strncpy(repname, tmp, dotptr - tmp);
                       snprintf(repname + (dotptr - tmp), size -
                                (dotptr - tmp), "%03i",
                                sd->shot.shot_counter + 1);
                       strcat(repname, dotptr);
                       free(tmp);
                       return repname;
                    }
               }
          }
     }
   free(tmp);
   if (!sd->shot.repeat_count) return strdup("out.png");

   repname = malloc(sizeof(char) * 24);
   snprintf(repname, sizeof(char) * 24, "out%03i.png",
            sd->shot.shot_counter + 1);

   return repname;
}

static int
_shot_repeat_count_get(Elm_Win_Smart_Data *sd)
{
   char *p, *pd;
   char *d = strdup(sd->shot.info);

   if (!d) return 0;
   for (p = (char *)sd->shot.info; *p; p++)
     {
        if (!strncmp(p, "repeat=", 7))
          {
             int v;

             for (pd = d, p += 7; (*p) && (*p != ':'); p++, pd++)
               {
                  *pd = *p;
               }
             *pd = 0;
             v = atoi(d);
             if (v < 0) v = 0;
             if (v > 1000) v = 999;
             free(d);
             return v;
          }
     }
   free(d);

   return 0;
}

static char *
_shot_key_get(Elm_Win_Smart_Data *sd __UNUSED__)
{
   return NULL;
}

static char *
_shot_flags_get(Elm_Win_Smart_Data *sd __UNUSED__)
{
   return NULL;
}

static void
_shot_do(Elm_Win_Smart_Data *sd)
{
   Ecore_Evas *ee;
   Evas_Object *o;
   unsigned int *pixels;
   int w, h;
   char *file, *key, *flags;

   ecore_evas_manual_render(sd->ee);
   pixels = (void *)ecore_evas_buffer_pixels_get(sd->ee);
   if (!pixels) return;

   ecore_evas_geometry_get(sd->ee, NULL, NULL, &w, &h);
   if ((w < 1) || (h < 1)) return;

   file = _shot_file_get(sd);
   if (!file) return;

   key = _shot_key_get(sd);
   flags = _shot_flags_get(sd);
   ee = ecore_evas_buffer_new(1, 1);
   o = evas_object_image_add(ecore_evas_get(ee));
   evas_object_image_alpha_set(o, ecore_evas_alpha_get(sd->ee));
   evas_object_image_size_set(o, w, h);
   evas_object_image_data_set(o, pixels);
   if (!evas_object_image_save(o, file, key, flags))
     {
        ERR("Cannot save window to '%s' (key '%s', flags '%s')",
            file, key, flags);
     }
   free(file);
   if (key) free(key);
   if (flags) free(flags);
   ecore_evas_free(ee);
   if (sd->shot.repeat_count) sd->shot.shot_counter++;
}

static Eina_Bool
_shot_delay(void *data)
{
   Elm_Win_Smart_Data *sd = data;

   _shot_do(sd);
   if (sd->shot.repeat_count)
     {
        int remainshot = (sd->shot.repeat_count - sd->shot.shot_counter);
        if (remainshot > 0) return EINA_TRUE;
     }
   sd->shot.timer = NULL;
   elm_exit();

   return EINA_FALSE;
}

static void
_shot_init(Elm_Win_Smart_Data *sd)
{
   if (!sd->shot.info) return;

   sd->shot.repeat_count = _shot_repeat_count_get(sd);
   sd->shot.shot_counter = 0;
}

static void
_shot_handle(Elm_Win_Smart_Data *sd)
{
   if (!sd->shot.info) return;

   sd->shot.timer = ecore_timer_add(_shot_delay_get(sd), _shot_delay, sd);
}

/* elm-win specific associate, does the trap while ecore_evas_object_associate()
 * does not.
 */
static Elm_Win_Smart_Data *
_elm_win_associate_get(const Ecore_Evas *ee)
{
   return ecore_evas_data_get(ee, "elm_win");
}

/* Interceptors Callbacks */
static void
_elm_win_obj_intercept_raise(void *data, Evas_Object *obj __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;
   TRAP(sd, raise);
}

static void
_elm_win_obj_intercept_lower(void *data, Evas_Object *obj __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;
   TRAP(sd, lower);
}

static void
_elm_win_obj_intercept_stack_above(void *data __UNUSED__, Evas_Object *obj __UNUSED__, Evas_Object *above __UNUSED__)
{
   INF("TODO: %s", __FUNCTION__);
}

static void
_elm_win_obj_intercept_stack_below(void *data __UNUSED__, Evas_Object *obj __UNUSED__, Evas_Object *below __UNUSED__)
{
   INF("TODO: %s", __FUNCTION__);
}

static void
_elm_win_obj_intercept_layer_set(void *data, Evas_Object *obj __UNUSED__, int l)
{
   Elm_Win_Smart_Data *sd = data;
   TRAP(sd, layer_set, l);
}

/* Event Callbacks */

static void
_elm_win_obj_callback_changed_size_hints(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;
   Evas_Coord w, h;

   evas_object_size_hint_min_get(obj, &w, &h);
   TRAP(sd, size_min_set, w, h);

   evas_object_size_hint_max_get(obj, &w, &h);
   if (w < 1) w = -1;
   if (h < 1) h = -1;
   TRAP(sd, size_max_set, w, h);
}
/* end of elm-win specific associate */


static void
_elm_win_move(Ecore_Evas *ee)
{
   Elm_Win_Smart_Data *sd = _elm_win_associate_get(ee);
   int x, y;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   ecore_evas_geometry_get(ee, &x, &y, NULL, NULL);
   sd->screen.x = x;
   sd->screen.y = y;
   evas_object_smart_callback_call(sd->obj, SIG_MOVED, NULL);
}

static void
_elm_win_resize_job(void *data)
{
   Elm_Win_Smart_Data *sd = data;
   const Eina_List *l;
   Evas_Object *obj;
   int w, h;

   sd->deferred_resize_job = NULL;
   ecore_evas_request_geometry_get(sd->ee, NULL, NULL, &w, &h);
   if (sd->constrain)
     {
        int sw, sh;
        ecore_evas_screen_geometry_get(sd->ee, NULL, NULL, &sw, &sh);
        w = MIN(w, sw);
        h = MIN(h, sh);
     }

   if (sd->frame_obj)
     evas_object_resize(sd->frame_obj, w, h);

   /* if (sd->img_obj) */
   /*   { */
   /*   } */

   evas_object_resize(sd->obj, w, h);
   EINA_LIST_FOREACH(sd->resize_objs, l, obj)
     {
        evas_object_move(obj, sd->fx, sd->fy);
        evas_object_resize(obj, w - sd->fw, h - sd->fy);
     }
}

static void
_elm_win_resize(Ecore_Evas *ee)
{
   Elm_Win_Smart_Data *sd = _elm_win_associate_get(ee);
   EINA_SAFETY_ON_NULL_RETURN(sd);

   if (sd->deferred_resize_job) ecore_job_del(sd->deferred_resize_job);
   sd->deferred_resize_job = ecore_job_add(_elm_win_resize_job, sd);
}

static void
_elm_win_mouse_in(Ecore_Evas *ee)
{
   Elm_Win_Smart_Data *sd = _elm_win_associate_get(ee);
   EINA_SAFETY_ON_NULL_RETURN(sd);

   if (sd->resizing) sd->resizing = EINA_FALSE;
}

static void
_elm_win_focus_highlight_reconfigure_job_stop(Elm_Win_Smart_Data *sd)
{
   if (sd->focus_highlight.reconf_job)
     ecore_job_del(sd->focus_highlight.reconf_job);
   sd->focus_highlight.reconf_job = NULL;
}

static void
_elm_win_focus_highlight_visible_set(Elm_Win_Smart_Data *sd,
                                     Eina_Bool visible)
{
   Evas_Object *top;

   top = sd->focus_highlight.top;
   if (visible)
     {
        if (top)
          {
             evas_object_show(top);
             edje_object_signal_emit(top, "elm,action,focus,show", "elm");
          }
     }
   else
     {
        if (top)
          edje_object_signal_emit(top, "elm,action,focus,hide", "elm");
     }
}

static void
_elm_win_focus_highlight_anim_setup(Elm_Win_Smart_Data *sd,
                                    Evas_Object *obj)
{
   Evas_Coord tx, ty, tw, th;
   Evas_Coord w, h, px, py, pw, ph;
   Edje_Message_Int_Set *m;
   Evas_Object *previous = sd->focus_highlight.prev.target;
   Evas_Object *target = sd->focus_highlight.cur.target;

   evas_object_geometry_get(sd->obj, NULL, NULL, &w, &h);
   evas_object_geometry_get(target, &tx, &ty, &tw, &th);
   evas_object_geometry_get(previous, &px, &py, &pw, &ph);
   evas_object_move(obj, 0, 0);
   evas_object_resize(obj, tw, th);
   evas_object_clip_unset(obj);

   m = alloca(sizeof(*m) + (sizeof(int) * 8));
   m->count = 8;
   m->val[0] = px;
   m->val[1] = py;
   m->val[2] = pw;
   m->val[3] = ph;
   m->val[4] = tx;
   m->val[5] = ty;
   m->val[6] = tw;
   m->val[7] = th;
   edje_object_message_send(obj, EDJE_MESSAGE_INT_SET, 1, m);
}

static void
_elm_win_focus_highlight_simple_setup(Elm_Win_Smart_Data *sd,
                                      Evas_Object *obj)
{
   Evas_Object *clip, *target = sd->focus_highlight.cur.target;
   Evas_Coord x, y, w, h;

   clip = evas_object_clip_get(target);
   evas_object_geometry_get(target, &x, &y, &w, &h);

   evas_object_move(obj, x, y);
   evas_object_resize(obj, w, h);
   evas_object_clip_set(obj, clip);
}

static void
_elm_win_focus_highlight_reconfigure(Elm_Win_Smart_Data *sd)
{
   Evas_Object *target = sd->focus_highlight.cur.target;
   Evas_Object *previous = sd->focus_highlight.prev.target;
   Evas_Object *top = sd->focus_highlight.top;
   Eina_Bool visible_changed;
   Eina_Bool common_visible;
   const char *sig = NULL;

   _elm_win_focus_highlight_reconfigure_job_stop(sd);

   visible_changed = (sd->focus_highlight.cur.visible !=
                      sd->focus_highlight.prev.visible);

   if ((target == previous) && (!visible_changed) &&
       (!sd->focus_highlight.geometry_changed))
     return;

   if ((previous) && (sd->focus_highlight.prev.handled))
     elm_widget_signal_emit
       (previous, "elm,action,focus_highlight,hide", "elm");

   if (!target)
     common_visible = EINA_FALSE;
   else if (sd->focus_highlight.cur.handled)
     {
        common_visible = EINA_FALSE;
        if (sd->focus_highlight.cur.visible)
          sig = "elm,action,focus_highlight,show";
        else
          sig = "elm,action,focus_highlight,hide";
     }
   else
     common_visible = sd->focus_highlight.cur.visible;

   _elm_win_focus_highlight_visible_set(sd, common_visible);
   if (sig)
     elm_widget_signal_emit(target, sig, "elm");

   if ((!target) || (!common_visible) || (sd->focus_highlight.cur.handled))
     goto the_end;

   if (sd->focus_highlight.changed_theme)
     {
        const char *str;
        if (sd->focus_highlight.style)
          str = sd->focus_highlight.style;
        else
          str = "default";
        elm_widget_theme_object_set
          (sd->obj, top, "focus_highlight", "top", str);
        sd->focus_highlight.changed_theme = EINA_FALSE;

        if (_elm_config->focus_highlight_animate)
          {
             str = edje_object_data_get(sd->focus_highlight.top, "animate");
             sd->focus_highlight.top_animate = ((str) && (!strcmp(str, "on")));
          }
     }

   if ((sd->focus_highlight.top_animate) && (previous) &&
       (!sd->focus_highlight.prev.handled))
     _elm_win_focus_highlight_anim_setup(sd, top);
   else
     _elm_win_focus_highlight_simple_setup(sd, top);
   evas_object_raise(top);

the_end:
   sd->focus_highlight.geometry_changed = EINA_FALSE;
   sd->focus_highlight.prev = sd->focus_highlight.cur;
}

static void
_elm_win_focus_highlight_reconfigure_job(void *data)
{
   _elm_win_focus_highlight_reconfigure((Elm_Win_Smart_Data *)data);
}

static void
_elm_win_focus_highlight_reconfigure_job_start(Elm_Win_Smart_Data *sd)
{
   if (sd->focus_highlight.reconf_job)
     ecore_job_del(sd->focus_highlight.reconf_job);

   sd->focus_highlight.reconf_job = ecore_job_add(
       _elm_win_focus_highlight_reconfigure_job, sd);
}

static void
_elm_win_focus_in(Ecore_Evas *ee)
{
   Elm_Win_Smart_Data *sd = _elm_win_associate_get(ee);
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   obj = sd->obj;

   _elm_widget_top_win_focused_set(obj, EINA_TRUE);
   if (!elm_widget_focus_order_get(obj))
     {
        elm_widget_focus_steal(obj);
        sd->show_count++;
     }
   else
     elm_widget_focus_restore(obj);
   evas_object_smart_callback_call(obj, SIG_FOCUS_IN, NULL);
   sd->focus_highlight.cur.visible = EINA_TRUE;
   _elm_win_focus_highlight_reconfigure_job_start(sd);
   if (sd->frame_obj)
     {
        edje_object_signal_emit(sd->frame_obj, "elm,action,focus", "elm");
     }

   /* do nothing */
   /* else if (sd->img_obj) */
   /*   { */
   /*   } */
}

static void
_elm_win_focus_out(Ecore_Evas *ee)
{
   Elm_Win_Smart_Data *sd = _elm_win_associate_get(ee);
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   obj = sd->obj;

   elm_object_focus_set(obj, EINA_FALSE);
   _elm_widget_top_win_focused_set(obj, EINA_FALSE);
   evas_object_smart_callback_call(obj, SIG_FOCUS_OUT, NULL);
   sd->focus_highlight.cur.visible = EINA_FALSE;
   _elm_win_focus_highlight_reconfigure_job_start(sd);
   if (sd->frame_obj)
     {
        edje_object_signal_emit(sd->frame_obj, "elm,action,unfocus", "elm");
     }

   /* do nothing */
   /* if (sd->img_obj) */
   /*   { */
   /*   } */
}

static void
_elm_win_available_profiles_del(Elm_Win_Smart_Data *sd)
{
   if (!sd->profile.available_list) return;

   unsigned int i;
   for (i = 0; i < sd->profile.count; i++)
     if (sd->profile.available_list[i])
       {
          eina_stringshare_del(sd->profile.available_list[i]);
          sd->profile.available_list[i] = NULL;
       }
   sd->profile.count = 0;
   free(sd->profile.available_list);
   sd->profile.available_list = NULL;
}

static void
_elm_win_profile_del(Elm_Win_Smart_Data *sd)
{
   if (!sd->profile.name) return;
   eina_stringshare_del(sd->profile.name);
   sd->profile.name = NULL;
}

static Eina_Bool
_elm_win_profile_set(Elm_Win_Smart_Data *sd, const char *profile)
{
   Eina_Bool changed = EINA_FALSE;
   if (profile)
     {
        if (sd->profile.name)
          {
             if (strcmp(sd->profile.name, profile) != 0)
               {
                  eina_stringshare_replace(&(sd->profile.name), profile);
                  changed = EINA_TRUE;
               }
          }
        else
          {
             sd->profile.name = eina_stringshare_add(profile);
             changed = EINA_TRUE;
          }
     }
   else
     _elm_win_profile_del(sd);

   return changed;
}

static void
_elm_win_profile_update(Elm_Win_Smart_Data *sd)
{
   if (sd->profile.available_list)
     {
        Eina_Bool found = EINA_FALSE;
        if (sd->profile.name)
          {
             unsigned int i;
             for (i = 0; i < sd->profile.count; i++)
               {
                  if (!strcmp(sd->profile.name,
                              sd->profile.available_list[i]))
                    {
                       found = EINA_TRUE;
                       break;
                    }
               }
          }

        /* If current profile is not present in an available profiles,
         * change current profile to the 1st element of an array.
         */
        if (!found)
          _elm_win_profile_set(sd, sd->profile.available_list[0]);
     }

   _elm_config_profile_set(sd->profile.name);

   /* update sub ee */
   Ecore_Evas *ee2;
   Eina_List *sub, *l = NULL;

   sub = ecore_evas_sub_ecore_evas_list_get(sd->ee);
   EINA_LIST_FOREACH(sub, l, ee2)
     ecore_evas_window_profile_set(ee2, sd->profile.name);

   evas_object_smart_callback_call(sd->obj, SIG_PROFILE_CHANGED, NULL);
}

static void
_elm_win_state_change(Ecore_Evas *ee)
{
   Elm_Win_Smart_Data *sd = _elm_win_associate_get(ee);
   Evas_Object *obj;
   Eina_Bool ch_withdrawn = EINA_FALSE;
   Eina_Bool ch_sticky = EINA_FALSE;
   Eina_Bool ch_iconified = EINA_FALSE;
   Eina_Bool ch_fullscreen = EINA_FALSE;
   Eina_Bool ch_maximized = EINA_FALSE;
   Eina_Bool ch_profile = EINA_FALSE;
   const char *profile;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   obj = sd->obj;

   if (sd->withdrawn) _elm_win_count_withdrawn--;
   if (sd->iconified) _elm_win_count_iconified--;

   if (sd->withdrawn != ecore_evas_withdrawn_get(sd->ee))
     {
        sd->withdrawn = ecore_evas_withdrawn_get(sd->ee);
        ch_withdrawn = EINA_TRUE;
     }
   if (sd->sticky != ecore_evas_sticky_get(sd->ee))
     {
        sd->sticky = ecore_evas_sticky_get(sd->ee);
        ch_sticky = EINA_TRUE;
     }
   if (sd->iconified != ecore_evas_iconified_get(sd->ee))
     {
        sd->iconified = ecore_evas_iconified_get(sd->ee);
        ch_iconified = EINA_TRUE;
     }
   if (sd->fullscreen != ecore_evas_fullscreen_get(sd->ee))
     {
        sd->fullscreen = ecore_evas_fullscreen_get(sd->ee);
        ch_fullscreen = EINA_TRUE;
     }
   if (sd->maximized != ecore_evas_maximized_get(sd->ee))
     {
        sd->maximized = ecore_evas_maximized_get(sd->ee);
        ch_maximized = EINA_TRUE;
     }

   profile = ecore_evas_window_profile_get(sd->ee);
   ch_profile = _elm_win_profile_set(sd, profile);

   if (sd->withdrawn) _elm_win_count_withdrawn++;
   if (sd->iconified) _elm_win_count_iconified++;
   _elm_win_state_eval_queue();

   if ((ch_withdrawn) || (ch_iconified))
     {
        if (sd->withdrawn)
          evas_object_smart_callback_call(obj, SIG_WITHDRAWN, NULL);
        else if (sd->iconified)
          evas_object_smart_callback_call(obj, SIG_ICONIFIED, NULL);
        else
          evas_object_smart_callback_call(obj, SIG_NORMAL, NULL);
     }
   if (ch_sticky)
     {
        if (sd->sticky)
          evas_object_smart_callback_call(obj, SIG_STICK, NULL);
        else
          evas_object_smart_callback_call(obj, SIG_UNSTICK, NULL);
     }
   if (ch_fullscreen)
     {
        if (sd->fullscreen)
          evas_object_smart_callback_call(obj, SIG_FULLSCREEN, NULL);
        else
          evas_object_smart_callback_call(obj, SIG_UNFULLSCREEN, NULL);
     }
   if (ch_maximized)
     {
        if (sd->maximized)
          evas_object_smart_callback_call(obj, SIG_MAXIMIZED, NULL);
        else
          evas_object_smart_callback_call(obj, SIG_UNMAXIMIZED, NULL);
     }
   if (ch_profile)
     {
        _elm_win_profile_update(sd);
     }
}

static void
_elm_win_smart_focus_next_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_win_smart_focus_next(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Focus_Direction dir = va_arg(*list, Elm_Focus_Direction);
   Evas_Object **next = va_arg(*list, Evas_Object **);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Elm_Widget_Smart_Data *wd = eo_data_get(obj, ELM_OBJ_WIDGET_CLASS);

   const Eina_List *items;
   void *(*list_data_get)(const Eina_List *list);

   /* Focus chain */
   if (wd->subobjs)
     {
        if (!(items = elm_widget_focus_custom_chain_get(obj)))
          {
             items = wd->subobjs;
             if (!items)
               return;
          }
        list_data_get = eina_list_data_get;

        elm_widget_focus_list_next_get(obj, items, list_data_get, dir, next);

        if (*next)
          {
             if (ret) *ret = EINA_TRUE;
             return;
          }
     }
   *next = (Evas_Object *)obj;
   return;
}

static void
_elm_win_smart_focus_direction_manager_is(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   *ret = EINA_TRUE;
}

static void
_elm_win_smart_focus_direction(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   const Evas_Object *base = va_arg(*list, Evas_Object *);
   double degree = va_arg(*list, double);
   Evas_Object **direction = va_arg(*list, Evas_Object **);
   double *weight = va_arg(*list, double *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Eina_Bool int_ret = EINA_FALSE;

   const Eina_List *items;
   void *(*list_data_get)(const Eina_List *list);

   Elm_Widget_Smart_Data *wd = eo_data_get(obj, ELM_OBJ_WIDGET_CLASS);

   /* Focus chain */
   if (wd->subobjs)
     {
        if (!(items = elm_widget_focus_custom_chain_get(obj)))
          items = wd->subobjs;

        list_data_get = eina_list_data_get;

        int_ret = elm_widget_focus_list_direction_get
                 (obj, base, items, list_data_get, degree, direction, weight);
        if (ret) *ret = int_ret;
     }
}

static void
_elm_win_smart_on_focus(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_TRUE;

   Elm_Win_Smart_Data *sd = _pd;

   if (sd->img_obj)
     evas_object_focus_set(sd->img_obj, elm_widget_focus_get(obj));
   else
     evas_object_focus_set(obj, elm_widget_focus_get(obj));
}

static void
_elm_win_smart_event(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *source = va_arg(*list, Evas_Object *);
   (void) source;
   Evas_Callback_Type type = va_arg(*list, Evas_Callback_Type);
   void *event_info = va_arg(*list, void *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   Evas_Event_Key_Down *ev = event_info;
   Evas_Object *current_focused;

   if (elm_widget_disabled_get(obj)) return;

   if (type != EVAS_CALLBACK_KEY_DOWN)
     return;

   current_focused = elm_widget_focused_object_get(obj);
   if ((!strcmp(ev->keyname, "Tab")) ||
       (!strcmp(ev->keyname, "ISO_Left_Tab")))
     {
        if (evas_key_modifier_is_set(ev->modifiers, "Shift"))
          elm_widget_focus_cycle(obj, ELM_FOCUS_PREVIOUS);
        else
          elm_widget_focus_cycle(obj, ELM_FOCUS_NEXT);

        goto success;
     }
   else if ((!strcmp(ev->keyname, "Left")) ||
            ((!strcmp(ev->keyname, "KP_Left")) && (!ev->string)))
     {
        if (current_focused == obj)
          elm_widget_focus_cycle(obj, ELM_FOCUS_NEXT);
        else
          elm_widget_focus_direction_go(obj, 270.0);

        goto success;
     }
   else if ((!strcmp(ev->keyname, "Right")) ||
            ((!strcmp(ev->keyname, "KP_Right")) && (!ev->string)))
     {
        if (current_focused == obj)
          elm_widget_focus_cycle(obj, ELM_FOCUS_NEXT);
        else
          elm_widget_focus_direction_go(obj, 90.0);

        goto success;
     }
   else if ((!strcmp(ev->keyname, "Up")) ||
            ((!strcmp(ev->keyname, "KP_Up")) && (!ev->string)))
     {
        if (current_focused == obj)
          elm_widget_focus_cycle(obj, ELM_FOCUS_NEXT);
        else
          elm_widget_focus_direction_go(obj, 0.0);

        goto success;
     }
   else if ((!strcmp(ev->keyname, "Down")) ||
            ((!strcmp(ev->keyname, "KP_Down")) && (!ev->string)))
     {
        if (current_focused == obj)
          elm_widget_focus_cycle(obj, ELM_FOCUS_NEXT);
        else
          elm_widget_focus_direction_go(obj, 180.0);

        goto success;
     }

   return;

success:
   ev->event_flags |= EVAS_EVENT_FLAG_ON_HOLD;
   if (ret) *ret = EINA_TRUE;
}

static void
_deferred_ecore_evas_free(void *data)
{
   ecore_evas_free(data);
   _elm_win_deferred_free--;
}

static void
_elm_win_smart_show(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Win_Smart_Data *sd = _pd;

   if (!evas_object_visible_get(obj))
     {
        _elm_win_count_shown++;
        _elm_win_state_eval_queue();
     }
   eo_do_super(obj, evas_obj_smart_show());

   TRAP(sd, show);

   if (!sd->show_count) sd->show_count++;
   if (sd->shot.info) _shot_handle(sd);
}

static void
_elm_win_smart_hide(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Win_Smart_Data *sd = _pd;

   if (evas_object_visible_get(obj))
     {
        _elm_win_count_shown--;
        _elm_win_state_eval_queue();
     }
   eo_do_super(obj, evas_obj_smart_hide());

   TRAP(sd, hide);

   if (sd->frame_obj)
     {
        evas_object_hide(sd->frame_obj);
     }
   if (sd->img_obj)
     {
        evas_object_hide(sd->img_obj);
     }
   if (sd->pointer.obj)
     {
        evas_object_hide(sd->pointer.obj);
        ecore_evas_hide(sd->pointer.ee);
     }
}

static void
_elm_win_on_parent_del(void *data,
                       Evas *e __UNUSED__,
                       Evas_Object *obj,
                       void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;

   if (obj == sd->parent) sd->parent = NULL;
}

static void
_elm_win_focus_target_move(void *data,
                           Evas *e __UNUSED__,
                           Evas_Object *obj __UNUSED__,
                           void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;

   sd->focus_highlight.geometry_changed = EINA_TRUE;
   _elm_win_focus_highlight_reconfigure_job_start(sd);
}

static void
_elm_win_focus_target_resize(void *data,
                             Evas *e __UNUSED__,
                             Evas_Object *obj __UNUSED__,
                             void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;

   sd->focus_highlight.geometry_changed = EINA_TRUE;
   _elm_win_focus_highlight_reconfigure_job_start(sd);
}

static void
_elm_win_focus_target_del(void *data,
                          Evas *e __UNUSED__,
                          Evas_Object *obj __UNUSED__,
                          void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;

   sd->focus_highlight.cur.target = NULL;

   _elm_win_focus_highlight_reconfigure_job_start(sd);
}

static Evas_Object *
_elm_win_focus_target_get(Evas_Object *obj)
{
   Evas_Object *o = obj;

   do
     {
        if (elm_widget_is(o))
          {
             if (!elm_widget_highlight_ignore_get(o))
               break;
             o = elm_widget_parent_get(o);
             if (!o)
               o = evas_object_smart_parent_get(o);
          }
        else
          {
             o = elm_widget_parent_widget_get(o);
             if (!o)
               o = evas_object_smart_parent_get(o);
          }
     }
   while (o);

   return o;
}

static void
_elm_win_focus_target_callbacks_add(Elm_Win_Smart_Data *sd)
{
   Evas_Object *obj = sd->focus_highlight.cur.target;
   if (!obj) return;

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_MOVE, _elm_win_focus_target_move, sd);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_RESIZE, _elm_win_focus_target_resize, sd);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_DEL, _elm_win_focus_target_del, sd);
}

static void
_elm_win_focus_target_callbacks_del(Elm_Win_Smart_Data *sd)
{
   Evas_Object *obj = sd->focus_highlight.cur.target;

   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_MOVE, _elm_win_focus_target_move, sd);
   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_RESIZE, _elm_win_focus_target_resize, sd);
   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_DEL, _elm_win_focus_target_del, sd);
}

static void
_elm_win_object_focus_in(void *data,
                         Evas *e __UNUSED__,
                         void *event_info)
{
   Evas_Object *obj = event_info, *target;
   Elm_Win_Smart_Data *sd = data;

   if (sd->focus_highlight.cur.target == obj)
     return;

   target = _elm_win_focus_target_get(obj);
   sd->focus_highlight.cur.target = target;
   if (elm_widget_highlight_in_theme_get(target))
     sd->focus_highlight.cur.handled = EINA_TRUE;
   else
     _elm_win_focus_target_callbacks_add(sd);

   _elm_win_focus_highlight_reconfigure_job_start(sd);
}

static void
_elm_win_object_focus_out(void *data,
                          Evas *e __UNUSED__,
                          void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;

   if (!sd->focus_highlight.cur.target)
     return;

   if (!sd->focus_highlight.cur.handled)
     _elm_win_focus_target_callbacks_del(sd);

   sd->focus_highlight.cur.target = NULL;
   sd->focus_highlight.cur.handled = EINA_FALSE;

   _elm_win_focus_highlight_reconfigure_job_start(sd);
}

static void
_elm_win_focus_highlight_shutdown(Elm_Win_Smart_Data *sd)
{
   _elm_win_focus_highlight_reconfigure_job_stop(sd);
   if (sd->focus_highlight.cur.target)
     {
        _elm_win_focus_target_callbacks_del(sd);
        sd->focus_highlight.cur.target = NULL;
     }
   if (sd->focus_highlight.top)
     {
        evas_object_del(sd->focus_highlight.top);
        sd->focus_highlight.top = NULL;
     }

   evas_event_callback_del_full
     (sd->evas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
     _elm_win_object_focus_in, sd);
   evas_event_callback_del_full
     (sd->evas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT,
     _elm_win_object_focus_out, sd);
}

static void
_elm_win_on_img_obj_del(void *data,
                        Evas *e __UNUSED__,
                        Evas_Object *obj __UNUSED__,
                        void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;
   sd->img_obj = NULL;
}

static void
_elm_win_smart_del(Eo *obj, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Win_Smart_Data *sd = _pd;

   /* NB: child deletion handled by parent's smart del */

   if ((trap) && (trap->del))
     trap->del(sd->trap_data, obj);

   if (sd->parent)
     {
        evas_object_event_callback_del_full
          (sd->parent, EVAS_CALLBACK_DEL, _elm_win_on_parent_del, sd);
        sd->parent = NULL;
     }

   if (sd->autodel_clear) *(sd->autodel_clear) = -1;

   _elm_win_list = eina_list_remove(_elm_win_list, obj);
   if (sd->withdrawn) _elm_win_count_withdrawn--;
   if (sd->iconified) _elm_win_count_iconified--;
   if (evas_object_visible_get(obj)) _elm_win_count_shown--;
   _elm_win_count--;
   _elm_win_state_eval_queue();

   if (sd->ee)
     {
        ecore_evas_callback_delete_request_set(sd->ee, NULL);
        ecore_evas_callback_resize_set(sd->ee, NULL);
     }
   if (sd->deferred_resize_job) ecore_job_del(sd->deferred_resize_job);
   if (sd->deferred_child_eval_job) ecore_job_del(sd->deferred_child_eval_job);
   if (sd->shot.info) eina_stringshare_del(sd->shot.info);
   if (sd->shot.timer) ecore_timer_del(sd->shot.timer);

#ifdef HAVE_ELEMENTARY_X
   if (sd->x.client_message_handler)
     ecore_event_handler_del(sd->x.client_message_handler);
   if (sd->x.property_handler)
     ecore_event_handler_del(sd->x.property_handler);
#endif

   if (sd->img_obj)
     {
        evas_object_event_callback_del_full
           (sd->img_obj, EVAS_CALLBACK_DEL, _elm_win_on_img_obj_del, sd);
        sd->img_obj = NULL;
     }
   else
     {
        if (sd->ee)
          {
             ecore_job_add(_deferred_ecore_evas_free, sd->ee);
             _elm_win_deferred_free++;
          }
     }

   _elm_win_focus_highlight_shutdown(sd);
   eina_stringshare_del(sd->focus_highlight.style);

   if (sd->title) eina_stringshare_del(sd->title);
   if (sd->icon_name) eina_stringshare_del(sd->icon_name);
   if (sd->role) eina_stringshare_del(sd->role);
   if (sd->icon) evas_object_del(sd->icon);

   _elm_win_profile_del(sd);
   _elm_win_available_profiles_del(sd);

   /* Don't let callback in the air that point to sd */
   ecore_evas_callback_delete_request_set(sd->ee, NULL);
   ecore_evas_callback_resize_set(sd->ee, NULL);
   ecore_evas_callback_mouse_in_set(sd->ee, NULL);
   ecore_evas_callback_focus_in_set(sd->ee, NULL);
   ecore_evas_callback_focus_out_set(sd->ee, NULL);
   ecore_evas_callback_move_set(sd->ee, NULL);
   ecore_evas_callback_state_change_set(sd->ee, NULL);

   eo_do_super(obj, evas_obj_smart_del());

   if ((!_elm_win_list) &&
       (elm_policy_get(ELM_POLICY_QUIT) == ELM_POLICY_QUIT_LAST_WINDOW_CLOSED))
     {
        edje_file_cache_flush();
        edje_collection_cache_flush();
        evas_image_cache_flush(evas_object_evas_get(obj));
        evas_font_cache_flush(evas_object_evas_get(obj));
        elm_exit();
     }
}

static void
_elm_win_obj_intercept_show(void *data,
                            Evas_Object *obj)
{
   Elm_Win_Smart_Data *sd = data;

   // this is called to make sure all smart containers have calculated their
   // sizes BEFORE we show the window to make sure it initially appears at
   // our desired size (ie min size is known first)
   evas_smart_objects_calculate(evas_object_evas_get(obj));
   if (sd->frame_obj)
     {
        evas_object_show(sd->frame_obj);
     }
   if (sd->img_obj)
     {
        evas_object_show(sd->img_obj);
     }
   if (sd->pointer.obj)
     {
        ecore_evas_show(sd->pointer.ee);
        evas_object_show(sd->pointer.obj);
     }
   evas_object_show(obj);
}

static void
_elm_win_smart_move(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord x = va_arg(*list, Evas_Coord);
   Evas_Coord y = va_arg(*list, Evas_Coord);
   Elm_Win_Smart_Data *sd = _pd;

   if (sd->img_obj)
     {
        if ((x != sd->screen.x) || (y != sd->screen.y))
          {
             sd->screen.x = x;
             sd->screen.y = y;
             evas_object_smart_callback_call(obj, SIG_MOVED, NULL);
          }
        return;
     }
   else
     {
        TRAP(sd, move, x, y);
        if (!ecore_evas_override_get(sd->ee))  return;
     }

   eo_do_super(obj, evas_obj_smart_move(x, y));

   if (ecore_evas_override_get(sd->ee))
     {
        sd->screen.x = x;
        sd->screen.y = y;
        evas_object_smart_callback_call(obj, SIG_MOVED, NULL);
     }
   if (sd->frame_obj)
     {
        /* FIXME: We should update ecore_wl_window_location here !! */
        sd->screen.x = x;
        sd->screen.y = y;
     }
   if (sd->img_obj)
     {
        sd->screen.x = x;
        sd->screen.y = y;
     }
}

static void
_elm_win_smart_resize(Eo *obj, void *_pd, va_list *list)
{
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);

   Elm_Win_Smart_Data *sd = _pd;

   eo_do_super(obj, evas_obj_smart_resize(w, h));

   if (sd->img_obj)
     {
        if (sd->constrain)
          {
             int sw, sh;

             ecore_evas_screen_geometry_get(sd->ee, NULL, NULL, &sw, &sh);
             w = MIN(w, sw);
             h = MIN(h, sh);
          }
        if (w < 1) w = 1;
        if (h < 1) h = 1;

        evas_object_image_size_set(sd->img_obj, w, h);
     }

   TRAP(sd, resize, w, h);
}

static void
_elm_win_delete_request(Ecore_Evas *ee)
{
   Elm_Win_Smart_Data *sd = _elm_win_associate_get(ee);
   Evas_Object *obj;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   obj = sd->obj;

   int autodel = sd->autodel;
   sd->autodel_clear = &autodel;
   evas_object_ref(obj);
   evas_object_smart_callback_call(obj, SIG_DELETE_REQUEST, NULL);
   // FIXME: if above callback deletes - then the below will be invalid
   if (autodel) evas_object_del(obj);
   else sd->autodel_clear = NULL;
   evas_object_unref(obj);
}

Ecore_X_Window
_elm_ee_xwin_get(const Ecore_Evas *ee)
{
#ifdef HAVE_ELEMENTARY_X
   Ecore_X_Window xwin = 0;

   if (!ee) return 0;
   if (EE_ENGINE_COMPARE(ee, ELM_SOFTWARE_X11))
     {
        if (ee) xwin = ecore_evas_software_x11_window_get(ee);
     }
   else if (EE_ENGINE_COMPARE(ee, ELM_SOFTWARE_FB) ||
            EE_ENGINE_COMPARE(ee, ELM_SOFTWARE_16_WINCE) ||
            EE_ENGINE_COMPARE(ee, ELM_SOFTWARE_SDL) ||
            EE_ENGINE_COMPARE(ee, ELM_SOFTWARE_16_SDL) ||
            EE_ENGINE_COMPARE(ee, ELM_OPENGL_SDL) ||
            EE_ENGINE_COMPARE(ee, ELM_OPENGL_COCOA))
     {
     }
   else if (EE_ENGINE_COMPARE(ee, ELM_SOFTWARE_16_X11))
     {
        if (ee) xwin = ecore_evas_software_x11_16_window_get(ee);
     }
   else if (EE_ENGINE_COMPARE(ee, ELM_SOFTWARE_8_X11))
     {
        if (ee) xwin = ecore_evas_software_x11_8_window_get(ee);
     }
   else if (EE_ENGINE_COMPARE(ee, ELM_OPENGL_X11))
     {
        if (ee) xwin = ecore_evas_gl_x11_window_get(ee);
     }
   else if (EE_ENGINE_COMPARE(ee, ELM_SOFTWARE_WIN32))
     {
        if (ee) xwin = (long)ecore_evas_win32_window_get(ee);
     }
   return xwin;

#endif
   return 0;
}

#ifdef HAVE_ELEMENTARY_X
static void
_elm_win_xwindow_get(Elm_Win_Smart_Data *sd)
{
   sd->x.xwin = _elm_ee_xwin_get(sd->ee);
}

#endif

#ifdef HAVE_ELEMENTARY_X
static void
_elm_win_xwin_update(Elm_Win_Smart_Data *sd)
{
   const char *s;

   _elm_win_xwindow_get(sd);
   if (sd->parent)
     {
        ELM_WIN_DATA_GET(sd->parent, sdp);
        if (sdp)
          {
             if (sd->x.xwin)
               ecore_x_icccm_transient_for_set(sd->x.xwin, sdp->x.xwin);
          }
     }

   if (!sd->x.xwin) return;  /* nothing more to do */

   s = sd->title;
   if (!s) s = _elm_appname;
   if (!s) s = "";
   if (sd->icon_name) s = sd->icon_name;
   ecore_x_icccm_icon_name_set(sd->x.xwin, s);
   ecore_x_netwm_icon_name_set(sd->x.xwin, s);

   s = sd->role;
   if (s) ecore_x_icccm_window_role_set(sd->x.xwin, s);

   // set window icon
   if (sd->icon)
     {
        void *data;

        data = evas_object_image_data_get(sd->icon, EINA_FALSE);
        if (data)
          {
             Ecore_X_Icon ic;
             int w = 0, h = 0, stride, x, y;
             unsigned char *p;
             unsigned int *p2;

             evas_object_image_size_get(sd->icon, &w, &h);
             stride = evas_object_image_stride_get(sd->icon);
             if ((w > 0) && (h > 0) &&
                 (stride >= (int)(w * sizeof(unsigned int))))
               {
                  ic.width = w;
                  ic.height = h;
                  ic.data = malloc(w * h * sizeof(unsigned int));

                  if (ic.data)
                    {
                       p = (unsigned char *)data;
                       p2 = (unsigned int *)ic.data;
                       for (y = 0; y < h; y++)
                         {
                            for (x = 0; x < w; x++)
                              {
                                 *p2 = *((unsigned int *)p);
                                 p += sizeof(unsigned int);
                                 p2++;
                              }
                            p += (stride - (w * sizeof(unsigned int)));
                         }
                       ecore_x_netwm_icons_set(sd->x.xwin, &ic, 1);
                       free(ic.data);
                    }
               }
             evas_object_image_data_set(sd->icon, data);
          }
     }

   switch (sd->type)
     {
      case ELM_WIN_BASIC:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_NORMAL);
        break;

      case ELM_WIN_DIALOG_BASIC:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_DIALOG);
        break;

      case ELM_WIN_DESKTOP:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_DESKTOP);
        break;

      case ELM_WIN_DOCK:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_DOCK);
        break;

      case ELM_WIN_TOOLBAR:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_TOOLBAR);
        break;

      case ELM_WIN_MENU:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_MENU);
        break;

      case ELM_WIN_UTILITY:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_UTILITY);
        break;

      case ELM_WIN_SPLASH:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_SPLASH);
        break;

      case ELM_WIN_DROPDOWN_MENU:
        ecore_x_netwm_window_type_set
          (sd->x.xwin, ECORE_X_WINDOW_TYPE_DROPDOWN_MENU);
        break;

      case ELM_WIN_POPUP_MENU:
        ecore_x_netwm_window_type_set
          (sd->x.xwin, ECORE_X_WINDOW_TYPE_POPUP_MENU);
        break;

      case ELM_WIN_TOOLTIP:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_TOOLTIP);
        break;

      case ELM_WIN_NOTIFICATION:
        ecore_x_netwm_window_type_set
          (sd->x.xwin, ECORE_X_WINDOW_TYPE_NOTIFICATION);
        break;

      case ELM_WIN_COMBO:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_COMBO);
        break;

      case ELM_WIN_DND:
        ecore_x_netwm_window_type_set(sd->x.xwin, ECORE_X_WINDOW_TYPE_DND);
        break;

      default:
        break;
     }
   ecore_x_e_virtual_keyboard_state_set
     (sd->x.xwin, (Ecore_X_Virtual_Keyboard_State)sd->kbdmode);
   if (sd->indmode == ELM_WIN_INDICATOR_SHOW)
     ecore_x_e_illume_indicator_state_set
       (sd->x.xwin, ECORE_X_ILLUME_INDICATOR_STATE_ON);
   else if (sd->indmode == ELM_WIN_INDICATOR_HIDE)
     ecore_x_e_illume_indicator_state_set
       (sd->x.xwin, ECORE_X_ILLUME_INDICATOR_STATE_OFF);
}

#endif

static void
_elm_win_resize_objects_eval(Evas_Object *obj)
{
   const Eina_List *l;
   const Evas_Object *child;

   ELM_WIN_DATA_GET(obj, sd);
   Evas_Coord w, h, minw = -1, minh = -1, maxw = -1, maxh = -1;
   int xx = 1, xy = 1;
   double wx, wy;

   EINA_LIST_FOREACH(sd->resize_objs, l, child)
     {
        evas_object_size_hint_weight_get(child, &wx, &wy);
        if (wx == 0.0) xx = 0;
        if (wy == 0.0) xy = 0;

        evas_object_size_hint_min_get(child, &w, &h);
        if (w < 1) w = 1;
        if (h < 1) h = 1;
        if (w > minw) minw = w;
        if (h > minh) minh = h;

        evas_object_size_hint_max_get(child, &w, &h);
        if (w < 1) w = -1;
        if (h < 1) h = -1;
        if (maxw == -1) maxw = w;
        else if ((w > 0) && (w < maxw))
          maxw = w;
        if (maxh == -1) maxh = h;
        else if ((h > 0) && (h < maxh))
          maxh = h;
     }
   if (!xx) maxw = minw;
   else maxw = 32767;
   if (!xy) maxh = minh;
   else maxh = 32767;
   evas_object_size_hint_min_set(obj, minw, minh);
   evas_object_size_hint_max_set(obj, maxw, maxh);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if (w < minw) w = minw;
   if (h < minh) h = minh;
   if ((maxw >= 0) && (w > maxw)) w = maxw;
   if ((maxh >= 0) && (h > maxh)) h = maxh;
   evas_object_move(obj, 0, 0);
   evas_object_resize(obj, w + sd->fw, h + sd->fh);
}

static void
_elm_win_on_resize_obj_del(void *data,
                           Evas *e __UNUSED__,
                           Evas_Object *obj __UNUSED__,
                           void *event_info __UNUSED__)
{
   ELM_WIN_DATA_GET(data, sd);

   sd->resize_objs = eina_list_remove(sd->resize_objs, obj);

   _elm_win_resize_objects_eval(data);
}

static void
_elm_win_on_resize_obj_changed_size_hints(void *data,
                                          Evas *e __UNUSED__,
                                          Evas_Object *obj __UNUSED__,
                                          void *event_info __UNUSED__)
{
   _elm_win_resize_objects_eval(data);
}

void
_elm_win_shutdown(void)
{
   while (_elm_win_list) evas_object_del(_elm_win_list->data);
   if (_elm_win_state_eval_job)
     {
        ecore_job_del(_elm_win_state_eval_job);
        _elm_win_state_eval_job = NULL;
     }
}

void
_elm_win_rescale(Elm_Theme *th,
                 Eina_Bool use_theme)
{
   const Eina_List *l;
   Evas_Object *obj;

   if (!use_theme)
     {
        EINA_LIST_FOREACH(_elm_win_list, l, obj)
          elm_widget_theme(obj);
     }
   else
     {
        EINA_LIST_FOREACH(_elm_win_list, l, obj)
          elm_widget_theme_specific(obj, th, EINA_FALSE);
     }
}

void
_elm_win_access(Eina_Bool is_access)
{
   const Eina_List *l;
   Evas_Object *obj;

   EINA_LIST_FOREACH(_elm_win_list, l, obj)
     elm_widget_access(obj, is_access);
}

void
_elm_win_translate(void)
{
   const Eina_List *l;
   Evas_Object *obj;

   EINA_LIST_FOREACH(_elm_win_list, l, obj)
     elm_widget_translate(obj);
}

#ifdef HAVE_ELEMENTARY_X
static Eina_Bool
_elm_win_client_message(void *data,
                        int type __UNUSED__,
                        void *event)
{
   Elm_Win_Smart_Data *sd = data;
   Ecore_X_Event_Client_Message *e = event;

   if (e->format != 32) return ECORE_CALLBACK_PASS_ON;
   if (e->message_type == ECORE_X_ATOM_E_COMP_FLUSH)
     {
        if ((unsigned int)e->data.l[0] == sd->x.xwin)
          {
             Evas *evas = evas_object_evas_get(sd->obj);
             if (evas)
               {
                  edje_file_cache_flush();
                  edje_collection_cache_flush();
                  evas_image_cache_flush(evas);
                  evas_font_cache_flush(evas);
               }
          }
     }
   else if (e->message_type == ECORE_X_ATOM_E_COMP_DUMP)
     {
        if ((unsigned int)e->data.l[0] == sd->x.xwin)
          {
             Evas *evas = evas_object_evas_get(sd->obj);
             if (evas)
               {
                  edje_file_cache_flush();
                  edje_collection_cache_flush();
                  evas_image_cache_flush(evas);
                  evas_font_cache_flush(evas);
                  evas_render_dump(evas);
               }
          }
     }
   else if (e->message_type == ECORE_X_ATOM_E_ILLUME_ACCESS_CONTROL)
     {
        if ((unsigned int)e->data.l[0] == sd->x.xwin)
          {
             if ((unsigned int)e->data.l[1] ==
                 ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_NEXT)
               {
                  // XXX: call right access func
               }
             else if ((unsigned int)e->data.l[1] ==
                      ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_PREV)
               {
                  // XXX: call right access func
               }
             else if ((unsigned int)e->data.l[1] ==
                      ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_ACTIVATE)
               {
                  _elm_access_highlight_object_activate
                    (sd->obj, ELM_ACTIVATE_DEFAULT);
               }
             else if ((unsigned int)e->data.l[1] ==
                      ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_READ)
               {
                  /* there would be better way to read highlight object */
                  ecore_x_mouse_in_send(sd->x.xwin, e->data.l[2], e->data.l[3]);
                  ecore_x_mouse_move_send(sd->x.xwin, e->data.l[2], e->data.l[3]);
               }
             else if ((unsigned int)e->data.l[1] ==
                      ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_READ_NEXT)
               {
                  _elm_access_highlight_cycle(sd->obj, ELM_FOCUS_NEXT);
               }
             else if ((unsigned int)e->data.l[1] ==
                      ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_READ_PREV)
               {
                  _elm_access_highlight_cycle(sd->obj, ELM_FOCUS_PREVIOUS);
               }
             else if ((unsigned int)e->data.l[1] ==
                      ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_UP)
               {
                  _elm_access_highlight_object_activate
                    (sd->obj, ELM_ACTIVATE_UP);
               }
             else if ((unsigned int)e->data.l[1] ==
                      ECORE_X_ATOM_E_ILLUME_ACCESS_ACTION_DOWN)
               {
                  _elm_access_highlight_object_activate
                    (sd->obj, ELM_ACTIVATE_DOWN);
               }
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool
_elm_win_property_change(void *data,
                         int type __UNUSED__,
                         void *event)
{
   Elm_Win_Smart_Data *sd = data;
   Ecore_X_Event_Window_Property *e = event;

   if (e->atom == ECORE_X_ATOM_E_ILLUME_INDICATOR_STATE)
     {
        if (e->win == sd->x.xwin)
          {
             sd->indmode = ecore_x_e_illume_indicator_state_get(e->win);
             evas_object_smart_callback_call(sd->obj, SIG_INDICATOR_PROP_CHANGED, NULL);
          }
     }
   return ECORE_CALLBACK_PASS_ON;
}
#endif

static void
_elm_win_focus_highlight_hide(void *data __UNUSED__,
                              Evas_Object *obj,
                              const char *emission __UNUSED__,
                              const char *source __UNUSED__)
{
   evas_object_hide(obj);
}

static void
_elm_win_focus_highlight_anim_end(void *data,
                                  Evas_Object *obj,
                                  const char *emission __UNUSED__,
                                  const char *source __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;

   _elm_win_focus_highlight_simple_setup(sd, obj);
}

static void
_elm_win_focus_highlight_init(Elm_Win_Smart_Data *sd)
{
   evas_event_callback_add(sd->evas, EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_IN,
                           _elm_win_object_focus_in, sd);
   evas_event_callback_add(sd->evas,
                           EVAS_CALLBACK_CANVAS_OBJECT_FOCUS_OUT,
                           _elm_win_object_focus_out, sd);

   sd->focus_highlight.cur.target = evas_focus_get(sd->evas);

   sd->focus_highlight.top = edje_object_add(sd->evas);
   sd->focus_highlight.changed_theme = EINA_TRUE;
   edje_object_signal_callback_add(sd->focus_highlight.top,
                                   "elm,action,focus,hide,end", "",
                                   _elm_win_focus_highlight_hide, NULL);
   edje_object_signal_callback_add(sd->focus_highlight.top,
                                   "elm,action,focus,anim,end", "",
                                   _elm_win_focus_highlight_anim_end, sd);
   _elm_win_focus_highlight_reconfigure_job_start(sd);
}

static void
_elm_win_frame_cb_move_start(void *data,
                             Evas_Object *obj __UNUSED__,
                             const char *sig __UNUSED__,
                             const char *source __UNUSED__)
{
   Elm_Win_Smart_Data *sd;

   if (!(sd = data)) return;
   /* FIXME: Change mouse pointer */

   /* NB: Wayland handles moving surfaces by itself so we cannot
    * specify a specific x/y we want. Instead, we will pass in the
    * existing x/y values so they can be recorded as 'previous'
    * position. The new position will get updated automatically when
    * the move is finished */

   ecore_evas_wayland_move(sd->ee, sd->screen.x, sd->screen.y);
}

static void
_elm_win_frame_cb_resize_show(void *data,
                              Evas_Object *obj __UNUSED__,
                              const char *sig __UNUSED__,
                              const char *source)
{
   Elm_Win_Smart_Data *sd;

   if (!(sd = data)) return;
   if (sd->resizing) return;

#ifdef HAVE_ELEMENTARY_WAYLAND
   if (!strcmp(source, "elm.event.resize.t"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win, ELM_CURSOR_TOP_SIDE);
   else if (!strcmp(source, "elm.event.resize.b"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win, ELM_CURSOR_BOTTOM_SIDE);
   else if (!strcmp(source, "elm.event.resize.l"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win, ELM_CURSOR_LEFT_SIDE);
   else if (!strcmp(source, "elm.event.resize.r"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win, ELM_CURSOR_RIGHT_SIDE);
   else if (!strcmp(source, "elm.event.resize.tl"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win,
                                          ELM_CURSOR_TOP_LEFT_CORNER);
   else if (!strcmp(source, "elm.event.resize.tr"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win,
                                          ELM_CURSOR_TOP_RIGHT_CORNER);
   else if (!strcmp(source, "elm.event.resize.bl"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win,
                                          ELM_CURSOR_BOTTOM_LEFT_CORNER);
   else if (!strcmp(source, "elm.event.resize.br"))
     ecore_wl_window_cursor_from_name_set(sd->wl.win,
                                          ELM_CURSOR_BOTTOM_RIGHT_CORNER);
   else
     ecore_wl_window_cursor_default_restore(sd->wl.win);
#else
   (void)source;
#endif
}

static void
_elm_win_frame_cb_resize_hide(void *data,
                              Evas_Object *obj __UNUSED__,
                              const char *sig __UNUSED__,
                              const char *source __UNUSED__)
{
   Elm_Win_Smart_Data *sd;

   if (!(sd = data)) return;
   if (sd->resizing) return;

#ifdef HAVE_ELEMENTARY_WAYLAND
   ecore_wl_window_cursor_default_restore(sd->wl.win);
#endif
}

static void
_elm_win_frame_cb_resize_start(void *data,
                               Evas_Object *obj __UNUSED__,
                               const char *sig __UNUSED__,
                               const char *source)
{
   Elm_Win_Smart_Data *sd;

   if (!(sd = data)) return;
   if (sd->resizing) return;

   sd->resizing = EINA_TRUE;

   if (!strcmp(source, "elm.event.resize.t"))
     sd->resize_location = 1;
   else if (!strcmp(source, "elm.event.resize.b"))
     sd->resize_location = 2;
   else if (!strcmp(source, "elm.event.resize.l"))
     sd->resize_location = 4;
   else if (!strcmp(source, "elm.event.resize.r"))
     sd->resize_location = 8;
   else if (!strcmp(source, "elm.event.resize.tl"))
     sd->resize_location = 5;
   else if (!strcmp(source, "elm.event.resize.tr"))
     sd->resize_location = 9;
   else if (!strcmp(source, "elm.event.resize.bl"))
     sd->resize_location = 6;
   else if (!strcmp(source, "elm.event.resize.br"))
     sd->resize_location = 10;
   else
     sd->resize_location = 0;

   if (sd->resize_location > 0)
     ecore_evas_wayland_resize(sd->ee, sd->resize_location);
}

static void
_elm_win_frame_cb_minimize(void *data,
                           Evas_Object *obj __UNUSED__,
                           const char *sig __UNUSED__,
                           const char *source __UNUSED__)
{
   Elm_Win_Smart_Data *sd;

   if (!(sd = data)) return;
   sd->iconified = EINA_TRUE;
   TRAP(sd, iconified_set, EINA_TRUE);
}

static void
_elm_win_frame_cb_maximize(void *data,
                           Evas_Object *obj __UNUSED__,
                           const char *sig __UNUSED__,
                           const char *source __UNUSED__)
{
   Elm_Win_Smart_Data *sd;

   if (!(sd = data)) return;
   if (sd->maximized) sd->maximized = EINA_FALSE;
   else sd->maximized = EINA_TRUE;
   TRAP(sd, maximized_set, sd->maximized);
}

static void
_elm_win_frame_cb_close(void *data,
                        Evas_Object *obj __UNUSED__,
                        const char *sig __UNUSED__,
                        const char *source __UNUSED__)
{
   Elm_Win_Smart_Data *sd;
   Evas_Object *win;

   /* FIXME: After the current freeze, this should be handled differently.
    *
    * Ideally, we would want to mimic the X11 backend and use something
    * like ECORE_WL_EVENT_WINDOW_DELETE and handle the delete_request
    * inside of ecore_evas. That would be the 'proper' way, but since we are
    * in a freeze right now, I cannot add a new event value, or a new
    * event structure to ecore_wayland.
    *
    * So yes, this is a temporary 'stop-gap' solution which will be fixed
    * when the freeze is over, but it does fix a trac bug for now, and in a
    * way which does not break API or the freeze. - dh
    */

   if (!(sd = data)) return;

   win = sd->obj;

   int autodel = sd->autodel;
   sd->autodel_clear = &autodel;
   evas_object_ref(win);
   evas_object_smart_callback_call(win, SIG_DELETE_REQUEST, NULL);
   // FIXME: if above callback deletes - then the below will be invalid
   if (autodel) evas_object_del(win);
   else sd->autodel_clear = NULL;
   evas_object_unref(win);
}

static void
_elm_win_frame_add(Elm_Win_Smart_Data *sd,
                   const char *style)
{
   Evas_Object *obj = sd->obj;
   short layer;

   // FIXME: Don't use hardcoded framespace values, get it from theme
   sd->fx = 0;
   sd->fy = 22;
   sd->fw = 0;
   sd->fh = 26;

   sd->frame_obj = edje_object_add(sd->evas);
   layer = evas_object_layer_get(obj);
   evas_object_layer_set(sd->frame_obj, layer + 1);
   elm_widget_theme_object_set
     (sd->obj, sd->frame_obj, "border", "base", style);

   evas_object_is_frame_object_set(sd->frame_obj, EINA_TRUE);
   evas_object_move(sd->frame_obj, 0, 0);
   evas_object_resize(sd->frame_obj, 1, 1);

   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,move,start", "elm",
     _elm_win_frame_cb_move_start, sd);
   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,resize,show", "*",
     _elm_win_frame_cb_resize_show, sd);
   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,resize,hide", "*",
     _elm_win_frame_cb_resize_hide, sd);
   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,resize,start", "*",
     _elm_win_frame_cb_resize_start, sd);
   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,minimize", "elm",
     _elm_win_frame_cb_minimize, sd);
   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,maximize", "elm",
     _elm_win_frame_cb_maximize, sd);
   edje_object_signal_callback_add
     (sd->frame_obj, "elm,action,close", "elm", _elm_win_frame_cb_close, sd);

   if (sd->title)
     {
        edje_object_part_text_escaped_set
          (sd->frame_obj, "elm.text.title", sd->title);
     }
}

static void
_elm_win_frame_del(Elm_Win_Smart_Data *sd)
{
   if (sd->frame_obj)
     {
        edje_object_signal_callback_del
          (sd->frame_obj, "elm,action,move,start", "elm",
              _elm_win_frame_cb_move_start);
        edje_object_signal_callback_del
          (sd->frame_obj, "elm,action,resize,show", "*",
              _elm_win_frame_cb_resize_show);
        edje_object_signal_callback_del
          (sd->frame_obj, "elm,action,resize,hide", "*",
              _elm_win_frame_cb_resize_hide);
        edje_object_signal_callback_del
          (sd->frame_obj, "elm,action,resize,start", "*",
              _elm_win_frame_cb_resize_start);
        edje_object_signal_callback_del
          (sd->frame_obj, "elm,action,minimize", "elm",
              _elm_win_frame_cb_minimize);
        edje_object_signal_callback_del
          (sd->frame_obj, "elm,action,maximize", "elm",
              _elm_win_frame_cb_maximize);
        edje_object_signal_callback_del
          (sd->frame_obj, "elm,action,close", "elm",
              _elm_win_frame_cb_close);

        evas_object_del(sd->frame_obj);
        sd->frame_obj = NULL;
     }
}

#ifdef ELM_DEBUG
static void
_debug_key_down(void *data __UNUSED__,
                Evas *e __UNUSED__,
                Evas_Object *obj,
                void *event_info)
{
   Evas_Event_Key_Down *ev = event_info;

   if (ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD)
     return;

   if ((strcmp(ev->keyname, "F12")) ||
       (!evas_key_modifier_is_set(ev->modifiers, "Control")))
     return;

   printf("Tree graph generated.\n");
   elm_object_tree_dot_dump(obj, "./dump.dot");
}

#endif

static void
_win_img_hide(void *data,
              Evas *e __UNUSED__,
              Evas_Object *obj __UNUSED__,
              void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;

   elm_widget_focus_hide_handle(sd->obj);
}

static void
_win_img_mouse_up(void *data,
                  Evas *e __UNUSED__,
                  Evas_Object *obj __UNUSED__,
                  void *event_info)
{
   Elm_Win_Smart_Data *sd = data;
   Evas_Event_Mouse_Up *ev = event_info;
   if (!(ev->event_flags & EVAS_EVENT_FLAG_ON_HOLD))
     elm_widget_focus_mouse_up_handle(sd->obj);
}

static void
_win_img_focus_in(void *data,
                  Evas *e __UNUSED__,
                  Evas_Object *obj __UNUSED__,
                  void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;
   elm_widget_focus_steal(sd->obj);
}

static void
_win_img_focus_out(void *data,
                   Evas *e __UNUSED__,
                   Evas_Object *obj __UNUSED__,
                   void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;
   elm_widget_focused_object_clear(sd->obj);
}

static void
_win_inlined_image_set(Elm_Win_Smart_Data *sd)
{
   evas_object_image_alpha_set(sd->img_obj, EINA_FALSE);
   evas_object_image_filled_set(sd->img_obj, EINA_TRUE);

   evas_object_event_callback_add
     (sd->img_obj, EVAS_CALLBACK_DEL, _elm_win_on_img_obj_del, sd);
   evas_object_event_callback_add
     (sd->img_obj, EVAS_CALLBACK_HIDE, _win_img_hide, sd);
   evas_object_event_callback_add
     (sd->img_obj, EVAS_CALLBACK_MOUSE_UP, _win_img_mouse_up, sd);
   evas_object_event_callback_add
     (sd->img_obj, EVAS_CALLBACK_FOCUS_IN, _win_img_focus_in, sd);
   evas_object_event_callback_add
     (sd->img_obj, EVAS_CALLBACK_FOCUS_OUT, _win_img_focus_out, sd);
}

static void
_elm_win_on_icon_del(void *data,
                     Evas *e __UNUSED__,
                     Evas_Object *obj,
                     void *event_info __UNUSED__)
{
   Elm_Win_Smart_Data *sd = data;

   if (sd->icon == obj) sd->icon = NULL;
}

static void
_elm_win_smart_add(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, evas_obj_smart_add());

   elm_widget_can_focus_set(obj, EINA_TRUE);

   elm_widget_highlight_ignore_set(obj, EINA_TRUE);
}

#ifdef HAVE_ELEMENTARY_X
static void
_elm_x_io_err(void *data __UNUSED__)
{
   Eina_List *l;
   Evas_Object *obj;

   EINA_LIST_FOREACH(_elm_win_list, l, obj)
     evas_object_smart_callback_call(obj, SIG_IOERR, NULL);
   elm_exit();
}
#endif

EAPI Evas_Object *
elm_win_add(Evas_Object *parent,
            const char *name,
            Elm_Win_Type type)
{
   Evas_Object *obj = eo_add_custom(MY_CLASS, parent, elm_obj_win_constructor(name, type));
   eo_unref(obj);
   return obj;
}

static void
_win_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list)
{
   Elm_Win_Smart_Data *sd = _pd;
   sd->obj = obj; // in ctor

   const char *name = va_arg(*list, const char *);
   Elm_Win_Type type = va_arg(*list, Elm_Win_Type);
   Evas_Object *parent = eo_parent_get(obj);
   Evas *e;
   const Eina_List *l;
   const char *fontpath, *fallback = NULL;

   Elm_Win_Smart_Data tmp_sd;

   /* just to store some data while trying out to create a canvas */
   memset(&tmp_sd, 0, sizeof(Elm_Win_Smart_Data));

#define FALLBACK_TRY(engine)                                      \
  if (!tmp_sd.ee) {                                               \
     CRITICAL(engine " engine creation failed. Trying default."); \
  } while (0)
#define FALLBACK_STORE(engine)                               \
   if (tmp_sd.ee)                                            \
   {                                                         \
      CRITICAL(engine "Fallback to %s successful.", engine); \
      fallback = engine;                                     \
   }

   switch (type)
     {
      case ELM_WIN_INLINED_IMAGE:
        if (!parent) break;
        {
           e = evas_object_evas_get(parent);
           Ecore_Evas *ee;

           if (!e) break;

           ee = ecore_evas_ecore_evas_get(e);
           if (!ee) break;

           tmp_sd.img_obj = ecore_evas_object_image_new(ee);
           if (!tmp_sd.img_obj) break;

           tmp_sd.ee = ecore_evas_object_ecore_evas_get(tmp_sd.img_obj);
           if (!tmp_sd.ee)
             {
                evas_object_del(tmp_sd.img_obj);
                tmp_sd.img_obj = NULL;
             }
        }
        break;

      case ELM_WIN_SOCKET_IMAGE:
        tmp_sd.ee = ecore_evas_extn_socket_new(1, 1);
        break;

      default:
        if (ENGINE_COMPARE(ELM_SOFTWARE_X11))
          {
             tmp_sd.ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
             FALLBACK_TRY("Software X11");
             if (!tmp_sd.ee)
               {
                  tmp_sd.ee = ecore_evas_fb_new(NULL, 0, 1, 1);
                  FALLBACK_STORE("Software FB");
               }
          }
        else if (ENGINE_COMPARE(ELM_SOFTWARE_FB))
          {
             tmp_sd.ee = ecore_evas_fb_new(NULL, 0, 1, 1);
             FALLBACK_TRY("Software FB");
             if (!tmp_sd.ee)
               {
                  tmp_sd.ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
                  FALLBACK_STORE("Software X11");
               }
          }
        else if (ENGINE_COMPARE(ELM_SOFTWARE_DIRECTFB))
          {
             tmp_sd.ee = ecore_evas_directfb_new(NULL, 1, 0, 0, 1, 1);
             FALLBACK_TRY("Software DirectFB");
             if (!tmp_sd.ee)
               {
                  tmp_sd.ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
                  FALLBACK_STORE("Software X11");
                  if (!tmp_sd.ee)
                    {
                       tmp_sd.ee = ecore_evas_fb_new(NULL, 0, 1, 1);
                       FALLBACK_STORE("Software FB");
                    }
               }
          }
        else if (ENGINE_COMPARE(ELM_SOFTWARE_16_X11))
          {
             tmp_sd.ee = ecore_evas_software_x11_16_new(NULL, 0, 0, 0, 1, 1);
             FALLBACK_TRY("Software-16");
             if (!tmp_sd.ee)
               {
                  tmp_sd.ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
                  FALLBACK_STORE("Software X11");
                  if (!tmp_sd.ee)
                    {
                       tmp_sd.ee = ecore_evas_fb_new(NULL, 0, 1, 1);
                       FALLBACK_STORE("Software FB");
                    }
               }
          }
        else if (ENGINE_COMPARE(ELM_SOFTWARE_8_X11))
          {
             tmp_sd.ee = ecore_evas_software_x11_8_new(NULL, 0, 0, 0, 1, 1);
             FALLBACK_TRY("Software-8");
             if (!tmp_sd.ee)
               {
                  tmp_sd.ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
                  FALLBACK_STORE("Software X11");
                  if (!tmp_sd.ee)
                    {
                       tmp_sd.ee = ecore_evas_fb_new(NULL, 0, 1, 1);
                       FALLBACK_STORE("Software FB");
                    }
               }
          }
        else if (ENGINE_COMPARE(ELM_OPENGL_X11))
          {
             int opt[10];
             int opt_i = 0;

             if (_elm_config->vsync)
               {
                  opt[opt_i] = ECORE_EVAS_GL_X11_OPT_VSYNC;
                  opt_i++;
                  opt[opt_i] = 1;
                  opt_i++;
               }
             if (opt_i > 0)
               tmp_sd.ee = ecore_evas_gl_x11_options_new
                   (NULL, 0, 0, 0, 1, 1, opt);
             else
               tmp_sd.ee = ecore_evas_gl_x11_new(NULL, 0, 0, 0, 1, 1);
             FALLBACK_TRY("OpenGL");
             if (!tmp_sd.ee)
               {
                  tmp_sd.ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
                  FALLBACK_STORE("Software X11");
                  if (!tmp_sd.ee)
                    {
                       tmp_sd.ee = ecore_evas_fb_new(NULL, 0, 1, 1);
                       FALLBACK_STORE("Software FB");
                    }
               }
          }
        else if (ENGINE_COMPARE(ELM_SOFTWARE_WIN32))
          {
             tmp_sd.ee = ecore_evas_software_gdi_new(NULL, 0, 0, 1, 1);
             FALLBACK_TRY("Software Win32");
          }
        else if (ENGINE_COMPARE(ELM_SOFTWARE_16_WINCE))
          {
             tmp_sd.ee = ecore_evas_software_wince_gdi_new(NULL, 0, 0, 1, 1);
             FALLBACK_TRY("Software-16-WinCE");
          }
        else if (ENGINE_COMPARE(ELM_SOFTWARE_PSL1GHT))
          {
             tmp_sd.ee = ecore_evas_psl1ght_new(NULL, 1, 1);
             FALLBACK_TRY("PSL1GHT");
          }
        else if (ENGINE_COMPARE(ELM_SOFTWARE_SDL))
          {
             tmp_sd.ee = ecore_evas_sdl_new(NULL, 0, 0, 0, 0, 0, 1);
             FALLBACK_TRY("Software SDL");
             if (!tmp_sd.ee)
               {
                  tmp_sd.ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
                  FALLBACK_STORE("Software X11");
                  if (!tmp_sd.ee)
                    {
                       tmp_sd.ee = ecore_evas_fb_new(NULL, 0, 1, 1);
                       FALLBACK_STORE("Software FB");
                    }
               }
          }
        else if (ENGINE_COMPARE(ELM_SOFTWARE_16_SDL))
          {
             tmp_sd.ee = ecore_evas_sdl16_new(NULL, 0, 0, 0, 0, 0, 1);
             FALLBACK_TRY("Software-16-SDL");
             if (!tmp_sd.ee)
               {
                  tmp_sd.ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
                  FALLBACK_STORE("Software X11");
                  if (!tmp_sd.ee)
                    {
                       tmp_sd.ee = ecore_evas_fb_new(NULL, 0, 1, 1);
                       FALLBACK_STORE("Software FB");
                    }
               }
          }
        else if (ENGINE_COMPARE(ELM_OPENGL_SDL))
          {
             tmp_sd.ee = ecore_evas_gl_sdl_new(NULL, 1, 1, 0, 0);
             FALLBACK_TRY("OpenGL SDL");
             if (!tmp_sd.ee)
               {
                  tmp_sd.ee = ecore_evas_software_x11_new(NULL, 0, 0, 0, 1, 1);
                  FALLBACK_STORE("Software X11");
                  if (!tmp_sd.ee)
                    {
                       tmp_sd.ee = ecore_evas_fb_new(NULL, 0, 1, 1);
                       FALLBACK_STORE("Software FB");
                    }
               }
          }
        else if (ENGINE_COMPARE(ELM_OPENGL_COCOA))
          {
             tmp_sd.ee = ecore_evas_cocoa_new(NULL, 1, 1, 0, 0);
             FALLBACK_TRY("OpenGL Cocoa");
          }
        else if (ENGINE_COMPARE(ELM_BUFFER))
          {
             tmp_sd.ee = ecore_evas_buffer_new(1, 1);
          }
        else if (ENGINE_COMPARE(ELM_EWS))
          {
             tmp_sd.ee = ecore_evas_ews_new(0, 0, 1, 1);
          }
        else if (ENGINE_COMPARE(ELM_WAYLAND_SHM))
          {
             tmp_sd.ee = ecore_evas_wayland_shm_new(NULL, 0, 0, 0, 1, 1, 0);
          }
        else if (ENGINE_COMPARE(ELM_WAYLAND_EGL))
          {
             tmp_sd.ee = ecore_evas_wayland_egl_new(NULL, 0, 0, 0, 1, 1, 0);
          }
        else if (!strncmp(ENGINE_GET(), "shot:", 5))
          {
             tmp_sd.ee = ecore_evas_buffer_new(1, 1);
             ecore_evas_manual_render_set(tmp_sd.ee, EINA_TRUE);
             tmp_sd.shot.info = eina_stringshare_add
                 (ENGINE_GET() + 5);
          }
#undef FALLBACK_TRY
        break;
     }

   if (!tmp_sd.ee)
     {
        ERR("Cannot create window.");
        eo_error_set(obj);
        return;
     }

   eo_parent_set(obj, ecore_evas_get(tmp_sd.ee));
   eo_do_super(obj, eo_constructor());
   eo_do(obj,
         evas_obj_type_set(MY_CLASS_NAME),
         evas_obj_smart_callbacks_descriptions_set(_smart_callbacks, NULL));

   /* copying possibly altered fields back */
#define SD_CPY(_field)             \
  do                               \
    {                              \
       sd->_field = tmp_sd._field; \
    } while (0)

   SD_CPY(ee);
   SD_CPY(img_obj);
   SD_CPY(shot.info);
#undef SD_CPY

   if ((trap) && (trap->add))
     sd->trap_data = trap->add(obj);

   /* complementary actions, which depend on final smart data
    * pointer */
   if (type == ELM_WIN_INLINED_IMAGE)
     _win_inlined_image_set(sd);

#ifdef HAVE_ELEMENTARY_X
   else if (ENGINE_COMPARE(ELM_SOFTWARE_X11) ||
            ENGINE_COMPARE(ELM_SOFTWARE_16_X11) ||
            ENGINE_COMPARE(ELM_SOFTWARE_8_X11) ||
            ENGINE_COMPARE(ELM_OPENGL_X11))
     {
        sd->x.client_message_handler = ecore_event_handler_add
            (ECORE_X_EVENT_CLIENT_MESSAGE, _elm_win_client_message, sd);
        sd->x.property_handler = ecore_event_handler_add
            (ECORE_X_EVENT_WINDOW_PROPERTY, _elm_win_property_change, sd);
     }
#endif

   else if (!strncmp(ENGINE_GET(), "shot:", 5))
     _shot_init(sd);

   sd->kbdmode = ELM_WIN_KEYBOARD_UNKNOWN;
   sd->indmode = ELM_WIN_INDICATOR_UNKNOWN;

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     {
        ecore_x_io_error_handler_set(_elm_x_io_err, NULL);
     }
#endif

#ifdef HAVE_ELEMENTARY_WAYLAND
   if ((ENGINE_COMPARE(ELM_WAYLAND_SHM)) || (ENGINE_COMPARE(ELM_WAYLAND_EGL)))
     sd->wl.win = ecore_evas_wayland_window_get(sd->ee);
#endif

   if ((_elm_config->bgpixmap)
#ifdef HAVE_ELEMENTARY_X
       &&
       (((sd->x.xwin) && (!ecore_x_screen_is_composited(0))) ||
           (!sd->x.xwin)))
#else
      )
#endif
     TRAP(sd, avoid_damage_set, ECORE_EVAS_AVOID_DAMAGE_EXPOSE);
   // bg pixmap done by x - has other issues like can be redrawn by x before it
   // is filled/ready by app
   //     TRAP(sd, avoid_damage_set, ECORE_EVAS_AVOID_DAMAGE_BUILT_IN);

   sd->type = type;
   sd->parent = parent;

   if (sd->parent)
     evas_object_event_callback_add
       (sd->parent, EVAS_CALLBACK_DEL, _elm_win_on_parent_del, sd);

   sd->evas = ecore_evas_get(sd->ee);

   evas_object_color_set(obj, 0, 0, 0, 0);
   evas_object_move(obj, 0, 0);
   evas_object_resize(obj, 1, 1);
   evas_object_layer_set(obj, 50);
   evas_object_pass_events_set(obj, EINA_TRUE);

   if (type == ELM_WIN_INLINED_IMAGE)
     elm_widget_parent2_set(obj, parent);

   /* use own version of ecore_evas_object_associate() that does TRAP() */
   ecore_evas_data_set(sd->ee, "elm_win", sd);

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
      _elm_win_obj_callback_changed_size_hints, sd);

   evas_object_intercept_raise_callback_add
     (obj, _elm_win_obj_intercept_raise, sd);
   evas_object_intercept_lower_callback_add
     (obj, _elm_win_obj_intercept_lower, sd);
   evas_object_intercept_stack_above_callback_add
     (obj, _elm_win_obj_intercept_stack_above, sd);
   evas_object_intercept_stack_below_callback_add
     (obj, _elm_win_obj_intercept_stack_below, sd);
   evas_object_intercept_layer_set_callback_add
     (obj, _elm_win_obj_intercept_layer_set, sd);
   evas_object_intercept_show_callback_add
     (obj, _elm_win_obj_intercept_show, sd);

   TRAP(sd, name_class_set, name, _elm_appname);
   ecore_evas_callback_delete_request_set(sd->ee, _elm_win_delete_request);
   ecore_evas_callback_resize_set(sd->ee, _elm_win_resize);
   ecore_evas_callback_mouse_in_set(sd->ee, _elm_win_mouse_in);
   ecore_evas_callback_focus_in_set(sd->ee, _elm_win_focus_in);
   ecore_evas_callback_focus_out_set(sd->ee, _elm_win_focus_out);
   ecore_evas_callback_move_set(sd->ee, _elm_win_move);
   ecore_evas_callback_state_change_set(sd->ee, _elm_win_state_change);

   evas_image_cache_set(sd->evas, (_elm_config->image_cache * 1024));
   evas_font_cache_set(sd->evas, (_elm_config->font_cache * 1024));

   EINA_LIST_FOREACH(_elm_config->font_dirs, l, fontpath)
     evas_font_path_append(sd->evas, fontpath);

   if (!_elm_config->font_hinting)
     evas_font_hinting_set(sd->evas, EVAS_FONT_HINTING_NONE);
   else if (_elm_config->font_hinting == 1)
     evas_font_hinting_set(sd->evas, EVAS_FONT_HINTING_AUTO);
   else if (_elm_config->font_hinting == 2)
     evas_font_hinting_set(sd->evas, EVAS_FONT_HINTING_BYTECODE);

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif

   _elm_win_list = eina_list_append(_elm_win_list, obj);
   _elm_win_count++;

   if (((fallback) && (!strcmp(fallback, "Software FB"))) ||
       ((!fallback) && (ENGINE_COMPARE(ELM_SOFTWARE_FB))))
     {
        TRAP(sd, fullscreen_set, 1);
     }
   else if ((type != ELM_WIN_INLINED_IMAGE) &&
            (ENGINE_COMPARE(ELM_WAYLAND_SHM) ||
             ENGINE_COMPARE(ELM_WAYLAND_EGL)))
     _elm_win_frame_add(sd, "default");

   if (_elm_config->focus_highlight_enable)
     elm_win_focus_highlight_enabled_set(obj, EINA_TRUE);

#ifdef ELM_DEBUG
   Evas_Modifier_Mask mask = evas_key_modifier_mask_get(sd->evas, "Control");
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_KEY_DOWN, _debug_key_down, sd);

   if (evas_object_key_grab(obj, "F12", mask, 0, EINA_TRUE))
     INF("Ctrl+F12 key combination exclusive for dot tree generation\n");
   else
     ERR("failed to grab F12 key to elm widgets (dot) tree generation");
#endif

   if ((_elm_config->softcursor_mode == ELM_SOFTCURSOR_MODE_ON) ||
       ((_elm_config->softcursor_mode == ELM_SOFTCURSOR_MODE_AUTO) &&
           (((fallback) && (!strcmp(fallback, "Software FB"))) ||
               ((!fallback) && (ENGINE_COMPARE(ELM_SOFTWARE_FB))))))
     {
        Evas_Object *o;
        Evas_Coord mw = 1, mh = 1, hx = 0, hy = 0;

        sd->pointer.obj = o = edje_object_add(ecore_evas_get(tmp_sd.ee));
        _elm_theme_object_set(obj, o, "pointer", "base", "default");
        edje_object_size_min_calc(o, &mw, &mh);
        evas_object_resize(o, mw, mh);
        edje_object_part_geometry_get(o, "elm.swallow.hotspot",
                                      &hx, &hy, NULL, NULL);
        sd->pointer.hot_x = hx;
        sd->pointer.hot_y = hy;
        evas_object_show(o);
        ecore_evas_object_cursor_set(tmp_sd.ee, o, EVAS_LAYER_MAX, hx, hy);
     }
   else if (_elm_config->softcursor_mode == ELM_SOFTCURSOR_MODE_OFF)
     {
        // do nothing
     }
}

static void
_constructor(Eo *obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_error_set(obj);
   ERR("only custom constructor can be used with '%s' class", MY_CLASS_NAME);
}

EAPI Evas_Object *
elm_win_util_standard_add(const char *name,
                          const char *title)
{
   Evas_Object *win, *bg;

   win = elm_win_add(NULL, name, ELM_WIN_BASIC);
   if (!win) return NULL;

   elm_win_title_set(win, title);
   bg = elm_bg_add(win);
   if (!bg)
     {
        evas_object_del(win);
        return NULL;
     }
   evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   return win;
}

EAPI void
elm_win_resize_object_add(Evas_Object *obj,
                          Evas_Object *subobj)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_resize_object_add(subobj));
}

static void
_resize_object_add(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *subobj = va_arg(*list, Evas_Object *);
   Evas_Coord w, h;

   Elm_Win_Smart_Data *sd = _pd;

   if (eina_list_data_find(sd->resize_objs, subobj)) return;

   if (!elm_widget_sub_object_add(obj, subobj))
     ERR("could not add %p as sub object of %p", subobj, obj);

   sd->resize_objs = eina_list_append(sd->resize_objs, subobj);

   evas_object_event_callback_add
     (subobj, EVAS_CALLBACK_DEL, _elm_win_on_resize_obj_del, obj);
   evas_object_event_callback_add
     (subobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _elm_win_on_resize_obj_changed_size_hints, obj);

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   evas_object_move(subobj, sd->fx, sd->fy);
   evas_object_resize(subobj, w - sd->fw, h - sd->fh);

   _elm_win_resize_objects_eval(obj);
}

EAPI void
elm_win_resize_object_del(Evas_Object *obj,
                          Evas_Object *subobj)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_resize_object_del(subobj));
}

static void
_resize_object_del(Eo *obj, void *_pd, va_list *list)
{
   Evas_Object *subobj = va_arg(*list, Evas_Object *);
   Elm_Win_Smart_Data *sd = _pd;

   if (!elm_widget_sub_object_del(obj, subobj))
     ERR("could not remove sub object %p from %p", subobj, obj);

   sd->resize_objs = eina_list_remove(sd->resize_objs, subobj);

   evas_object_event_callback_del_full
     (subobj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
     _elm_win_on_resize_obj_changed_size_hints, obj);
   evas_object_event_callback_del_full
     (subobj, EVAS_CALLBACK_DEL, _elm_win_on_resize_obj_del, obj);

   _elm_win_resize_objects_eval(obj);
}

EAPI void
elm_win_title_set(Evas_Object *obj,
                  const char *title)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_title_set(title));
}

static void
_title_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *title = va_arg(*list, const char *);
   Elm_Win_Smart_Data *sd = _pd;

   if (!title) return;
   eina_stringshare_replace(&(sd->title), title);
   TRAP(sd, title_set, sd->title);
   if (sd->frame_obj)
     edje_object_part_text_escaped_set
       (sd->frame_obj, "elm.text.title", sd->title);
}

EAPI const char *
elm_win_title_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_win_title_get(&ret));
   return ret;
}

static void
_title_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->title;
}

EAPI void
elm_win_icon_name_set(Evas_Object *obj,
                      const char *icon_name)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_icon_name_set(icon_name));
}

static void
_icon_name_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *icon_name = va_arg(*list, const char *);
   Elm_Win_Smart_Data *sd = _pd;

   if (!icon_name) return;
   eina_stringshare_replace(&(sd->icon_name), icon_name);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI const char *
elm_win_icon_name_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_win_icon_name_get(&ret));
   return ret;
}

static void
_icon_name_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->icon_name;
}

EAPI void
elm_win_role_set(Evas_Object *obj, const char *role)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_role_set(role));
}

static void
_role_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *role = va_arg(*list, const char *);
   Elm_Win_Smart_Data *sd = _pd;

   if (!role) return;
   eina_stringshare_replace(&(sd->role), role);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI const char *
elm_win_role_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_win_role_get(&ret));
   return ret;
}

static void
_role_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->role;
}

EAPI void
elm_win_icon_object_set(Evas_Object *obj,
                        Evas_Object *icon)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_icon_object_set(icon));
}

static void
_icon_object_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object *icon = va_arg(*list, Evas_Object *);
   Elm_Win_Smart_Data *sd = _pd;

   if (sd->icon)
     evas_object_event_callback_del_full
       (sd->icon, EVAS_CALLBACK_DEL, _elm_win_on_icon_del, sd);
   sd->icon = icon;
   if (sd->icon)
     evas_object_event_callback_add
       (sd->icon, EVAS_CALLBACK_DEL, _elm_win_on_icon_del, sd);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI const Evas_Object *
elm_win_icon_object_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) NULL;
   const Evas_Object *ret = NULL;
   eo_do((Eo *) obj, elm_obj_win_icon_object_get(&ret));
   return ret;
}

static void
_icon_object_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const Evas_Object **ret = va_arg(*list, const Evas_Object **);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->icon;
}

EAPI void
elm_win_autodel_set(Evas_Object *obj,
                    Eina_Bool autodel)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_autodel_set(autodel));
}

static void
_autodel_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool autodel = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;
   sd->autodel = autodel;
}

EAPI Eina_Bool
elm_win_autodel_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_win_autodel_get(&ret));
   return ret;
}

static void
_autodel_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->autodel;
}

EAPI void
elm_win_activate(Evas_Object *obj)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_activate());
}

static void
_activate(Eo *obj EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Win_Smart_Data *sd = _pd;
   TRAP(sd, activate);
}

EAPI void
elm_win_lower(Evas_Object *obj)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_lower());
}

static void
_lower(Eo *obj EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Win_Smart_Data *sd = _pd;
   TRAP(sd, lower);
}

EAPI void
elm_win_raise(Evas_Object *obj)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_raise());
}

static void
_raise(Eo *obj EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   Elm_Win_Smart_Data *sd = _pd;
   TRAP(sd, raise);
}

EAPI void
elm_win_center(Evas_Object *obj,
               Eina_Bool h,
               Eina_Bool v)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_center(h, v));
}

static void
_center(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool h = va_arg(*list, int);
   Eina_Bool v = va_arg(*list, int);
   int win_w, win_h, screen_w, screen_h, nx, ny;

   Elm_Win_Smart_Data *sd = _pd;

   if ((trap) && (trap->center) && (!trap->center(sd->trap_data, obj)))
     return;

   ecore_evas_screen_geometry_get(sd->ee, NULL, NULL, &screen_w, &screen_h);
   if ((!screen_w) || (!screen_h)) return;

   evas_object_geometry_get(obj, NULL, NULL, &win_w, &win_h);
   if ((!win_w) || (!win_h)) return;

   if (h) nx = win_w >= screen_w ? 0 : (screen_w / 2) - (win_w / 2);
   else nx = sd->screen.x;
   if (v) ny = win_h >= screen_h ? 0 : (screen_h / 2) - (win_h / 2);
   else ny = sd->screen.y;
   if (nx < 0) nx = 0;
   if (ny < 0) ny = 0;

   evas_object_move(obj, nx, ny);
}

EAPI void
elm_win_borderless_set(Evas_Object *obj,
                       Eina_Bool borderless)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_borderless_set(borderless));
}

static void
_borderless_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool borderless = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;

   TRAP(sd, borderless_set, borderless);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI Eina_Bool
elm_win_borderless_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_win_borderless_get(&ret));
   return ret;
}

static void
_borderless_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = ecore_evas_borderless_get(sd->ee);
}

EAPI void
elm_win_shaped_set(Evas_Object *obj,
                   Eina_Bool shaped)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_shaped_set(shaped));
}

static void
_shaped_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool shaped = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;

   TRAP(sd, shaped_set, shaped);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI Eina_Bool
elm_win_shaped_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_win_shaped_get(&ret));
   return ret;
}

static void
_shaped_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = ecore_evas_shaped_get(sd->ee);
}

EAPI void
elm_win_alpha_set(Evas_Object *obj,
                  Eina_Bool enabled)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_alpha_set(enabled));
}

static void
_alpha_set(Eo *obj, void *_pd, va_list *list)
{
   Eina_Bool enabled = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;

   if (sd->img_obj)
     {
        evas_object_image_alpha_set(sd->img_obj, enabled);
        ecore_evas_alpha_set(sd->ee, enabled);
     }
   else
     {
#ifdef HAVE_ELEMENTARY_X
        if (sd->x.xwin)
          {
             if (enabled)
               {
                  if (!ecore_x_screen_is_composited(0))
                    elm_win_shaped_set(obj, enabled);
                  else
                    TRAP(sd, alpha_set, enabled);
               }
             else
               TRAP(sd, alpha_set, enabled);
             _elm_win_xwin_update(sd);
          }
        else
#endif
          TRAP(sd, alpha_set, enabled);
     }
}

EAPI Eina_Bool
elm_win_alpha_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_win_alpha_get(&ret));
   return ret;
}

static void
_alpha_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Win_Smart_Data *sd = _pd;

   if (sd->img_obj)
     {
        *ret = evas_object_image_alpha_get(sd->img_obj);
     }
   else
     {
        *ret = ecore_evas_alpha_get(sd->ee);
     }
}

EAPI void
elm_win_override_set(Evas_Object *obj,
                     Eina_Bool override)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_override_set(override));
}

static void
_override_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool override = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;

   TRAP(sd, override_set, override);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI Eina_Bool
elm_win_override_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_win_override_get(&ret));
   return ret;
}

static void
_override_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = ecore_evas_override_get(sd->ee);
}

EAPI void
elm_win_fullscreen_set(Evas_Object *obj,
                       Eina_Bool fullscreen)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_fullscreen_set(fullscreen));
}

static void
_fullscreen_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool fullscreen = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;
   // YYY: handle if sd->img_obj
   if (ENGINE_COMPARE(ELM_SOFTWARE_FB) ||
       ENGINE_COMPARE(ELM_SOFTWARE_16_WINCE))
     {
        // these engines... can ONLY be fullscreen
        return;
     }
   else
     {
        sd->fullscreen = fullscreen;

        if (fullscreen)
          {
             if (EE_ENGINE_COMPARE(sd->ee, ELM_WAYLAND_SHM) ||
                 EE_ENGINE_COMPARE(sd->ee, ELM_WAYLAND_EGL))
               _elm_win_frame_del(sd);
          }
        else
          {
             if (EE_ENGINE_COMPARE(sd->ee, ELM_WAYLAND_SHM) ||
                 EE_ENGINE_COMPARE(sd->ee, ELM_WAYLAND_EGL))
               _elm_win_frame_add(sd, "default");

             evas_object_show(sd->frame_obj);
          }

        TRAP(sd, fullscreen_set, fullscreen);
#ifdef HAVE_ELEMENTARY_X
        _elm_win_xwin_update(sd);
#endif
     }
}

EAPI Eina_Bool
elm_win_fullscreen_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_win_fullscreen_get(&ret));
   return ret;
}

static void
_fullscreen_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Win_Smart_Data *sd = _pd;

   if (EE_ENGINE_COMPARE(sd->ee, ELM_SOFTWARE_FB) ||
       EE_ENGINE_COMPARE(sd->ee, ELM_SOFTWARE_16_WINCE))
     {
        // these engines... can ONLY be fullscreen
        *ret = EINA_TRUE;
     }
   else
     {
        *ret = sd->fullscreen;
     }
}

EAPI void
elm_win_maximized_set(Evas_Object *obj,
                      Eina_Bool maximized)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_maximized_set(maximized));
}

static void
_maximized_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool maximized = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;

   sd->maximized = maximized;
   // YYY: handle if sd->img_obj
   TRAP(sd, maximized_set, maximized);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI Eina_Bool
elm_win_maximized_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_win_maximized_get(&ret));
   return ret;
}

static void
_maximized_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->maximized;
}

EAPI void
elm_win_iconified_set(Evas_Object *obj,
                      Eina_Bool iconified)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_iconified_set(iconified));
}

static void
_iconified_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool iconified = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;

   sd->iconified = iconified;
   TRAP(sd, iconified_set, iconified);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI Eina_Bool
elm_win_iconified_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_win_iconified_get(&ret));
   return ret;
}

static void
_iconified_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->iconified;
}

EAPI void
elm_win_withdrawn_set(Evas_Object *obj,
                      Eina_Bool withdrawn)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_withdrawn_set(withdrawn));
}

static void
_withdrawn_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool withdrawn = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;

   sd->withdrawn = withdrawn;
   TRAP(sd, withdrawn_set, withdrawn);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI Eina_Bool
elm_win_withdrawn_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_win_withdrawn_get(&ret));
   return ret;
}

static void
_withdrawn_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->withdrawn;
}

EAPI void
elm_win_available_profiles_set(Evas_Object  *obj,
                               const char  **profiles,
                               unsigned int  count)
{
   ELM_WIN_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_win_available_profiles_set(profiles, count));
}

static void
_available_profiles_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **profiles = va_arg(*list, const char **);
   unsigned int count = va_arg(*list, unsigned int);
   Elm_Win_Smart_Data *sd = _pd;
   Eina_Bool found = EINA_FALSE;

   _elm_win_available_profiles_del(sd);
   if ((profiles) && (count >= 1))
     {
        sd->profile.available_list = calloc(count, sizeof(char *));
        if (sd->profile.available_list)
          {
             if (!sd->profile.name) found = EINA_TRUE;

             unsigned int i;
             for (i = 0; i < count; i++)
               {
                  sd->profile.available_list[i] = eina_stringshare_add(profiles[i]);

                  /* check to see if a given array has a current profile of elm_win */
                  if ((sd->profile.name) &&
                      (!strcmp(sd->profile.name, profiles[i])))
                    {
                       found = EINA_TRUE;
                    }
               }
             sd->profile.count = count;
          }
     }

   if (ecore_evas_window_profile_supported_get(sd->ee))
     {
        ecore_evas_window_available_profiles_set(sd->ee,
                                                 sd->profile.available_list,
                                                 sd->profile.count);

        /* current profile of elm_win is wrong, change profile */
        if ((sd->profile.available_list) && (!found))
          {
             eina_stringshare_replace(&(sd->profile.name),
                                      sd->profile.available_list[0]);
             ecore_evas_window_profile_set(sd->ee, sd->profile.name);
          }

     }
   else
     {
        if (sd->profile.available_list)
          _elm_win_profile_update(sd);
     }
}

EAPI Eina_Bool
elm_win_available_profiles_get(Evas_Object   *obj,
                               char        ***profiles,
                               unsigned int  *count)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_win_available_profiles_get(&ret, profiles, count));
   return ret;
}

static void
_available_profiles_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   char ***profiles = va_arg(*list, char ***);
   unsigned int *count = va_arg(*list, unsigned int *);
   Elm_Win_Smart_Data *sd = _pd;
   Eina_Bool res;

   if (ecore_evas_window_profile_supported_get(sd->ee))
     {
        res = ecore_evas_window_available_profiles_get(sd->ee,
                                                       profiles,
                                                       count);
     }
   else
     {
        if (profiles) *profiles = (char **)sd->profile.available_list;
        if (count) *count = sd->profile.count;
        res = EINA_TRUE;
     }
   *ret = res;
}

EAPI void
elm_win_profile_set(Evas_Object *obj,
                    const char *profile)
{
   ELM_WIN_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_win_profile_set(profile));
}

static void
_profile_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *profile = va_arg(*list, const char *);
   Elm_Win_Smart_Data *sd = _pd;

   /* check to see if a given profile is present in an available profiles */
   if ((profile) && (sd->profile.available_list))
     {
        Eina_Bool found = EINA_FALSE;
        unsigned int i;
        for (i = 0; i < sd->profile.count; i++)
          {
             if (!strcmp(profile,
                         sd->profile.available_list[i]))
               {
                  found = EINA_TRUE;
                  break;
               }
          }
        if (!found) return;
     }

   if (ecore_evas_window_profile_supported_get(sd->ee))
     {
        if (!profile) _elm_win_profile_del(sd);
        ecore_evas_window_profile_set(sd->ee, profile);
     }
   else
     {
        if (_elm_win_profile_set(sd, profile))
          _elm_win_profile_update(sd);
     }
}

EAPI const char *
elm_win_profile_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_win_profile_get(&ret));
   return ret;
}

static void
_profile_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->profile.name;
}

EAPI void
elm_win_urgent_set(Evas_Object *obj,
                   Eina_Bool urgent)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_urgent_set(urgent));
}

static void
_urgent_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool urgent = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;

   sd->urgent = urgent;
   TRAP(sd, urgent_set, urgent);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI Eina_Bool
elm_win_urgent_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_win_urgent_get(&ret));
   return ret;
}

static void
_urgent_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->urgent;
}

EAPI void
elm_win_demand_attention_set(Evas_Object *obj,
                             Eina_Bool demand_attention)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_demand_attention_set(demand_attention));
}

static void
_demand_attention_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool demand_attention = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;

   sd->demand_attention = demand_attention;
   TRAP(sd, demand_attention_set, demand_attention);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI Eina_Bool
elm_win_demand_attention_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_win_demand_attention_get(&ret));
   return ret;
}

static void
_demand_attention_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->demand_attention;
}

EAPI void
elm_win_modal_set(Evas_Object *obj,
                  Eina_Bool modal)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_modal_set(modal));
}

static void
_modal_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool modal = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;

   sd->modal = modal;
   TRAP(sd, modal_set, modal);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI Eina_Bool
elm_win_modal_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_win_modal_get(&ret));
   return ret;
}

static void
_modal_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->modal;
}

EAPI void
elm_win_aspect_set(Evas_Object *obj,
                   double aspect)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_aspect_set(aspect));
}

static void
_aspect_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double aspect = va_arg(*list, double);
   Elm_Win_Smart_Data *sd = _pd;

   sd->aspect = aspect;
   TRAP(sd, aspect_set, aspect);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI double
elm_win_aspect_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) 0.0;
   double ret = 0.0;
   eo_do((Eo *) obj, elm_obj_win_aspect_get(&ret));
   return ret;
}

static void
_aspect_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   double *ret = va_arg(*list, double *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->aspect;
}

EAPI void
elm_win_size_base_set(Evas_Object *obj, int w, int h)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_size_base_set(w, h));
}

static void
_size_base_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int w = va_arg(*list, int);
   int h = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;
   sd->size_base_w = w;
   sd->size_base_h = h;
   TRAP(sd, size_base_set, w, h);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI void
elm_win_size_base_get(Evas_Object *obj, int *w, int *h)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_size_base_get(w, h));
}

static void
_size_base_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *w = va_arg(*list, int *);
   int *h = va_arg(*list, int *);

   Elm_Win_Smart_Data *sd = _pd;
   if (w) *w = sd->size_base_w;
   if (w) *h = sd->size_base_h;
}

EAPI void
elm_win_size_step_set(Evas_Object *obj, int w, int h)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_size_step_set(w, h));
}

static void
_size_step_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int w = va_arg(*list, int);
   int h = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;
   sd->size_step_w = w;
   sd->size_step_h = h;
   TRAP(sd, size_step_set, w, h);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI void
elm_win_size_step_get(Evas_Object *obj, int *w, int *h)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_size_step_get(w, h));
}

static void
_size_step_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *w = va_arg(*list, int *);
   int *h = va_arg(*list, int *);
   Elm_Win_Smart_Data *sd = _pd;
   if (w) *w = sd->size_step_w;
   if (w) *h = sd->size_step_h;
}

EAPI void
elm_win_layer_set(Evas_Object *obj,
                  int layer)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_layer_set(layer));
}

static void
_layer_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int layer = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;

   TRAP(sd, layer_set, layer);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI int
elm_win_layer_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) - 1;
   int ret = - 1;
   eo_do((Eo *) obj, elm_obj_win_layer_get(&ret));
   return ret;
}

static void
_layer_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = ecore_evas_layer_get(sd->ee);
}

EAPI void
elm_win_norender_push(Evas_Object *obj)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   sd->norender++;
   if (sd->norender == 1) ecore_evas_manual_render_set(sd->ee, EINA_TRUE);
}

EAPI void
elm_win_norender_pop(Evas_Object *obj)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   if (sd->norender <= 0) return;
   sd->norender--;
   if (sd->norender == 0) ecore_evas_manual_render_set(sd->ee, EINA_FALSE);
}

EAPI int
elm_win_norender_get(Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) - 1;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, -1);
   return sd->norender;
}

EAPI void
elm_win_render(Evas_Object *obj)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);
   ecore_evas_manual_render(sd->ee);
}


static int
_win_rotation_degree_check(int rotation)
{
   if ((rotation > 360) || (rotation < 0))
     {
        WRN("Rotation degree should be 0 ~ 360 (passed degree: %d)", rotation);
        rotation %= 360;
        if (rotation < 0) rotation += 360;
     }
   return rotation;
}

EAPI void
elm_win_rotation_set(Evas_Object *obj,
                     int rotation)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_rotation_set(rotation));
}

static void
_rotation_set(Eo *obj, void *_pd, va_list *list)
{
   int rotation = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;

   rotation = _win_rotation_degree_check(rotation);
   if (sd->rot == rotation) return;
   sd->rot = rotation;
   TRAP(sd, rotation_set, rotation);
   evas_object_size_hint_min_set(obj, -1, -1);
   evas_object_size_hint_max_set(obj, -1, -1);
   _elm_win_resize_objects_eval(obj);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
   evas_object_smart_callback_call(obj, SIG_ROTATION_CHANGED, NULL);
}

EAPI void
elm_win_rotation_with_resize_set(Evas_Object *obj,
                                 int rotation)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_rotation_with_resize_set(rotation));
}

static void
_rotation_with_resize_set(Eo *obj, void *_pd, va_list *list)
{
   int rotation = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;

   rotation = _win_rotation_degree_check(rotation);
   if (sd->rot == rotation) return;
   sd->rot = rotation;
   TRAP(sd, rotation_with_resize_set, rotation);
   evas_object_size_hint_min_set(obj, -1, -1);
   evas_object_size_hint_max_set(obj, -1, -1);
   _elm_win_resize_objects_eval(obj);

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
   evas_object_smart_callback_call(obj, SIG_ROTATION_CHANGED, NULL);
}

EAPI int
elm_win_rotation_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) - 1;
   int ret = - 1;
   eo_do((Eo *) obj, elm_obj_win_rotation_get(&ret));
   return ret;
}

static void
_rotation_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->rot;
}

EAPI void
elm_win_sticky_set(Evas_Object *obj,
                   Eina_Bool sticky)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_sticky_set(sticky));
}

static void
_sticky_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool sticky = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;

   sd->sticky = sticky;
   TRAP(sd, sticky_set, sticky);
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwin_update(sd);
#endif
}

EAPI Eina_Bool
elm_win_sticky_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_win_sticky_get(&ret));
   return ret;
}

static void
_sticky_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->sticky;
}

EAPI void
elm_win_keyboard_mode_set(Evas_Object *obj,
                          Elm_Win_Keyboard_Mode mode)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_keyboard_mode_set(mode));
}

static void
_keyboard_mode_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Win_Keyboard_Mode mode = va_arg(*list, Elm_Win_Keyboard_Mode);
   Elm_Win_Smart_Data *sd = _pd;

   if (mode == sd->kbdmode) return;
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
#endif
   sd->kbdmode = mode;
#ifdef HAVE_ELEMENTARY_X
   if (sd->x.xwin)
     ecore_x_e_virtual_keyboard_state_set
       (sd->x.xwin, (Ecore_X_Virtual_Keyboard_State)sd->kbdmode);
#endif
}

EAPI Elm_Win_Keyboard_Mode
elm_win_keyboard_mode_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) ELM_WIN_KEYBOARD_UNKNOWN;
   Elm_Win_Keyboard_Mode ret = ELM_WIN_KEYBOARD_UNKNOWN;
   eo_do((Eo *) obj, elm_obj_win_keyboard_mode_get(&ret));
   return ret;
}

static void
_keyboard_mode_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Win_Keyboard_Mode *ret = va_arg(*list, Elm_Win_Keyboard_Mode *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->kbdmode;
}

EAPI void
elm_win_keyboard_win_set(Evas_Object *obj,
                         Eina_Bool is_keyboard)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_keyboard_win_set(is_keyboard));
}

static void
_keyboard_win_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool is_keyboard = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     ecore_x_e_virtual_keyboard_set(sd->x.xwin, is_keyboard);
#else
   (void)is_keyboard;
#endif
}

EAPI Eina_Bool
elm_win_keyboard_win_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_win_keyboard_win_get(&ret));
   return ret;
}

static void
_keyboard_win_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Win_Smart_Data *sd = _pd;

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     {
        *ret = ecore_x_e_virtual_keyboard_get(sd->x.xwin);
        return;
     }
#endif
   *ret = EINA_FALSE;
}

EAPI void
elm_win_indicator_mode_set(Evas_Object *obj,
      Elm_Win_Indicator_Mode mode)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_indicator_mode_set(mode));
}

static void
_indicator_mode_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Win_Indicator_Mode mode = va_arg(*list, Elm_Win_Indicator_Mode);
   Elm_Win_Smart_Data *sd = _pd;

   if (mode == sd->indmode) return;
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
#endif
   sd->indmode = mode;
#ifdef HAVE_ELEMENTARY_X
   if (sd->x.xwin)
     {
        if (sd->indmode == ELM_WIN_INDICATOR_SHOW)
          ecore_x_e_illume_indicator_state_set
            (sd->x.xwin, ECORE_X_ILLUME_INDICATOR_STATE_ON);
        else if (sd->indmode == ELM_WIN_INDICATOR_HIDE)
          ecore_x_e_illume_indicator_state_set
            (sd->x.xwin, ECORE_X_ILLUME_INDICATOR_STATE_OFF);
     }
#endif
   evas_object_smart_callback_call(obj, SIG_INDICATOR_PROP_CHANGED, NULL);
}

EAPI Elm_Win_Indicator_Mode
elm_win_indicator_mode_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) ELM_WIN_INDICATOR_UNKNOWN;
   Elm_Win_Indicator_Mode ret = ELM_WIN_INDICATOR_UNKNOWN;
   eo_do((Eo *) obj, elm_obj_win_indicator_mode_get(&ret));
   return ret;
}

static void
_indicator_mode_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Win_Indicator_Mode *ret = va_arg(*list, Elm_Win_Indicator_Mode *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->indmode;
}

EAPI void
elm_win_indicator_opacity_set(Evas_Object *obj,
                              Elm_Win_Indicator_Opacity_Mode mode)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_indicator_opacity_set(mode));
}

static void
_indicator_opacity_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Win_Indicator_Opacity_Mode mode = va_arg(*list, Elm_Win_Indicator_Opacity_Mode);
   Elm_Win_Smart_Data *sd = _pd;

   if (mode == sd->ind_o_mode) return;
   sd->ind_o_mode = mode;
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     {
        if (sd->ind_o_mode == ELM_WIN_INDICATOR_OPAQUE)
          ecore_x_e_illume_indicator_opacity_set
            (sd->x.xwin, ECORE_X_ILLUME_INDICATOR_OPAQUE);
        else if (sd->ind_o_mode == ELM_WIN_INDICATOR_TRANSLUCENT)
          ecore_x_e_illume_indicator_opacity_set
            (sd->x.xwin, ECORE_X_ILLUME_INDICATOR_TRANSLUCENT);
        else if (sd->ind_o_mode == ELM_WIN_INDICATOR_TRANSPARENT)
          ecore_x_e_illume_indicator_opacity_set
            (sd->x.xwin, ECORE_X_ILLUME_INDICATOR_TRANSPARENT);
     }
#endif
   evas_object_smart_callback_call(obj, SIG_INDICATOR_PROP_CHANGED, NULL);
}

EAPI Elm_Win_Indicator_Opacity_Mode
elm_win_indicator_opacity_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) ELM_WIN_INDICATOR_OPACITY_UNKNOWN;
   Elm_Win_Indicator_Opacity_Mode ret = ELM_WIN_INDICATOR_OPACITY_UNKNOWN;
   eo_do((Eo *) obj, elm_obj_win_indicator_opacity_get(&ret));
   return ret;
}

static void
_indicator_opacity_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Win_Indicator_Opacity_Mode *ret = va_arg(*list, Elm_Win_Indicator_Opacity_Mode *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->ind_o_mode;
}

EAPI void
elm_win_screen_position_get(const Evas_Object *obj,
                            int *x,
                            int *y)
{
   ELM_WIN_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_win_screen_position_get(x, y));
}

static void
_screen_position_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *x = va_arg(*list, int *);
   int *y = va_arg(*list, int *);
   Elm_Win_Smart_Data *sd = _pd;

   if (x) *x = sd->screen.x;
   if (y) *y = sd->screen.y;
}

EAPI Eina_Bool
elm_win_focus_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_win_focus_get(&ret));
   return ret;
}

static void
_focus_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = ecore_evas_focus_get(sd->ee);
}

EAPI void
elm_win_screen_constrain_set(Evas_Object *obj,
                             Eina_Bool constrain)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_screen_constrain_set(constrain));
}

static void
_screen_constrain_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool constrain = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;
   sd->constrain = !!constrain;
}

EAPI Eina_Bool
elm_win_screen_constrain_get(Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_win_screen_constrain_get(&ret));
   return ret;
}

static void
_screen_constrain_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->constrain;
}

EAPI void
elm_win_screen_size_get(const Evas_Object *obj,
                        int *x,
                        int *y,
                        int *w,
                        int *h)
{
   ELM_WIN_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_win_screen_size_get(x, y, w, h));
}

static void
_screen_size_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *x = va_arg(*list, int *);
   int *y = va_arg(*list, int *);
   int *w = va_arg(*list, int *);
   int *h = va_arg(*list, int *);
   Elm_Win_Smart_Data *sd = _pd;

   ecore_evas_screen_geometry_get(sd->ee, x, y, w, h);
}

EAPI void
elm_win_screen_dpi_get(const Evas_Object *obj,
                       int *xdpi,
                       int *ydpi)
{
   ELM_WIN_CHECK(obj);
   eo_do((Eo *) obj, elm_obj_win_screen_dpi_get(xdpi, ydpi));
}

static void
_screen_dpi_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *xdpi = va_arg(*list, int *);
   int *ydpi = va_arg(*list, int *);
   Elm_Win_Smart_Data *sd = _pd;

   ecore_evas_screen_dpi_get(sd->ee, xdpi, ydpi);
}

EAPI void
elm_win_conformant_set(Evas_Object *obj,
                       Eina_Bool conformant)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_conformant_set(conformant));
}

static void
_conformant_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool conformant = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     ecore_x_e_illume_conformant_set(sd->x.xwin, conformant);
#else
   (void)conformant;
#endif
}

EAPI Eina_Bool
elm_win_conformant_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_win_conformant_get(&ret));
   return ret;
}

static void
_conformant_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = EINA_FALSE;

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     *ret = ecore_x_e_illume_conformant_get(sd->x.xwin);
#endif
}

EAPI void
elm_win_quickpanel_set(Evas_Object *obj,
                       Eina_Bool quickpanel)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_quickpanel_set(quickpanel));
}

static void
_quickpanel_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool quickpanel = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     {
        ecore_x_e_illume_quickpanel_set(sd->x.xwin, quickpanel);
        if (quickpanel)
          {
             Ecore_X_Window_State states[2];

             states[0] = ECORE_X_WINDOW_STATE_SKIP_TASKBAR;
             states[1] = ECORE_X_WINDOW_STATE_SKIP_PAGER;
             ecore_x_netwm_window_state_set(sd->x.xwin, states, 2);
             ecore_x_icccm_hints_set(sd->x.xwin, 0, 0, 0, 0, 0, 0, 0);
          }
     }
#else
   (void)quickpanel;
#endif
}

EAPI Eina_Bool
elm_win_quickpanel_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_win_quickpanel_get(&ret));
   return ret;
}

static void
_quickpanel_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = EINA_FALSE;

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     *ret = ecore_x_e_illume_quickpanel_get(sd->x.xwin);
#endif
}

EAPI void
elm_win_quickpanel_priority_major_set(Evas_Object *obj,
                                      int priority)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_quickpanel_priority_major_set(priority));
}

static void
_quickpanel_priority_major_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int priority = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     ecore_x_e_illume_quickpanel_priority_major_set(sd->x.xwin, priority);
#else
   (void)priority;
#endif
}

EAPI int
elm_win_quickpanel_priority_major_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) - 1;
   int ret = - 1;
   eo_do((Eo *) obj, elm_obj_win_quickpanel_priority_major_get(&ret));
   return ret;
}

static void
_quickpanel_priority_major_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = -1;

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     *ret = ecore_x_e_illume_quickpanel_priority_major_get(sd->x.xwin);
#endif
}

EAPI void
elm_win_quickpanel_priority_minor_set(Evas_Object *obj,
                                      int priority)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_quickpanel_priority_minor_set(priority));
}

static void
_quickpanel_priority_minor_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int priority = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     ecore_x_e_illume_quickpanel_priority_minor_set(sd->x.xwin, priority);
#else
   (void)priority;
#endif
}

EAPI int
elm_win_quickpanel_priority_minor_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) - 1;
   int ret = - 1;
   eo_do((Eo *) obj, elm_obj_win_quickpanel_priority_minor_get(&ret));
   return ret;
}

static void
_quickpanel_priority_minor_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = -1;

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     *ret = ecore_x_e_illume_quickpanel_priority_minor_get(sd->x.xwin);
#endif
}

EAPI void
elm_win_quickpanel_zone_set(Evas_Object *obj,
                            int zone)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_quickpanel_zone_set(zone));
}

static void
_quickpanel_zone_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int zone = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     ecore_x_e_illume_quickpanel_zone_set(sd->x.xwin, zone);
#else
   (void)zone;
#endif
}

EAPI int
elm_win_quickpanel_zone_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) 0;
   int ret = 0;
   eo_do((Eo *) obj, elm_obj_win_quickpanel_zone_get(&ret));
   return ret;
}

static void
_quickpanel_zone_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = 0;

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     *ret = ecore_x_e_illume_quickpanel_zone_get(sd->x.xwin);
#endif
}

EAPI void
elm_win_prop_focus_skip_set(Evas_Object *obj,
                            Eina_Bool skip)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_prop_focus_skip_set(skip));
}

static void
_prop_focus_skip_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool skip = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;

   sd->skip_focus = skip;
   TRAP(sd, focus_skip_set, skip);
}

EAPI void
elm_win_illume_command_send(Evas_Object *obj,
                            Elm_Illume_Command command,
                            void *params)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_illume_command_send(command, params));
}

static void
_illume_command_send(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Elm_Illume_Command command = va_arg(*list, Elm_Illume_Command);
   void *params = va_arg(*list, void *);
   (void) params;
   Elm_Win_Smart_Data *sd = _pd;

#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     {
        switch (command)
          {
           case ELM_ILLUME_COMMAND_FOCUS_BACK:
             ecore_x_e_illume_focus_back_send(sd->x.xwin);
             break;

           case ELM_ILLUME_COMMAND_FOCUS_FORWARD:
             ecore_x_e_illume_focus_forward_send(sd->x.xwin);
             break;

           case ELM_ILLUME_COMMAND_FOCUS_HOME:
             ecore_x_e_illume_focus_home_send(sd->x.xwin);
             break;

           case ELM_ILLUME_COMMAND_CLOSE:
             ecore_x_e_illume_close_send(sd->x.xwin);
             break;

           default:
             break;
          }
     }
#else
   (void)command;
#endif
}

EAPI Evas_Object *
elm_win_inlined_image_object_get(Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) NULL;
   Evas_Object *ret = NULL;
   eo_do(obj, elm_obj_win_inlined_image_object_get(&ret));
   return ret;
}

static void
_inlined_image_object_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->img_obj;
}

EAPI void
elm_win_focus_highlight_enabled_set(Evas_Object *obj,
                                    Eina_Bool enabled)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_focus_highlight_enabled_set(enabled));
}

static void
_focus_highlight_enabled_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool enabled = va_arg(*list, int);
   Elm_Win_Smart_Data *sd = _pd;

   enabled = !!enabled;
   if (sd->focus_highlight.enabled == enabled)
     return;

   sd->focus_highlight.enabled = enabled;

   if (sd->focus_highlight.enabled)
     _elm_win_focus_highlight_init(sd);
   else
     _elm_win_focus_highlight_shutdown(sd);
}

EAPI Eina_Bool
elm_win_focus_highlight_enabled_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *) obj, elm_obj_win_focus_highlight_enabled_get(&ret));
   return ret;
}

static void
_focus_highlight_enabled_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->focus_highlight.enabled;
}

EAPI void
elm_win_focus_highlight_style_set(Evas_Object *obj,
                                  const char *style)
{
   ELM_WIN_CHECK(obj);
   eo_do(obj, elm_obj_win_focus_highlight_style_set(style));
}

static void
_focus_highlight_style_set(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *style = va_arg(*list, const char *);
   Elm_Win_Smart_Data *sd = _pd;

   eina_stringshare_replace(&sd->focus_highlight.style, style);
   sd->focus_highlight.changed_theme = EINA_TRUE;
   _elm_win_focus_highlight_reconfigure_job_start(sd);
}

EAPI const char *
elm_win_focus_highlight_style_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) NULL;
   const char *ret = NULL;
   eo_do((Eo *) obj, elm_obj_win_focus_highlight_style_get(&ret));
   return ret;
}

static void
_focus_highlight_style_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char **ret = va_arg(*list, const char **);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = sd->focus_highlight.style;
}

EAPI Eina_Bool
elm_win_socket_listen(Evas_Object *obj,
                      const char *svcname,
                      int svcnum,
                      Eina_Bool svcsys)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   Eina_Bool ret = EINA_FALSE;
   eo_do(obj, elm_obj_win_socket_listen(svcname, svcnum, svcsys, &ret));
   return ret;
}

static void
_socket_listen(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   const char *svcname = va_arg(*list, const char *);
   int svcnum = va_arg(*list, int);
   Eina_Bool svcsys = va_arg(*list, int);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   Elm_Win_Smart_Data *sd = _pd;
   *ret = EINA_FALSE;

   if (!sd->ee) return;

   if (!ecore_evas_extn_socket_listen(sd->ee, svcname, svcnum, svcsys))
     return;

   *ret = EINA_TRUE;
}

/* windowing specific calls - shall we do this differently? */

EAPI Ecore_X_Window
elm_win_xwindow_get(const Evas_Object *obj)
{
   if (!obj) return 0;

   if (!evas_object_smart_type_check_ptr(obj, MY_CLASS_NAME))
     {
        Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
        return _elm_ee_xwin_get(ee);
     }

   ELM_WIN_CHECK(obj) 0;
   Ecore_X_Window ret = 0;
   eo_do((Eo *) obj, elm_obj_win_xwindow_get(&ret));
   return ret;
}

static void
_xwindow_get(Eo *obj EINA_UNUSED, void *_pd, va_list *list)
{
   Ecore_X_Window *ret = va_arg(*list, Ecore_X_Window *);
   Elm_Win_Smart_Data *sd = _pd;

#ifdef HAVE_ELEMENTARY_X
   if (sd->x.xwin)
     {
        *ret = sd->x.xwin;
        return;
     }
   if (sd->parent)
     {
        *ret = elm_win_xwindow_get(sd->parent);
        return;
     }
#endif
   *ret = 0;
}

EAPI Ecore_Wl_Window *
elm_win_wl_window_get(const Evas_Object *obj)
{
   if (!obj) return NULL;

   if ((!ENGINE_COMPARE(ELM_WAYLAND_SHM)) || 
       (!ENGINE_COMPARE(ELM_WAYLAND_EGL)))
     return NULL;

   if (!evas_object_smart_type_check_ptr(obj, MY_CLASS_NAME))
     {
        Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(obj));
        return ecore_evas_wayland_window_get(ee);
     }

   ELM_WIN_CHECK(obj) NULL;
   Ecore_Wl_Window *ret = NULL;
   eo_do((Eo *) obj, elm_obj_win_wl_window_get(&ret));
   return ret;
}

static void
_wl_window_get(Eo *obj EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Ecore_Wl_Window **ret = va_arg(*list, Ecore_Wl_Window **);
#if HAVE_ELEMENTARY_WAYLAND
   Elm_Win_Smart_Data *sd = _pd;
   if (sd->wl.win)
     {
        *ret = sd->wl.win;
        return;
     }
   if (sd->parent)
     {
        *ret = elm_win_wl_window_get(sd->parent);
        return;
     }
#endif
   *ret = NULL;
}

EAPI Eina_Bool
elm_win_trap_set(const Elm_Win_Trap *t)
{
   DBG("old %p, new %p", trap, t);

   if ((t) && (t->version != ELM_WIN_TRAP_VERSION))
     {
        CRITICAL("trying to set a trap version %lu while %lu was expected!",
                 t->version, ELM_WIN_TRAP_VERSION);
        return EINA_FALSE;
     }

   trap = t;
   return EINA_TRUE;
}

EAPI void
elm_win_floating_mode_set(Evas_Object *obj, Eina_Bool floating)
{
   ELM_WIN_CHECK(obj);
   ELM_WIN_DATA_GET_OR_RETURN(obj, sd);

   if (floating == sd->floating) return;
   sd->floating = floating;
#ifdef HAVE_ELEMENTARY_X
   _elm_win_xwindow_get(sd);
   if (sd->x.xwin)
     {
        if (sd->floating)
          ecore_x_e_illume_window_state_set
             (sd->x.xwin, ECORE_X_ILLUME_WINDOW_STATE_FLOATING);
        else
          ecore_x_e_illume_window_state_set
             (sd->x.xwin, ECORE_X_ILLUME_WINDOW_STATE_NORMAL);
     }
#endif
}

EAPI Eina_Bool
elm_win_floating_mode_get(const Evas_Object *obj)
{
   ELM_WIN_CHECK(obj) EINA_FALSE;
   ELM_WIN_DATA_GET_OR_RETURN_VAL(obj, sd, EINA_FALSE);

   return sd->floating;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _elm_win_smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _elm_win_smart_del),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_RESIZE), _elm_win_smart_resize),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MOVE), _elm_win_smart_move),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_SHOW), _elm_win_smart_show),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_HIDE), _elm_win_smart_hide),

        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_ON_FOCUS), _elm_win_smart_on_focus),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_EVENT), _elm_win_smart_event),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_NEXT_MANAGER_IS), _elm_win_smart_focus_next_manager_is),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_NEXT), _elm_win_smart_focus_next),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_DIRECTION_MANAGER_IS), _elm_win_smart_focus_direction_manager_is),
        EO_OP_FUNC(ELM_WIDGET_ID(ELM_WIDGET_SUB_ID_FOCUS_DIRECTION), _elm_win_smart_focus_direction),

        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_WIN_CONSTRUCTOR), _win_constructor),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_RESIZE_OBJECT_ADD), _resize_object_add),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_RESIZE_OBJECT_DEL), _resize_object_del),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_TITLE_SET), _title_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_TITLE_GET), _title_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ICON_NAME_SET), _icon_name_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ICON_NAME_GET), _icon_name_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ROLE_SET), _role_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ROLE_GET), _role_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ICON_OBJECT_SET), _icon_object_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ICON_OBJECT_GET), _icon_object_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_AUTODEL_SET), _autodel_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_AUTODEL_GET), _autodel_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ACTIVATE), _activate),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_LOWER), _lower),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_RAISE), _raise),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_CENTER), _center),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_BORDERLESS_SET), _borderless_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_BORDERLESS_GET), _borderless_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SHAPED_SET), _shaped_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SHAPED_GET), _shaped_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ALPHA_SET), _alpha_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ALPHA_GET), _alpha_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_OVERRIDE_SET), _override_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_OVERRIDE_GET), _override_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_FULLSCREEN_SET), _fullscreen_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_FULLSCREEN_GET), _fullscreen_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_MAXIMIZED_SET), _maximized_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_MAXIMIZED_GET), _maximized_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ICONIFIED_SET), _iconified_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ICONIFIED_GET), _iconified_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_WITHDRAWN_SET), _withdrawn_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_WITHDRAWN_GET), _withdrawn_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_AVAILABLE_PROFILES_SET), _available_profiles_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_AVAILABLE_PROFILES_GET), _available_profiles_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_PROFILE_SET), _profile_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_PROFILE_GET), _profile_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_URGENT_SET), _urgent_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_URGENT_GET), _urgent_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_DEMAND_ATTENTION_SET), _demand_attention_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_DEMAND_ATTENTION_GET), _demand_attention_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_MODAL_SET), _modal_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_MODAL_GET), _modal_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ASPECT_SET), _aspect_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ASPECT_GET), _aspect_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SIZE_BASE_SET), _size_base_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SIZE_BASE_GET), _size_base_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SIZE_STEP_SET), _size_step_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SIZE_STEP_GET), _size_step_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_LAYER_SET), _layer_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_LAYER_GET), _layer_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ROTATION_SET), _rotation_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ROTATION_WITH_RESIZE_SET), _rotation_with_resize_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ROTATION_GET), _rotation_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_STICKY_SET), _sticky_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_STICKY_GET), _sticky_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_KEYBOARD_MODE_SET), _keyboard_mode_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_KEYBOARD_MODE_GET), _keyboard_mode_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_KEYBOARD_WIN_SET), _keyboard_win_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_KEYBOARD_WIN_GET), _keyboard_win_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_INDICATOR_MODE_SET), _indicator_mode_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_INDICATOR_MODE_GET), _indicator_mode_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_INDICATOR_OPACITY_SET), _indicator_opacity_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_INDICATOR_OPACITY_GET), _indicator_opacity_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SCREEN_POSITION_GET), _screen_position_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_FOCUS_GET), _focus_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SCREEN_CONSTRAIN_SET), _screen_constrain_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SCREEN_CONSTRAIN_GET), _screen_constrain_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SCREEN_SIZE_GET), _screen_size_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SCREEN_DPI_GET), _screen_dpi_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_CONFORMANT_SET), _conformant_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_CONFORMANT_GET), _conformant_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_SET), _quickpanel_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_GET), _quickpanel_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_PRIORITY_MAJOR_SET), _quickpanel_priority_major_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_PRIORITY_MAJOR_GET), _quickpanel_priority_major_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_PRIORITY_MINOR_SET), _quickpanel_priority_minor_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_PRIORITY_MINOR_GET), _quickpanel_priority_minor_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_ZONE_SET), _quickpanel_zone_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_ZONE_GET), _quickpanel_zone_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_PROP_FOCUS_SKIP_SET), _prop_focus_skip_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_ILLUME_COMMAND_SEND), _illume_command_send),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_INLINED_IMAGE_OBJECT_GET), _inlined_image_object_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_FOCUS_HIGHLIGHT_ENABLED_SET), _focus_highlight_enabled_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_FOCUS_HIGHLIGHT_ENABLED_GET), _focus_highlight_enabled_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_FOCUS_HIGHLIGHT_STYLE_SET), _focus_highlight_style_set),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_FOCUS_HIGHLIGHT_STYLE_GET), _focus_highlight_style_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_SOCKET_LISTEN), _socket_listen),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_XWINDOW_GET), _xwindow_get),
        EO_OP_FUNC(ELM_OBJ_WIN_ID(ELM_OBJ_WIN_SUB_ID_WL_WINDOW_GET), _wl_window_get),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_WIN_CONSTRUCTOR, "Adds a window object."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_RESIZE_OBJECT_ADD, "Add subobj as a resize object of window obj."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_RESIZE_OBJECT_DEL, "Delete subobj as a resize object of window obj."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_TITLE_SET, "Set the title of the window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_TITLE_GET, "Get the title of the window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_ICON_NAME_SET, "Set the icon name of the window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_ICON_NAME_GET, "Get the icon name of the window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_ROLE_SET, "Set the role of the window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_ROLE_GET, "Get the role of the window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_ICON_OBJECT_SET, "Set a window object's icon."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_ICON_OBJECT_GET, "Get the icon object used for the window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_AUTODEL_SET, "Set the window's autodel state."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_AUTODEL_GET, "Get the window's autodel state."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_ACTIVATE, "Activate a window object."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_LOWER, "Lower a window object."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_RAISE, "Raise a window object."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_CENTER, "Center a window on its screen."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_BORDERLESS_SET, "Set the borderless state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_BORDERLESS_GET, "Get the borderless state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_SHAPED_SET, "Set the shaped state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_SHAPED_GET, "Get the shaped state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_ALPHA_SET, "Set the alpha channel state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_ALPHA_GET, "Get the alpha channel state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_OVERRIDE_SET, "Set the override state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_OVERRIDE_GET, "Get the override state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_FULLSCREEN_SET, "Set the fullscreen state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_FULLSCREEN_GET, "Get the fullscreen state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_MAXIMIZED_SET, "Set the maximized state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_MAXIMIZED_GET, "Get the maximized state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_ICONIFIED_SET, "Set the iconified state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_ICONIFIED_GET, "Get the iconified state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_WITHDRAWN_SET, "Set the withdrawn state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_WITHDRAWN_GET, "Get the withdrawn state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_AVAILABLE_PROFILES_SET, "Set the array of available profiles to a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_AVAILABLE_PROFILES_GET, "Get the array of available profiles of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_PROFILE_SET, "Set the profile of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_PROFILE_GET, "Get the profile of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_URGENT_SET, "Set the urgent state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_URGENT_GET, "Get the urgent state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_DEMAND_ATTENTION_SET, "Set the demand_attention state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_DEMAND_ATTENTION_GET, "Get the demand_attention state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_MODAL_SET, "Set the modal state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_MODAL_GET, "Get the modal state of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_ASPECT_SET, "Set the aspect ratio of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_ASPECT_GET, "Get the aspect ratio of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_SIZE_BASE_SET, "Set the base window size used with stepping calculation."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_SIZE_BASE_GET, "Get the base size of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_SIZE_STEP_SET, "Set the window stepping used with sizing calculation."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_SIZE_STEP_GET, "Get the stepping of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_LAYER_SET, "Set the layer of the window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_LAYER_GET, "Get the layer of the window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_ROTATION_SET, "Set the rotation of the window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_ROTATION_WITH_RESIZE_SET, "Rotates the window and resizes it."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_ROTATION_GET, "Get the rotation of the window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_STICKY_SET, "Set the sticky state of the window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_STICKY_GET, "Get the sticky state of the window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_KEYBOARD_MODE_SET, "Sets the keyboard mode of the window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_KEYBOARD_MODE_GET, "Gets the keyboard mode of the window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_KEYBOARD_WIN_SET, "Sets whether the window is a keyboard."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_KEYBOARD_WIN_GET, "Gets whether the window is a keyboard."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_INDICATOR_MODE_SET, "Sets the indicator mode of the window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_INDICATOR_MODE_GET, "Gets the indicator mode of the window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_INDICATOR_OPACITY_SET, "Sets the indicator opacity mode of the window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_INDICATOR_OPACITY_GET, "Gets the indicator opacity mode of the window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_SCREEN_POSITION_GET, "Get the screen position of a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_FOCUS_GET, "Determine whether a window has focus."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_SCREEN_CONSTRAIN_SET, "Constrain the maximum width and height of a window to the width and height of its screen."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_SCREEN_CONSTRAIN_GET, "Retrieve the constraints on the maximum width and height of a window relative to the width and height of its screen."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_SCREEN_SIZE_GET, "Get screen geometry details for the screen that a window is on."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_SCREEN_DPI_GET, "Get screen dpi for the screen that a window is on."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_CONFORMANT_SET, "Set if this window is an illume conformant window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_CONFORMANT_GET, "Get if this window is an illume conformant window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_SET, "Set a window to be an illume quickpanel window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_GET, "Get if this window is a quickpanel or not."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_PRIORITY_MAJOR_SET, "Set the major priority of a quickpanel window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_PRIORITY_MAJOR_GET, "Get the major priority of a quickpanel window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_PRIORITY_MINOR_SET, "Set the minor priority of a quickpanel window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_PRIORITY_MINOR_GET, "Get the minor priority of a quickpanel window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_ZONE_SET, "Set which zone this quickpanel should appear in."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_QUICKPANEL_ZONE_GET, "Get which zone this quickpanel should appear in."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_PROP_FOCUS_SKIP_SET, "Set the window to be skipped by keyboard focus."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_ILLUME_COMMAND_SEND, "Send a command to the windowing environment."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_INLINED_IMAGE_OBJECT_GET, "Get the inlined image object handle."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_FOCUS_HIGHLIGHT_ENABLED_SET, "Set the enabled status for the focus highlight in a window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_FOCUS_HIGHLIGHT_ENABLED_GET, "Get the enabled value of the focus highlight for this window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_FOCUS_HIGHLIGHT_STYLE_SET, "Set the style for the focus highlight on this window."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_FOCUS_HIGHLIGHT_STYLE_GET, "Get the style set for the focus highlight object."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_SOCKET_LISTEN, "Create a socket to provide the service for Plug widget."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_XWINDOW_GET, "Get the Ecore_X_Window of an Evas_Object."),
     EO_OP_DESCRIPTION(ELM_OBJ_WIN_SUB_ID_WL_WINDOW_GET, "Get the Ecore_Wl_Window of and Evas_Object."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     MY_CLASS_NAME,
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&ELM_OBJ_WIN_BASE_ID, op_desc, ELM_OBJ_WIN_SUB_ID_LAST),
     NULL,
     sizeof(Elm_Win_Smart_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(elm_obj_win_class_get, &class_desc, ELM_OBJ_WIDGET_CLASS, NULL);
