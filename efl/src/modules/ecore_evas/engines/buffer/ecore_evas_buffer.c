#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_evas_buffer_private.h"

static int _ecore_evas_init_count = 0;

static const char *interface_buffer_name = "buffer";
static const int   interface_buffer_version = 1;


static Ecore_Evas_Interface_Buffer *_ecore_evas_buffer_interface_new(void);

static int
_ecore_evas_buffer_init(void)
{
   _ecore_evas_init_count++;
   return _ecore_evas_init_count;
}

static int
_ecore_evas_buffer_shutdown(void)
{
   _ecore_evas_init_count--;
   if (_ecore_evas_init_count < 0) _ecore_evas_init_count = 0;
   return _ecore_evas_init_count;
}

static void
_ecore_evas_buffer_free(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;

   if (bdata->image)
     {
        Ecore_Evas *ee2;

        ee2 = evas_object_data_get(bdata->image, "Ecore_Evas_Parent");
        evas_object_del(bdata->image);
        ee2->sub_ecore_evas = eina_list_remove(ee2->sub_ecore_evas, ee);
     }
   else
     {
        bdata->free_func(bdata->data,
			 bdata->pixels);
     }

   free(bdata);
   _ecore_evas_buffer_shutdown();
}

static void
_ecore_evas_resize(Ecore_Evas *ee, int w, int h)
{
   Evas_Engine_Info_Buffer *einfo;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   int stride = 0;

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->req.w = w;
   ee->req.h = h;
   if ((w == ee->w) && (h == ee->h)) return;
   ee->w = w;
   ee->h = h;
   evas_output_size_set(ee->evas, ee->w, ee->h);
   evas_output_viewport_set(ee->evas, 0, 0, ee->w, ee->h);
   evas_damage_rectangle_add(ee->evas, 0, 0, ee->w, ee->h);

   if (bdata->image)
     {
        bdata->pixels = evas_object_image_data_get(bdata->image, 1);
        stride = evas_object_image_stride_get(bdata->image);
     }
   else
     {
        if (bdata->pixels)
          bdata->free_func(bdata->data,
                                      bdata->pixels);
        bdata->pixels = bdata->alloc_func(bdata->data,
					  ee->w * ee->h * sizeof(int));
        stride = ee->w * sizeof(int);
     }

   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        if (ee->alpha)
          einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
        else
          einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_RGB32;
        einfo->info.dest_buffer = bdata->pixels;
        einfo->info.dest_buffer_row_bytes = stride;
        einfo->info.use_color_key = 0;
        einfo->info.alpha_threshold = 0;
        einfo->info.func.new_update_region = NULL;
        einfo->info.func.free_update_region = NULL;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
          }
     }
   if (bdata->image)
      evas_object_image_data_set(bdata->image, bdata->pixels);
   if (ee->func.fn_resize) ee->func.fn_resize(ee);
}

static void
_ecore_evas_move_resize(Ecore_Evas *ee, int x EINA_UNUSED, int y EINA_UNUSED, int w, int h)
{
   _ecore_evas_resize(ee, w, h);
}

static void
_ecore_evas_show(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;

   if (bdata->image) return;
   if (ee->prop.focused) return;
   ee->prop.focused = 1;
   evas_focus_in(ee->evas);
   if (ee->func.fn_focus_in) ee->func.fn_focus_in(ee);
}

