#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <Imlib2.h>
#include <E.h>
#include <Engrave.h>
#include <Ecore.h>

#include "config.h"

/* TODO: 
 * make Esetroot respect our options
 * add empty -s -t -c flags that will take current bg and apply option
 * implement -g (get current bg)
 */

static int e_bg_type;
static int e_bg_no_load = 0;
static char *e_bg_img_file = NULL;

enum E_Bg_Types
{
     E_BG_TILE = 1,
     E_BG_SCALE = 2,
     E_BG_CENTER = 3,
     E_BG_FIT = 4,
     E_BG_GET = 5,
     E_BG_NO_LOAD = 6,
};
typedef enum E_Bg_Types E_Bg_Types;


void _e_bg_bg_help() { 
   printf("Usage: e17setroot <imagename> | <eet>\n");
}

static int _e_bg_bg_get(void *data, int type, void *event) {   
   E_Response_Background_Get *bg;   
   bg = event;   
   printf("Bg: %s\n", bg->data);
   ecore_main_loop_quit();      
}

/* parse command line options */
void _e_bg_bg_parseargs(int argc, char **argv) {
   int c;

   static char *options = "tscfghn";
   static struct option long_options[] = {
	{"tile",   0, 0, E_BG_TILE},
	{"scale",  0, 0, E_BG_SCALE},
	{"center", 0, 0, E_BG_CENTER},
	{"fit",    0, 0, E_BG_FIT},
	{"get",    0, 0, E_BG_GET},
	{"noload", 0, 0, E_BG_NO_LOAD},
	{0,        0, 0, 0}
   };

   while((c = getopt_long (argc, argv, options, long_options, NULL)) != -1) {
      switch (c) {
	 /* tile */
       case E_BG_TILE:	   
       case 't':	    
	 e_bg_type = E_BG_TILE;
	 break;

	 /* scale */
       case E_BG_SCALE:
       case 's':
	 e_bg_type = E_BG_SCALE;
	 break;
	 
	 /* center */
       case E_BG_CENTER:
       case 'c':
	 e_bg_type = E_BG_CENTER;
	 break;
	 
	 /* fit */
       case E_BG_FIT:
       case 'f':
	 break;

	 /* get current bg */
       case E_BG_GET:
       case 'g':
         e_bg_type = E_BG_GET;
	 break;
	 
       case E_BG_NO_LOAD:
       case 'n':
	 e_bg_no_load = 1;
	 break;

	 /* show help screen */
       case 'h':
       default:
	 _e_bg_bg_help();
	 break;
      }
   }

   if (optind < argc)
        e_bg_img_file = argv[optind];
}

/* return dir from a full path filename */
char *_e_bg_bg_file_getdir(char *path) {
   char *ptr;
   char *c;
   char *dir;
   int i = 0;

   ptr=path;
   c=strrchr(ptr, '/');
   if (!c)
     return ".";
   dir = malloc(strlen(path) + 1);

   while(ptr != c) {
      dir[i] = *ptr;
      ptr++;
      i++;
   }

   dir[i] = '\0';
   return dir;
}

/* return filename from a full path filename */
char *_e_bg_bg_file_getfile(char *path) {
   char *ptr;
   char *c;
   char *file;
   int i;

   i = 0;
   ptr = path;
   c = strrchr(ptr, '/');
   if (!c)
     return path;
   file = malloc(strlen(ptr) + 1);

   while(ptr != c) {
      file[i] = *ptr;
      ptr++;
      i++;
   }

   file[i] = '\0';
   return c;
}

/* strip extention from a file */
char *_e_bg_bg_file_stripext(char *path) {
   char *ptr;
   char *c;
   char *file;
   int i;

   i = 0;
   ptr = path;
   c = strrchr(ptr,'.');
   file = malloc(strlen(path));

   while(ptr != c) {
      file[i] = *ptr;
      ptr++;
      i++;
   }

   file[i] = '\0';
   return file;
}

int _e_bg_bg_set(char *filename) {

   e_background_set(filename);
   return 1;
}

