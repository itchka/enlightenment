
#include "main.h"

static Evas_Object *_pager;
static Evas_Object *_area, *_empty, *_image;

static Evas_Object *_image_file_entry, *_image_check_border, *_image_check_shadow;

Evas_Object *rightpanel_create()
{
    Evas_Object *vbox, *bt, *fr, *bx, *bx2, *tb, *entry, *sc, *lbl, *ck;

    vbox = elm_box_add(win);
    evas_object_size_hint_weight_set(vbox, -1.0, -1.0);
    evas_object_size_hint_align_set(vbox, -1.0, -1.0);
    evas_object_show(vbox);

    bt = elm_button_add(win);
    elm_button_label_set(bt, D_("Add a new area"));
    evas_object_size_hint_weight_set(bt, -1.0, -1.0);
    evas_object_size_hint_align_set(bt, -1.0, 0.0);
    evas_object_smart_callback_add(bt, "clicked", utils_edit_area_add, NULL);
    evas_object_show(bt);
    elm_box_pack_end(vbox, bt);

    bt = elm_button_add(win);
    elm_button_label_set(bt, D_("Test 2"));
    evas_object_size_hint_weight_set(bt, -1.0, -1.0);
    evas_object_size_hint_align_set(bt, -1.0, 0.0);
    evas_object_show(bt);
    elm_box_pack_end(vbox, bt);

    //
    _pager = elm_pager_add(win);
    evas_object_size_hint_weight_set(_pager, 1.0, 1.0);
    evas_object_size_hint_align_set(_pager, -1.0, -1.0);
    elm_box_pack_end(vbox, _pager);
    evas_object_show(_pager);
    //

    //Empty
    bx = elm_box_add(win);
    _empty = bx;
    elm_pager_content_push(_pager, bx);

    //Area
    bx = elm_box_add(win);
    evas_object_size_hint_weight_set(bx, -1.0, 1.0);
    evas_object_size_hint_align_set(bx, -1.0, -1.0);
    evas_object_show(bx);

    fr = elm_frame_add(win);
    _area = fr;
    evas_object_size_hint_weight_set(bx, 1.0, 1.0);
    evas_object_size_hint_align_set(bx, -1.0, -1.0);
    elm_frame_label_set(fr, D_("Area"));
    elm_frame_content_set(fr, bx);
    evas_object_show(fr);

    bt = elm_button_add(win);
    elm_button_label_set(bt, D_("UP"));
    evas_object_size_hint_weight_set(bt, 0.0, 0.0);
    evas_object_size_hint_align_set(bt, -1.0, 0.0);
    evas_object_smart_callback_add(bt, "clicked", utils_edit_area_up, NULL);
    evas_object_show(bt);
    elm_box_pack_end(bx, bt);

    bt = elm_button_add(win);
    elm_button_label_set(bt, D_("DOWN"));
    evas_object_size_hint_weight_set(bt, -1.0, -1.0);
    evas_object_size_hint_align_set(bt, -1.0, 0.0);
    evas_object_smart_callback_add(bt, "clicked", utils_edit_area_down, NULL);
    evas_object_show(bt);
    elm_box_pack_end(bx,bt);

    bt = elm_button_add(win);
    elm_button_label_set(bt, D_("Add an image"));
    evas_object_size_hint_weight_set(bt, -1.0, -1.0);
    evas_object_size_hint_align_set(bt, -1.0, 0.0);
    evas_object_smart_callback_add(bt, "clicked", utils_edit_area_image_add, NULL);
    evas_object_show(bt);
    elm_box_pack_end(bx,bt);

    bx2 = elm_box_add(win);
    evas_object_size_hint_weight_set(bx2, -1.0, 1.0);
    evas_object_size_hint_align_set(bx2, -1.0, -1.0);
    evas_object_show(bx2);
    elm_box_pack_end(bx, bx2);

    bt = elm_button_add(win);
    elm_button_label_set(bt, D_("Delete the area"));
    evas_object_color_set(bt, 255, 0, 0, 255);
    evas_object_size_hint_weight_set(bt, -1.0, -1.0);
    evas_object_size_hint_align_set(bt, -1.0, 0.0);
    evas_object_smart_callback_add(bt, "clicked", utils_edit_area_delete, NULL);
    evas_object_show(bt);
    elm_box_pack_end(bx, bt);

    elm_pager_content_push(_pager, fr);
    //

    //Image
    bx = elm_box_add(win);
    evas_object_size_hint_weight_set(bx, -1.0, 1.0);
    evas_object_size_hint_align_set(bx, -1.0, -1.0);
    evas_object_show(bx);

    fr = elm_frame_add(win);
    _image = fr;
    evas_object_size_hint_weight_set(bx, 1.0, 1.0);
    evas_object_size_hint_align_set(bx, -1.0, -1.0);
    elm_frame_label_set(fr, D_("Image"));
    elm_frame_content_set(fr, bx);
    evas_object_show(fr);

    tb = elm_table_add(win);
    evas_object_size_hint_weight_set(tb, -1.0, 1.0);
    evas_object_size_hint_align_set(tb, -1.0, -1.0);
    evas_object_show(tb);
    elm_box_pack_end(bx, tb);

    bt = elm_button_add(win);
    elm_button_label_set(bt, D_("UP"));
    evas_object_size_hint_weight_set(bt, 0.0, 0.0);
    evas_object_size_hint_align_set(bt, -1.0, 0.0);
    //evas_object_smart_callback_add(bt, "clicked", utils_edit_area_up, NULL);
    evas_object_show(bt);
    elm_table_pack(tb, bt, 0, 0, 2, 1);

    bt = elm_button_add(win);
    elm_button_label_set(bt, D_("DOWN"));
    evas_object_size_hint_weight_set(bt, -1.0, -1.0);
    evas_object_size_hint_align_set(bt, -1.0, 0.0);
    //evas_object_smart_callback_add(bt, "clicked", utils_edit_area_down, NULL);
    evas_object_show(bt);
    elm_table_pack(tb, bt, 0, 1, 2, 1);

    bt = elm_button_add(win);
    elm_button_label_set(bt, D_("Image : "));
    evas_object_size_hint_weight_set(bt, -1.0, -1.0);
    evas_object_size_hint_align_set(bt, -1.0, 0.0);
    evas_object_smart_callback_add(bt, "clicked", utils_edit_image_file_change, NULL);
    evas_object_show(bt);
    elm_table_pack(tb, bt, 0, 2, 1, 1);

    entry = elm_entry_add(win);
    _image_file_entry = entry;
    elm_entry_editable_set(entry, 0);
    elm_entry_entry_set(entry, "tesgq");
    evas_object_size_hint_weight_set(entry, -1.0, -1.0);
    evas_object_size_hint_align_set(entry, -1.0, 0.0);
    evas_object_show(entry);

    sc = elm_scroller_add(win);
    elm_table_pack(tb, sc, 1, 2, 1,1);
    evas_object_size_hint_weight_set(sc, 1.0, 0.0);
    evas_object_size_hint_align_set(sc, -1.0, 0.5);
    elm_scroller_content_min_limit(sc, 0, 1);
    elm_scroller_policy_set(sc, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
    elm_scroller_content_set(sc, entry);
    evas_object_show(sc);

    lbl = elm_label_add(win);
    elm_label_label_set(lbl, D_("Border : "));
    evas_object_show(lbl);
    elm_table_pack(tb, lbl, 0, 3, 1, 1);
    ck = elm_check_add(win);
    _image_check_border = ck;
    evas_object_show(ck);
    evas_object_smart_callback_add(ck, "changed", utils_edit_image_border_change, NULL);
    elm_table_pack(tb, ck, 1, 3, 1, 1);

    lbl = elm_label_add(win);
    elm_label_label_set(lbl, D_("Shadow : "));
    evas_object_show(lbl);
    elm_table_pack(tb, lbl, 0, 4, 1, 1);
    ck = elm_check_add(win);
    _image_check_shadow = ck;
    evas_object_show(ck);
    evas_object_smart_callback_add(ck, "changed", utils_edit_image_shadow_change, NULL);
    elm_table_pack(tb, ck, 1, 4, 1, 1);



    bx2 = elm_box_add(win);
    evas_object_size_hint_weight_set(bx2, -1.0, 1.0);
    evas_object_size_hint_align_set(bx2, -1.0, -1.0);
    evas_object_show(bx2);
    elm_box_pack_end(bx, bx2);

    bt = elm_button_add(win);
    elm_button_label_set(bt, D_("Delete the image"));
    evas_object_color_set(bt, 255, 0, 0, 255);
    evas_object_size_hint_weight_set(bt, -1.0, -1.0);
    evas_object_size_hint_align_set(bt, -1.0, 0.0);
    //evas_object_smart_callback_add(bt, "clicked", utils_edit_area_delete, NULL);
    evas_object_show(bt);
    elm_box_pack_end(bx, bt);

    elm_pager_content_push(_pager, fr);
    //
    elm_pager_content_promote(_pager, _empty);

    return vbox;
}

void rightpanel_area_show()
{
    elm_pager_content_promote(_pager, _area);
}

void rightpanel_empty_show()
{
    elm_pager_content_promote(_pager, _empty);
}

void rightpanel_image_show()
{
    elm_pager_content_promote(_pager, _image);
}

void rightpanel_image_data_set(const char* file, int border, int shadow)
{
    if(file)
        elm_entry_entry_set(_image_file_entry, file);
    if(border > -1)
        elm_check_state_set(_image_check_border, border);
    if(shadow > -1)
        elm_check_state_set(_image_check_shadow, shadow);
}