static int
_ecore_evas_buffer_render(Ecore_Evas *ee)
{
   Eina_List *updates = NULL, *l, *ll;
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Ecore_Evas *ee2;
   int rend = 0;

   EINA_LIST_FOREACH(ee->sub_ecore_evas, ll, ee2)
     {
        if (ee2->func.fn_pre_render) ee2->func.fn_pre_render(ee2);
        if (ee2->engine.func->fn_render)
           rend |= ee2->engine.func->fn_render(ee2);
        if (ee2->func.fn_post_render) ee2->func.fn_post_render(ee2);
     }
   if (bdata->image)
     {
        int w, h;

        evas_object_image_size_get(bdata->image, &w, &h);
        if ((w != ee->w) || (h != ee->h))
           _ecore_evas_resize(ee, w, h);
        bdata->pixels = evas_object_image_data_get(bdata->image, 1);
     }
   if (bdata->pixels)
     {
        updates = evas_render_updates(ee->evas);
     }
   if (bdata->image)
     {
        Eina_Rectangle *r;

        evas_object_image_data_set(bdata->image, bdata->pixels);
        EINA_LIST_FOREACH(updates, l, r)
           evas_object_image_data_update_add(bdata->image,
                                             r->x, r->y, r->w, r->h);
     }
   if (updates)
     {
        evas_render_updates_free(updates);
        _ecore_evas_idle_timeout_update(ee);
     }

   return updates ? 1 : rend;
}

// NOTE: if you fix this, consider fixing ecore_evas_ews.c as it is similar!
static void
_ecore_evas_buffer_coord_translate(Ecore_Evas *ee, Evas_Coord *x, Evas_Coord *y)
{
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   Evas_Coord xx, yy, ww, hh, fx, fy, fw, fh;

   evas_object_geometry_get(bdata->image, &xx, &yy, &ww, &hh);
   evas_object_image_fill_get(bdata->image, &fx, &fy, &fw, &fh);

   if (fw < 1) fw = 1;
   if (fh < 1) fh = 1;

   if (evas_object_map_get(bdata->image) &&
       evas_object_map_enable_get(bdata->image))
     {
        fx = 0; fy = 0;
        fw = ee->w; fh = ee->h;
        ww = ee->w; hh = ee->h;
     }
   
   if ((fx == 0) && (fy == 0) && (fw == ww) && (fh == hh))
     {
        *x = (ee->w * (*x - xx)) / fw;
        *y = (ee->h * (*y - yy)) / fh;
     }
   else
     {
        xx = (*x - xx) - fx;
        while (xx < 0) xx += fw;
        while (xx > fw) xx -= fw;
        *x = (ee->w * xx) / fw;

        yy = (*y - yy) - fy;
        while (yy < 0) yy += fh;
        while (yy > fh) yy -= fh;
        *y = (ee->h * yy) / fh;
     }
}

static void
_ecore_evas_buffer_transfer_modifiers_locks(Evas *e, Evas *e2)
{
   const char *mods[] = 
     { "Shift", "Control", "Alt", "Meta", "Hyper", "Super", NULL };
   const char *locks[] = 
     { "Scroll_Lock", "Num_Lock", "Caps_Lock", NULL };
   int i;
   
   for (i = 0; mods[i]; i++)
     {
        if (evas_key_modifier_is_set(evas_key_modifier_get(e), mods[i]))
          evas_key_modifier_on(e2, mods[i]);
        else
          evas_key_modifier_off(e2, mods[i]);
     }
   for (i = 0; locks[i]; i++)
     {
        if (evas_key_lock_is_set(evas_key_lock_get(e), locks[i]))
          evas_key_lock_on(e2, locks[i]);
        else
          evas_key_lock_off(e2, locks[i]);
     }
}

