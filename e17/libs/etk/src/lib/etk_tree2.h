/** @file etk_tree2.h */
#ifndef _ETK_TREE2_H_
#define _ETK_TREE2_H_

#include "etk_widget.h"
#include <stdarg.h>
#include <Evas.h>
#include <Ecore_Job.h>
#include "etk_types.h"

/**
 * @defgroup Etk_Tree2 Etk_Tree2
 * @brief A tree is widget that displays rows of elements of different types (text, image, checkbox, etc), separated
 * into columns
 * @{
 */

/** Gets the type of a tree */
#define ETK_TREE2_TYPE       (etk_tree2_type_get())
/** Casts the object to an Etk_Tree2 */
#define ETK_TREE2(obj)       (ETK_OBJECT_CAST((obj), ETK_TREE2_TYPE, Etk_Tree2))
/** Check if the object is an Etk_Tree2 */
#define ETK_IS_TREE2(obj)    (ETK_OBJECT_CHECK_TYPE((obj), ETK_TREE2_TYPE))

/** Gets the type of a tree column */
#define ETK_TREE2_COL_TYPE        (etk_tree2_col_type_get())
/** Casts the object to an Etk_Tree2_Col */
#define ETK_TREE2_COL(obj)        (ETK_OBJECT_CAST((obj), ETK_TREE2_COL_TYPE, Etk_Tree2_Col))
/** Check if the object is an Etk_Tree2_Col */
#define ETK_IS_TREE2_COL(obj)     (ETK_OBJECT_CHECK_TYPE((obj), ETK_TREE2_COL_TYPE))


/** @brief The different modes of the tree: List (rows can not have children) or tree (rows can have children) */
typedef enum Etk_Tree2_Mode
{
   ETK_TREE2_MODE_LIST,          /**< The rows of a list can not have children (the rows can not be folded/unfolded) */
   ETK_TREE2_MODE_TREE           /**< The rows of a tree can have children */
} Etk_Tree2_Mode;

/**
 * @brief A column of a tree
 * @structinfo
 */
struct Etk_Tree2_Col
{
   /* private: */
   /* Inherit form Etk_Object */
   Etk_Object object;

   int id;
   Etk_Tree2 *tree;
   Etk_Tree2_Model *model;

   int position;
   Etk_Bool resizable;
   Etk_Bool visible;
   Etk_Bool expand;
   
   int xoffset;
   int min_width;
   int width;
   int visible_width;
   
   Etk_Widget *header;
   Evas_Object *clip;
   Evas_Object *separator;
   
   struct
   {
      int (*compare_cb)(Etk_Tree2 *tree, Etk_Tree2_Row *row1, Etk_Tree2_Row *row2, Etk_Tree2_Col *col, void *data);
      void *data;
   } sort;
};

/**
 * @brief A row of a tree
 * @structinfo
 */
struct Etk_Tree2_Row
{
   Etk_Tree2 *tree;

   Etk_Tree2_Row *prev;
   Etk_Tree2_Row *next;
   Etk_Tree2_Row *parent;
   Etk_Tree2_Row *first_child;
   Etk_Tree2_Row *last_child;
   int num_children;
   int num_visible_children;
   
   void **cells_data;
   void *data;
   void (*data_free_cb)(void *data);
   
   unsigned int delete_me : 1;
   unsigned int unfolded : 1;
   unsigned int selected : 1;
};

/**
 * @brief @widget A widget that displays rows of elements of different types, separated into columns
 * @structinfo
 */
struct Etk_Tree2
{
   /* private: */
   /* Inherit from Etk_Widget */
   Etk_Widget widget;

   Etk_Widget *scrolled_view;
   Etk_Widget *scroll_content;
   Etk_Widget *grid;

   int num_cols;
   Etk_Tree2_Col **columns;
   Etk_Tree2_Col *col_to_resize;
   Etk_Bool col_resize_pointer_set;
   int col_resize_orig_width;
   int col_resize_orig_mouse_x;
   Etk_Bool headers_visible;
   Evas_Object *headers_clip;
   Evas_Object *grid_clip;
   
   int total_rows;
   Etk_Tree2_Row root;
   Etk_Tree2_Row *last_selected_row;
   Evas_List *purge_pool;
   Evas_List *row_objects;
   
   int rows_height;
   int scroll_x;
   int scroll_y;
   
   Ecore_Job *purge_job;
   Etk_Color separator_color;
   Etk_Bool tree_contains_headers;
   Etk_Tree2_Mode mode;
   Etk_Bool multiple_select;
   Etk_Bool frozen;
   Etk_Bool built;
};


Etk_Type *etk_tree2_type_get();
Etk_Type *etk_tree2_col_type_get();

Etk_Widget    *etk_tree2_new();
void           etk_tree2_mode_set(Etk_Tree2 *tree, Etk_Tree2_Mode mode);
Etk_Tree2_Mode etk_tree2_mode_get(Etk_Tree2 *tree);
void           etk_tree2_multiple_select_set(Etk_Tree2 *tree, Etk_Bool multiple_select);
Etk_Bool       etk_tree2_multiple_select_get(Etk_Tree2 *tree);
void           etk_tree2_headers_visible_set(Etk_Tree2 *tree, Etk_Bool headers_visible);
Etk_Bool       etk_tree2_headers_visible_get(Etk_Tree2 *tree);
void           etk_tree2_rows_height_set(Etk_Tree2 *tree, int rows_height);
int            etk_tree2_rows_height_get(Etk_Tree2 *tree);

