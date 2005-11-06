/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifdef E_TYPEDEFS
#else
#ifndef E_ILIST_H
#define E_ILIST_H

EAPI Evas_Object *e_ilist_add                   (Evas *evas);
EAPI void         e_ilist_icon_size_set         (Evas_Object *obj, Evas_Coord w, Evas_Coord h);
EAPI void         e_ilist_append                (Evas_Object *obj, Evas_Object *icon, char *label, void (*func) (void *data, void *data2), void *data, void *data2);
EAPI void         e_ilist_select_set            (Evas_Object *obj, int n);
EAPI int          e_ilist_select_get            (Evas_Object *obj);
EAPI void        *e_ilist_select_data_get       (Evas_Object *obj);
EAPI void        *e_ilist_select_data2_get      (Evas_Object *obj);
EAPI void         e_ilist_selected_geometry_get (Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);
EAPI void         e_ilist_min_size_get          (Evas_Object *obj, Evas_Coord *w, Evas_Coord *h);
    
#endif
#endif