static void
_ecore_evas_buffer_cb_mouse_in(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee;
   Evas_Event_Mouse_In *ev;

   ee = data;
   ev = event_info;
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   evas_event_feed_mouse_in(ee->evas, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_mouse_out(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee;
   Evas_Event_Mouse_Out *ev;

   ee = data;
   ev = event_info;
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   evas_event_feed_mouse_out(ee->evas, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_mouse_down(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Mouse_Down *ev;

   ee = data;
   ev = event_info;
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   evas_event_feed_mouse_down(ee->evas, ev->button, ev->flags, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_mouse_up(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Mouse_Up *ev;

   ee = data;
   ev = event_info;
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   evas_event_feed_mouse_up(ee->evas, ev->button, ev->flags, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_mouse_move(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Mouse_Move *ev;
   Evas_Coord x, y;

   ee = data;
   ev = event_info;
   x = ev->cur.canvas.x;
   y = ev->cur.canvas.y;
   _ecore_evas_buffer_coord_translate(ee, &x, &y);
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   _ecore_evas_mouse_move_process(ee, x, y, ev->timestamp);
}

static void
_ecore_evas_buffer_cb_mouse_wheel(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Mouse_Wheel *ev;

   ee = data;
   ev = event_info;
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   evas_event_feed_mouse_wheel(ee->evas, ev->direction, ev->z, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_multi_down(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Multi_Down *ev;
   Evas_Coord x, y, xx, yy;
   double xf, yf;

   ee = data;
   ev = event_info;
   x = ev->canvas.x;
   y = ev->canvas.y;
   xx = x;
   yy = y;
   _ecore_evas_buffer_coord_translate(ee, &x, &y);
   xf = (ev->canvas.xsub - (double)xx) + (double)x;
   yf = (ev->canvas.ysub - (double)yy) + (double)y;
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   evas_event_feed_multi_down(ee->evas, ev->device, x, y, ev->radius, ev->radius_x, ev->radius_y, ev->pressure, ev->angle, xf, yf, ev->flags, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_multi_up(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Multi_Up *ev;
   Evas_Coord x, y, xx, yy;
   double xf, yf;

   ee = data;
   ev = event_info;
   x = ev->canvas.x;
   y = ev->canvas.y;
   xx = x;
   yy = y;
   _ecore_evas_buffer_coord_translate(ee, &x, &y);
   xf = (ev->canvas.xsub - (double)xx) + (double)x;
   yf = (ev->canvas.ysub - (double)yy) + (double)y;
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   evas_event_feed_multi_up(ee->evas, ev->device, x, y, ev->radius, ev->radius_x, ev->radius_y, ev->pressure, ev->angle, xf, yf, ev->flags, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_multi_move(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Multi_Move *ev;
   Evas_Coord x, y, xx, yy;
   double xf, yf;

   ee = data;
   ev = event_info;
   x = ev->cur.canvas.x;
   y = ev->cur.canvas.y;
   xx = x;
   yy = y;
   _ecore_evas_buffer_coord_translate(ee, &x, &y);
   xf = (ev->cur.canvas.xsub - (double)xx) + (double)x;
   yf = (ev->cur.canvas.ysub - (double)yy) + (double)y;
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   evas_event_feed_multi_move(ee->evas, ev->device, x, y, ev->radius, ev->radius_x, ev->radius_y, ev->pressure, ev->angle, xf, yf, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_free(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee;

   ee = data;
   if (ee->driver) _ecore_evas_free(ee);
}

static void
_ecore_evas_buffer_cb_key_down(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Key_Down *ev;

   ee = data;
   ev = event_info;
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   evas_event_feed_key_down(ee->evas, ev->keyname, ev->key, ev->string, ev->compose, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_key_up(void *data, Evas *e, Evas_Object *obj EINA_UNUSED, void *event_info)
{
   Ecore_Evas *ee;
   Evas_Event_Key_Up *ev;

   ee = data;
   ev = event_info;
   _ecore_evas_buffer_transfer_modifiers_locks(e, ee->evas);
   evas_event_feed_key_up(ee->evas, ev->keyname, ev->key, ev->string, ev->compose, ev->timestamp, NULL);
}

static void
_ecore_evas_buffer_cb_focus_in(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee;

   ee = data;
   ee->prop.focused = 1;
   evas_focus_in(ee->evas);
   if (ee->func.fn_focus_in) ee->func.fn_focus_in(ee);
}

static void
_ecore_evas_buffer_cb_focus_out(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee;

   ee = data;
   ee->prop.focused = 0;
   evas_focus_out(ee->evas);
   if (ee->func.fn_focus_out) ee->func.fn_focus_out(ee);
}

static void
_ecore_evas_buffer_cb_show(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee;

   ee = data;
   ee->visible = 1;
   if (ee->func.fn_show) ee->func.fn_show(ee);
}

static void
_ecore_evas_buffer_cb_hide(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Ecore_Evas *ee;

   ee = data;
   ee->visible = 0;
   if (ee->func.fn_hide) ee->func.fn_hide(ee);
}

static void
_ecore_evas_buffer_alpha_set(Ecore_Evas *ee, int alpha)
{
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;
   if (((ee->alpha) && (alpha)) || ((!ee->alpha) && (!alpha))) return;
   ee->alpha = alpha;
   if (bdata->image)
      evas_object_image_alpha_set(bdata->image, ee->alpha);
   else
     {
        Evas_Engine_Info_Buffer *einfo;
        
        einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(ee->evas);
        if (einfo)
          {
             if (ee->alpha)
               einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
             else
               einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_RGB32;
             evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo);
          }
     }
}

static void
_ecore_evas_buffer_profile_set(Ecore_Evas *ee, const char *profile)
{
   _ecore_evas_window_profile_free(ee);
   ee->prop.profile.name = NULL;

   if (profile)
     {
        ee->prop.profile.name = (char *)eina_stringshare_add(profile);

        /* just change ee's state.*/
        if (ee->func.fn_state_change)
          ee->func.fn_state_change(ee);
     }
}

static Ecore_Evas_Engine_Func _ecore_buffer_engine_func =
{
   _ecore_evas_buffer_free,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     _ecore_evas_resize,
     _ecore_evas_move_resize,
     NULL,
     NULL,
     _ecore_evas_show,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     _ecore_evas_buffer_alpha_set,
     NULL, //transparent
     NULL, // profiles_set
     _ecore_evas_buffer_profile_set,

     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,

     _ecore_evas_buffer_render,
     NULL, // screen_geometry_get
     NULL  // screen_dpi_get
};

static void *
_ecore_evas_buffer_pix_alloc(void *data EINA_UNUSED, int size)
{
   return malloc(size);
}

static void
_ecore_evas_buffer_pix_free(void *data EINA_UNUSED, void *pix)
{
   free(pix);
}

EAPI Ecore_Evas *
ecore_evas_buffer_allocfunc_new_internal(int w, int h, void *(*alloc_func) (void *data, int size), void (*free_func) (void *data, void *pix), const void *data)
{
   Evas_Engine_Info_Buffer *einfo;
   Ecore_Evas_Engine_Buffer_Data *bdata;
   Ecore_Evas_Interface_Buffer *iface;
   Ecore_Evas *ee;
   int rmethod;

   if ((!alloc_func) || (!free_func)) return NULL;
   rmethod = evas_render_method_lookup("buffer");
   if (!rmethod) return NULL;
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;
   bdata = calloc(1, sizeof(Ecore_Evas_Engine_Buffer_Data));
   if (!bdata)
     {
	free(ee);
	return NULL;
     }

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_buffer_init();

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_buffer_engine_func;
   ee->engine.data = bdata;
   bdata->alloc_func = alloc_func;
   bdata->free_func = free_func;
   bdata->data = (void *)data;

   iface = _ecore_evas_buffer_interface_new();
   ee->engine.ifaces = eina_list_append(ee->engine.ifaces, iface);

   ee->driver = "buffer";

   if (w < 1) w = 1;
   if (h < 1) h = 1;
   ee->rotation = 0;
   ee->visible = 1;
   ee->w = w;
   ee->h = h;
   ee->req.w = ee->w;
   ee->req.h = ee->h;
   ee->profile_supported = 1;

   ee->prop.max.w = 0;
   ee->prop.max.h = 0;
   ee->prop.layer = 0;
   ee->prop.focused = 1;
   ee->prop.borderless = 1;
   ee->prop.override = 1;
   ee->prop.maximized = 1;
   ee->prop.fullscreen = 0;
   ee->prop.withdrawn = 0;
   ee->prop.sticky = 0;

   /* init evas here */
   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, rmethod);
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

   bdata->pixels = bdata->alloc_func(bdata->data, w * h * sizeof(int));

   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_RGB32;
        einfo->info.dest_buffer = bdata->pixels;
        einfo->info.dest_buffer_row_bytes = ee->w * sizeof(int);
        einfo->info.use_color_key = 0;
        einfo->info.alpha_threshold = 0;
        einfo->info.func.new_update_region = NULL;
        einfo->info.func.free_update_region = NULL;
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
             ecore_evas_free(ee);
             return NULL;
          }
     }
   else
     {
        ERR("evas_engine_info_set() init engine '%s' failed.", ee->driver);
        ecore_evas_free(ee);
        return NULL;
     }
   evas_key_modifier_add(ee->evas, "Shift");
   evas_key_modifier_add(ee->evas, "Control");
   evas_key_modifier_add(ee->evas, "Alt");
   evas_key_modifier_add(ee->evas, "Meta");
   evas_key_modifier_add(ee->evas, "Hyper");
   evas_key_modifier_add(ee->evas, "Super");
   evas_key_lock_add(ee->evas, "Caps_Lock");
   evas_key_lock_add(ee->evas, "Num_Lock");
   evas_key_lock_add(ee->evas, "Scroll_Lock");

   evas_event_feed_mouse_in(ee->evas, 0, NULL);

   _ecore_evas_register(ee);

   evas_event_feed_mouse_in(ee->evas, (unsigned int)((unsigned long long)(ecore_time_get() * 1000.0) & 0xffffffff), NULL);
   
   return ee;
}

EAPI Ecore_Evas *
ecore_evas_buffer_new_internal(int w, int h)
{
    return ecore_evas_buffer_allocfunc_new_internal
     (w, h, _ecore_evas_buffer_pix_alloc, _ecore_evas_buffer_pix_free, NULL);
}

const void *
_ecore_evas_buffer_pixels_get(Ecore_Evas *ee)
{
   Ecore_Evas_Engine_Buffer_Data *bdata = ee->engine.data;

   if (!ee)
     {
        CRIT("Ecore_Evas is missing");
        return NULL;
     }
   _ecore_evas_buffer_render(ee);
   return bdata->pixels;
}

EAPI Evas_Object *
ecore_evas_object_image_new_internal(Ecore_Evas *ee_target)
{
   Evas_Object *o;
   Ecore_Evas_Engine_Buffer_Data *bdata;
   Evas_Engine_Info_Buffer *einfo;
   Ecore_Evas_Interface_Buffer *iface;
   Ecore_Evas *ee;
   int rmethod;
   int w = 1, h = 1;

   if (!ee_target) return NULL;

   rmethod = evas_render_method_lookup("buffer");
   if (!rmethod) return NULL;
   ee = calloc(1, sizeof(Ecore_Evas));
   if (!ee) return NULL;
   bdata = calloc(1, sizeof(Ecore_Evas_Engine_Buffer_Data));
   if (!bdata)
     {
	free(ee);
	return NULL;
     }

   ee->engine.data = bdata;
   iface = _ecore_evas_buffer_interface_new();
   ee->engine.ifaces = eina_list_append(ee->engine.ifaces, iface);

   o = evas_object_image_add(ee_target->evas);
   evas_object_image_content_hint_set(o, EVAS_IMAGE_CONTENT_HINT_DYNAMIC);
   evas_object_image_colorspace_set(o, EVAS_COLORSPACE_ARGB8888);
   evas_object_image_alpha_set(o, 0);
   evas_object_image_size_set(o, w, h);

   ECORE_MAGIC_SET(ee, ECORE_MAGIC_EVAS);

   _ecore_evas_buffer_init();

   ee->engine.func = (Ecore_Evas_Engine_Func *)&_ecore_buffer_engine_func;

   ee->driver = "buffer";

   ee->rotation = 0;
   ee->visible = 0;
   ee->w = w;
   ee->h = h;
   ee->req.w = ee->w;
   ee->req.h = ee->h;
   ee->profile_supported = 1;

   ee->prop.max.w = 0;
   ee->prop.max.h = 0;
   ee->prop.layer = 0;
   ee->prop.focused = 0;
   ee->prop.borderless = 1;
   ee->prop.override = 1;
   ee->prop.maximized = 0;
   ee->prop.fullscreen = 0;
   ee->prop.withdrawn = 0;
   ee->prop.sticky = 0;

   /* init evas here */
   ee->evas = evas_new();
   evas_data_attach_set(ee->evas, ee);
   evas_output_method_set(ee->evas, rmethod);
   evas_output_size_set(ee->evas, w, h);
   evas_output_viewport_set(ee->evas, 0, 0, w, h);

   bdata->image = o;
   evas_object_data_set(bdata->image, "Ecore_Evas", ee);
   evas_object_data_set(bdata->image, "Ecore_Evas_Parent", ee_target);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_IN,
                                  _ecore_evas_buffer_cb_mouse_in, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_OUT,
                                  _ecore_evas_buffer_cb_mouse_out, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_DOWN,
                                  _ecore_evas_buffer_cb_mouse_down, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_UP,
                                  _ecore_evas_buffer_cb_mouse_up, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_MOVE,
                                  _ecore_evas_buffer_cb_mouse_move, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MOUSE_WHEEL,
                                  _ecore_evas_buffer_cb_mouse_wheel, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MULTI_DOWN,
                                  _ecore_evas_buffer_cb_multi_down, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MULTI_UP,
                                  _ecore_evas_buffer_cb_multi_up, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_MULTI_MOVE,
                                  _ecore_evas_buffer_cb_multi_move, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_FREE,
                                  _ecore_evas_buffer_cb_free, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_KEY_DOWN,
                                  _ecore_evas_buffer_cb_key_down, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_KEY_UP,
                                  _ecore_evas_buffer_cb_key_up, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_FOCUS_IN,
                                  _ecore_evas_buffer_cb_focus_in, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_FOCUS_OUT,
                                  _ecore_evas_buffer_cb_focus_out, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_SHOW,
                                  _ecore_evas_buffer_cb_show, ee);
   evas_object_event_callback_add(bdata->image,
                                  EVAS_CALLBACK_HIDE,
                                  _ecore_evas_buffer_cb_hide, ee);
   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(ee->evas);
   if (einfo)
     {
        bdata->pixels = evas_object_image_data_get(o, 1);
        einfo->info.depth_type = EVAS_ENGINE_BUFFER_DEPTH_ARGB32;
        einfo->info.dest_buffer = bdata->pixels;
        einfo->info.dest_buffer_row_bytes = evas_object_image_stride_get(o);
        einfo->info.use_color_key = 0;
        einfo->info.alpha_threshold = 0;
        einfo->info.func.new_update_region = NULL;
        einfo->info.func.free_update_region = NULL;
        evas_object_image_data_set(o, bdata->pixels);
        if (!evas_engine_info_set(ee->evas, (Evas_Engine_Info *)einfo))
          {
             ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
             ecore_evas_free(ee);
             return NULL;
          }
     }
   else
     {
        ERR("evas_engine_info_set() for engine '%s' failed.", ee->driver);
        ecore_evas_free(ee);
        return NULL;
     }
   evas_key_modifier_add(ee->evas, "Shift");
   evas_key_modifier_add(ee->evas, "Control");
   evas_key_modifier_add(ee->evas, "Alt");
   evas_key_modifier_add(ee->evas, "Meta");
   evas_key_modifier_add(ee->evas, "Hyper");
   evas_key_modifier_add(ee->evas, "Super");
   evas_key_lock_add(ee->evas, "Caps_Lock");
   evas_key_lock_add(ee->evas, "Num_Lock");
   evas_key_lock_add(ee->evas, "Scroll_Lock");

   ee_target->sub_ecore_evas = eina_list_append(ee_target->sub_ecore_evas, ee);

   return o;
}

static Ecore_Evas_Interface_Buffer *
_ecore_evas_buffer_interface_new(void)
{
   Ecore_Evas_Interface_Buffer *iface;

   iface = calloc(1, sizeof(Ecore_Evas_Interface_Buffer));
   if (!iface) return NULL;

   iface->base.name = interface_buffer_name;
   iface->base.version = interface_buffer_version;

   iface->pixels_get = _ecore_evas_buffer_pixels_get;
   iface->render = _ecore_evas_buffer_render;

   return iface;
}