void etk_tree2_build(Etk_Tree2 *tree);
void etk_tree2_freeze(Etk_Tree2 *tree);
void etk_tree2_thaw(Etk_Tree2 *tree);

Etk_Scrolled_View *etk_tree2_scrolled_view_get(Etk_Tree2 *tree);

Etk_Tree2_Col *etk_tree2_col_new(Etk_Tree2 *tree, const char *title, Etk_Tree2_Model *model, int width);
int            etk_tree2_num_cols_get(Etk_Tree2 *tree);
Etk_Tree2_Col *etk_tree2_nth_col_get(Etk_Tree2 *tree, int nth);

void        etk_tree2_col_title_set(Etk_Tree2_Col *col, const char *title);
const char *etk_tree2_col_title_get(Etk_Tree2_Col *col);
void        etk_tree2_col_width_set(Etk_Tree2_Col *col, int width);
int         etk_tree2_col_width_get(Etk_Tree2_Col *col);
void        etk_tree2_col_min_width_set(Etk_Tree2_Col *col, int min_width);
int         etk_tree2_col_min_width_get(Etk_Tree2_Col *col);
void        etk_tree2_col_resizable_set(Etk_Tree2_Col *col, Etk_Bool resizable);
Etk_Bool    etk_tree2_col_resizable_get(Etk_Tree2_Col *col);
void        etk_tree2_col_expand_set(Etk_Tree2_Col *col, Etk_Bool expand);
Etk_Bool    etk_tree2_col_expand_get(Etk_Tree2_Col *col);
void        etk_tree2_col_visible_set(Etk_Tree2_Col *col, Etk_Bool visible);
Etk_Bool    etk_tree2_col_visible_get(Etk_Tree2_Col *col);
void        etk_tree2_col_position_set(Etk_Tree2_Col *col, int position);
int         etk_tree2_col_position_get(Etk_Tree2_Col *col);
void        etk_tree2_col_sort_set(Etk_Tree2_Col *col, int (*compare_cb)(Etk_Tree2 *tree, Etk_Tree2_Row *row1, Etk_Tree2_Row *row2, Etk_Tree2_Col *col, void *data), void *data);

Etk_Tree2_Row *etk_tree2_row_prepend(Etk_Tree2 *tree, Etk_Tree2_Row *parent, ...);
Etk_Tree2_Row *etk_tree2_row_append(Etk_Tree2 *tree, Etk_Tree2_Row *parent, ...);
Etk_Tree2_Row *etk_tree2_row_insert(Etk_Tree2 *tree, Etk_Tree2_Row *parent, Etk_Tree2_Row *after, ...);
Etk_Tree2_Row *etk_tree2_row_insert_valist(Etk_Tree2 *tree, Etk_Tree2_Row *parent, Etk_Tree2_Row *after, va_list args);
/* TODO: Etk_Tree2_Row *etk_tree2_row_insert_sorted(Etk_Tree2 *tree, Etk_Tree2_Row *parent, ...); */
void           etk_tree2_row_delete(Etk_Tree2_Row *row);
void           etk_tree2_clear(Etk_Tree2 *tree);

void etk_tree2_row_fields_set(Etk_Tree2_Row *row, ...);
void etk_tree2_row_fields_set_valist(Etk_Tree2_Row *row, va_list args);
void etk_tree2_row_fields_get(Etk_Tree2_Row *row, ...);
void etk_tree2_row_fields_get_valist(Etk_Tree2_Row *row, va_list args);

void  etk_tree2_row_data_set(Etk_Tree2_Row *row, void *data);
void  etk_tree2_row_data_set_full(Etk_Tree2_Row *row, void *data, void (*free_cb)(void *data));
void *etk_tree2_row_data_get(Etk_Tree2_Row *row);

void     etk_tree2_select_all(Etk_Tree2 *tree);
void     etk_tree2_unselect_all(Etk_Tree2 *tree);
void     etk_tree2_row_select(Etk_Tree2_Row *row);
void     etk_tree2_row_unselect(Etk_Tree2_Row *row);
Etk_Bool etk_tree2_row_is_selected(Etk_Tree2_Row *row);

void     etk_tree2_row_fold(Etk_Tree2_Row *row);
void     etk_tree2_row_unfold(Etk_Tree2_Row *row);
Etk_Bool etk_tree2_row_is_folded(Etk_Tree2_Row *row);

Etk_Tree2_Row *etk_tree2_first_row_get(Etk_Tree2 *tree);
Etk_Tree2_Row *etk_tree2_last_row_get(Etk_Tree2 *tree);
Etk_Tree2_Row *etk_tree2_row_prev_get(Etk_Tree2_Row *row);
Etk_Tree2_Row *etk_tree2_row_next_get(Etk_Tree2_Row *row);
Etk_Tree2_Row *etk_tree2_row_parent_get(Etk_Tree2_Row *row);
Etk_Tree2_Row *etk_tree2_row_first_child_get(Etk_Tree2_Row *row);
Etk_Tree2_Row *etk_tree2_row_last_child_get(Etk_Tree2_Row *row);

/** @} */

#endif
