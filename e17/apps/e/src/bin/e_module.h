#ifdef E_TYPEDEFS

#define E_MODULE_API_VERSION 1

typedef struct _E_Module     E_Module;
typedef struct _E_Module_Api E_Module_Api;

#else
#ifndef E_MODULE_H
#define E_MODULE_H

struct _E_Module
{
   E_Object             e_obj_inherit;
   
   E_Module_Api        *api;
   
   char                *name;
   char                *dir;
   void                *handle;
   
   struct {
      void * (*init)        (E_Module *m);
      int    (*shutdown)    (E_Module *m);
      int    (*save)        (E_Module *m);
      int    (*info)        (E_Module *m);
      int    (*about)       (E_Module *m);
   } func;
   
   unsigned char        enabled : 1;
   
   /* the module is allowed to modify these */
   void                *data;
   E_Menu              *config_menu;

   /* modify these but only set them up when the info func is called */
   /* e_module will free them when the module is freed. */
   /* note you will need to malloc (strdup) these fields due to the free */
   char                *label;
   char                *icon_file;
   char                *edje_icon_file;
   char                *edje_icon_key;
};

struct _E_Module_Api
{
   int    version;
};

EAPI int          e_module_init(void);
EAPI int          e_module_shutdown(void);

EAPI E_Module    *e_module_new(char *name);
EAPI int          e_module_save(E_Module *m);
EAPI const char  *e_module_dir_get(E_Module *m);
EAPI int          e_module_enable(E_Module *m);
EAPI int          e_module_disable(E_Module *m);
EAPI int          e_module_enabled_get(E_Module *m);
EAPI int          e_module_save_all(void);
EAPI E_Module    *e_module_find(char *name);
EAPI Evas_List   *e_module_list(void);
EAPI E_Menu      *e_module_menu_new(void);

#endif
#endif
