#ifndef __RGBA
#define __RGBA 1

void    __imlib_RGBASetupContext(Context *ct);
void    __imlib_RGBA_init(void *rd, void *gd, void *bd, int depth, 
			  DATA8 palette_type);

typedef void (*ImlibRGBAFunction)(DATA32*, int, DATA8*,
				  int, int, int, int, int);
ImlibRGBAFunction
__imlib_GetRGBAFunction(int depth, char bgr, char hiq, DATA8 palette_type);
ImlibRGBAFunction
__imlib_GetMaskFunction(char hiq);

#ifdef DO_MMX_ASM
void __imlib_mmx_rgb555_fast(DATA32*, int, DATA8*, int, int, int, int, int);
void __imlib_mmx_bgr555_fast(DATA32*, int, DATA8*, int, int, int, int, int);
void __imlib_mmx_rgb565_fast(DATA32*, int, DATA8*, int, int, int, int, int);
void __imlib_mmx_bgr565_fast(DATA32*, int, DATA8*, int, int, int, int, int);
#endif
#endif