void _e_bg_bg_eet_gen(char *filename) {
   int w, h;
   char *file, *dir, *eet_file, *filenoext, *esetroot;
   Imlib_Image *im;
   Engrave_File *eet;
   Engrave_Image *image;
   Engrave_Group *grp;
   Engrave_Part *part;
   Engrave_Part_State *ps;

   /* make sure we got a file name */
   if (!filename) return;

   if (strcmp(filename + strlen(filename) - 4, ".eet") == 0) {
      _e_bg_bg_set(filename);
      ecore_main_loop_quit();
      return;
   }
   
   file = _e_bg_bg_file_getfile(filename);
   dir = _e_bg_bg_file_getdir(filename);

   filenoext = _e_bg_bg_file_stripext(filename);
   filenoext = _e_bg_bg_file_getfile(filenoext);

   /* Set up eet path */
   eet_file = malloc(strlen(getenv("HOME")) +  strlen("/.e/e/backgrounds/") 
		+ strlen(filenoext) + strlen(".eet") + 1);
   strcpy(eet_file, getenv("HOME"));
   strcat(eet_file, "/.e/e/backgrounds/");
   strcat(eet_file, filenoext);
   strcat(eet_file, ".eet");
 
   /* Determine image width / height */
   im = imlib_load_image(filename);
   imlib_context_set_image(im);
   w = imlib_image_get_width();
   h = imlib_image_get_height();
   imlib_free_image_and_decache();

   /* create the .eet */
   eet = engrave_file_new();
   engrave_file_image_dir_set(eet, dir);
   image = engrave_image_new(file, ENGRAVE_IMAGE_TYPE_COMP, 0);
   engrave_file_image_add(eet, image);

   grp = engrave_group_new();
   engrave_group_name_set(grp, "desktop/background");
   engrave_file_group_add(eet, grp);

   part = engrave_part_new(ENGRAVE_PART_TYPE_IMAGE);
   engrave_part_name_set(part, "background_image");
   engrave_group_part_add(grp, part);

   ps = engrave_part_state_new();
   engrave_part_state_name_set(ps, "default", 0.0);
   engrave_part_state_image_normal_set(ps, image);
      
   switch(e_bg_type) {      
    case E_BG_CENTER:
      engrave_part_state_max_size_set(ps, w, h);
      break;
      
    case E_BG_SCALE:
      break;
      
    case E_BG_FIT:
      break;
      
    case E_BG_TILE:
      /* FIXME: This is a temp until dj2 fixes engrave */
      //engrave_part_state_max_size_set(ps, w, h);
      engrave_part_state_fill_size_relative_set(ps, 0.0, 0.0);
      engrave_part_state_fill_size_offset_set(ps, w, h);
      break;
      
    default:
      /* FIXME: This is a temp until dj2 fixes engrave */
      //engrave_part_state_max_size_set(ps, w, h);
      engrave_part_state_fill_size_relative_set(ps, 0.0, 0.0);
      engrave_part_state_fill_size_offset_set(ps, w, h);
      break;
   }

   engrave_part_state_add(part, ps);   
   
   engrave_eet_output(eet, eet_file);
   engrave_file_free(eet);

   /* don't do anything more if we're not loading the image */
   if (e_bg_no_load)
      return;

   /* set the background */
   if (!_e_bg_bg_set(eet_file)) 
      return;

   /* If we're using pseudo-trans for eterm, then this will help */
   esetroot = malloc(strlen("Esetroot ") + strlen(filename) + 1);
   strcpy(esetroot, "Esetroot ");
   strcat(esetroot, filename);
   system(esetroot);
   free(esetroot);
}

int main(int argc, char **argv)
{
   _e_bg_bg_parseargs(argc, argv);

   if (!e_bg_no_load) {
      if (!e_init(":0.0")) {
         printf("Can't connect to enlightenment, perhaps we are not on :0.0!\n");
         return 0;
      }
   }

   if (e_bg_type == E_BG_GET) {
      ecore_init();
      ecore_event_handler_add(E_RESPONSE_BACKGROUND_GET, _e_bg_bg_get, NULL);
      e_background_get();
      ecore_main_loop_begin();
      ecore_shutdown();
   } else
      _e_bg_bg_eet_gen(e_bg_img_file);
  
   if (!e_bg_no_load)
      e_shutdown();

   return 0;
}

