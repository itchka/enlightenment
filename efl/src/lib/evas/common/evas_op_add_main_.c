#include "evas_common.h"
static RGBA_Gfx_Func     op_add_span_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];
static RGBA_Gfx_Pt_Func  op_add_pt_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];

static void op_add_init(void);
static void op_add_shutdown(void);

static RGBA_Gfx_Func op_add_pixel_span_get(RGBA_Image *src, RGBA_Image *dst, int pixels);
static RGBA_Gfx_Func op_add_color_span_get(DATA32 col, RGBA_Image *dst, int pixels);
static RGBA_Gfx_Func op_add_pixel_color_span_get(RGBA_Image *src, DATA32 col, RGBA_Image *dst, int pixels);
static RGBA_Gfx_Func op_add_mask_color_span_get(DATA32 col, RGBA_Image *dst, int pixels);
static RGBA_Gfx_Func op_add_pixel_mask_span_get(RGBA_Image *src, RGBA_Image *dst, int pixels);

static RGBA_Gfx_Pt_Func op_add_pixel_pt_get(Image_Entry_Flags src_flags, RGBA_Image *dst);
static RGBA_Gfx_Pt_Func op_add_color_pt_get(DATA32 col, RGBA_Image *dst);
static RGBA_Gfx_Pt_Func op_add_pixel_color_pt_get(Image_Entry_Flags src_flags, DATA32 col, RGBA_Image *dst);
static RGBA_Gfx_Pt_Func op_add_mask_color_pt_get(DATA32 col, RGBA_Image *dst);
static RGBA_Gfx_Pt_Func op_add_pixel_mask_pt_get(Image_Entry_Flags src_flags, RGBA_Image *dst);

static RGBA_Gfx_Compositor  _composite_add = { "add",
 op_add_init, op_add_shutdown,
 op_add_pixel_span_get, op_add_color_span_get,
 op_add_pixel_color_span_get, op_add_mask_color_span_get,
 op_add_pixel_mask_span_get,
 op_add_pixel_pt_get, op_add_color_pt_get,
 op_add_pixel_color_pt_get, op_add_mask_color_pt_get,
 op_add_pixel_mask_pt_get
 };

RGBA_Gfx_Compositor  *
evas_common_gfx_compositor_add_get(void)
{
   return &(_composite_add);
}

static RGBA_Gfx_Func     op_add_rel_span_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];
static RGBA_Gfx_Pt_Func  op_add_rel_pt_funcs[SP_LAST][SM_LAST][SC_LAST][DP_LAST][CPU_LAST];

static void op_add_rel_init(void);
static void op_add_rel_shutdown(void);

static RGBA_Gfx_Func op_add_rel_pixel_span_get(RGBA_Image *src, RGBA_Image *dst, int pixels);
static RGBA_Gfx_Func op_add_rel_color_span_get(DATA32 col, RGBA_Image *dst, int pixels);
static RGBA_Gfx_Func op_add_rel_pixel_color_span_get(RGBA_Image *src, DATA32 col, RGBA_Image *dst, int pixels);
static RGBA_Gfx_Func op_add_rel_mask_color_span_get(DATA32 col, RGBA_Image *dst, int pixels);
static RGBA_Gfx_Func op_add_rel_pixel_mask_span_get(RGBA_Image *src, RGBA_Image *dst, int pixels);

static RGBA_Gfx_Pt_Func op_add_rel_pixel_pt_get(Image_Entry_Flags src_flags, RGBA_Image *dst);
static RGBA_Gfx_Pt_Func op_add_rel_color_pt_get(DATA32 col, RGBA_Image *dst);
static RGBA_Gfx_Pt_Func op_add_rel_pixel_color_pt_get(Image_Entry_Flags src_flags, DATA32 col, RGBA_Image *dst);
static RGBA_Gfx_Pt_Func op_add_rel_mask_color_pt_get(DATA32 col, RGBA_Image *dst);
static RGBA_Gfx_Pt_Func op_add_rel_pixel_mask_pt_get(Image_Entry_Flags src_flags, RGBA_Image *dst);

static RGBA_Gfx_Compositor  _composite_add_rel = { "add_rel",
 op_add_rel_init, op_add_rel_shutdown,
 op_add_rel_pixel_span_get, op_add_rel_color_span_get,
 op_add_rel_pixel_color_span_get, op_add_rel_mask_color_span_get,
 op_add_rel_pixel_mask_span_get,
 op_add_rel_pixel_pt_get, op_add_rel_color_pt_get,
 op_add_rel_pixel_color_pt_get, op_add_rel_mask_color_pt_get,
 op_add_rel_pixel_mask_pt_get
 };

RGBA_Gfx_Compositor  *
evas_common_gfx_compositor_add_rel_get(void)
{
   return &(_composite_add_rel);
}


# include "./evas_op_add/op_add_pixel_.c"
# include "./evas_op_add/op_add_color_.c"
# include "./evas_op_add/op_add_pixel_color_.c"
# include "./evas_op_add/op_add_pixel_mask_.c"
# include "./evas_op_add/op_add_mask_color_.c"
//# include "./evas_op_add/op_add_pixel_mask_color_.c"

# include "./evas_op_add/op_add_pixel_i386.c"
# include "./evas_op_add/op_add_color_i386.c"
# include "./evas_op_add/op_add_pixel_color_i386.c"
# include "./evas_op_add/op_add_pixel_mask_i386.c"
# include "./evas_op_add/op_add_mask_color_i386.c"
//# include "op_add_pixel_mask_color_.c"

static void
op_add_init(void)
{
   memset(op_add_span_funcs, 0, sizeof(op_add_span_funcs));
   memset(op_add_pt_funcs, 0, sizeof(op_add_pt_funcs));
#ifdef BUILD_MMX
   init_add_pixel_span_funcs_mmx();
   init_add_pixel_color_span_funcs_mmx();
   init_add_pixel_mask_span_funcs_mmx();
   init_add_color_span_funcs_mmx();
   init_add_mask_color_span_funcs_mmx();

   init_add_pixel_pt_funcs_mmx();
   init_add_pixel_color_pt_funcs_mmx();
   init_add_pixel_mask_pt_funcs_mmx();
   init_add_color_pt_funcs_mmx();
   init_add_mask_color_pt_funcs_mmx();
#endif
   init_add_pixel_span_funcs_c();
   init_add_pixel_color_span_funcs_c();
   init_add_rel_pixel_mask_span_funcs_c();
   init_add_color_span_funcs_c();
   init_add_mask_color_span_funcs_c();

   init_add_pixel_pt_funcs_c();
   init_add_pixel_color_pt_funcs_c();
   init_add_rel_pixel_mask_pt_funcs_c();
   init_add_color_pt_funcs_c();
   init_add_mask_color_pt_funcs_c();
}

static void
op_add_shutdown(void)
{
}

static RGBA_Gfx_Func
add_gfx_span_func_cpu(int s, int m, int c, int d)
{
   RGBA_Gfx_Func  func = NULL;
   int cpu = CPU_N;
#ifdef BUILD_MMX
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
     {
	cpu = CPU_MMX;
	func = op_add_span_funcs[s][m][c][d][cpu];
	if (func) return func;
     }
#endif
   cpu = CPU_C;
   func = op_add_span_funcs[s][m][c][d][cpu];
   if (func) return func;
   return func;
}

static RGBA_Gfx_Func
op_add_pixel_span_get(RGBA_Image *src, RGBA_Image *dst, int pixels EINA_UNUSED)
{
   int  s = SP_AN, m = SM_N, c = SC_N, d = DP_AN;

   if (src && src->cache_entry.flags.alpha)
	s = SP;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return add_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_add_color_span_get(DATA32 col, RGBA_Image *dst, int pixels EINA_UNUSED)
{
   int  s = SP_N, m = SM_N, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return add_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_add_pixel_color_span_get(RGBA_Image *src, DATA32 col, RGBA_Image *dst, int pixels EINA_UNUSED)
{
   int  s = SP_AN, m = SM_N, c = SC_AN, d = DP_AN;

   if (src && src->cache_entry.flags.alpha)
	s = SP;
   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return add_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_add_mask_color_span_get(DATA32 col, RGBA_Image *dst, int pixels EINA_UNUSED)
{
   int  s = SP_N, m = SM_AS, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return add_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_add_pixel_mask_span_get(RGBA_Image *src, RGBA_Image *dst, int pixels EINA_UNUSED)
{
   int  s = SP_AN, m = SM_AS, c = SC_N, d = DP_AN;

   if (src && src->cache_entry.flags.alpha)
	s = SP;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return add_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
add_gfx_pt_func_cpu(int s, int m, int c, int d)
{
   RGBA_Gfx_Pt_Func  func = NULL;
   int cpu = CPU_N;
#ifdef BUILD_MMX
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
     {
	cpu = CPU_MMX;
	func = op_add_pt_funcs[s][m][c][d][cpu];
	if (func) return func;
     }
#endif
   cpu = CPU_C;
   func = op_add_pt_funcs[s][m][c][d][cpu];
   if (func) return func;
   return func;
}

static RGBA_Gfx_Pt_Func
op_add_pixel_pt_get(Image_Entry_Flags src_flags, RGBA_Image *dst)
{
   int  s = SP_AN, m = SM_N, c = SC_N, d = DP_AN;

   if (src_flags.alpha)
	s = SP;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return add_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_add_color_pt_get(DATA32 col, RGBA_Image *dst)
{
   int  s = SP_N, m = SM_N, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return add_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_add_pixel_color_pt_get(Image_Entry_Flags src_flags, DATA32 col, RGBA_Image *dst)
{
   int  s = SP_AN, m = SM_N, c = SC_AN, d = DP_AN;

   if (src_flags.alpha)
	s = SP;
   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return add_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_add_mask_color_pt_get(DATA32 col, RGBA_Image *dst)
{
   int  s = SP_N, m = SM_AS, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return add_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_add_pixel_mask_pt_get(Image_Entry_Flags src_flags, RGBA_Image *dst)
{
   int  s = SP_AN, m = SM_AS, c = SC_N, d = DP_AN;

   if (src_flags.alpha)
	s = SP;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return add_gfx_pt_func_cpu(s, m, c, d);
}



static void
op_add_rel_init(void)
{
   memset(op_add_rel_span_funcs, 0, sizeof(op_add_rel_span_funcs));
   memset(op_add_rel_pt_funcs, 0, sizeof(op_add_rel_pt_funcs));
#ifdef BUILD_MMX
   init_add_rel_pixel_span_funcs_mmx();
   init_add_rel_pixel_color_span_funcs_mmx();
   init_add_rel_pixel_mask_span_funcs_mmx();
   init_add_rel_color_span_funcs_mmx();
   init_add_rel_mask_color_span_funcs_mmx();

   init_add_rel_pixel_pt_funcs_mmx();
   init_add_rel_pixel_color_pt_funcs_mmx();
   init_add_rel_pixel_mask_pt_funcs_mmx();
   init_add_rel_color_pt_funcs_mmx();
   init_add_rel_mask_color_pt_funcs_mmx();
#endif
   init_add_rel_pixel_span_funcs_c();
   init_add_rel_pixel_color_span_funcs_c();
   init_add_rel_pixel_mask_span_funcs_c();
   init_add_rel_color_span_funcs_c();
   init_add_rel_mask_color_span_funcs_c();

   init_add_rel_pixel_pt_funcs_c();
   init_add_rel_pixel_color_pt_funcs_c();
   init_add_rel_pixel_mask_pt_funcs_c();
   init_add_rel_color_pt_funcs_c();
   init_add_rel_mask_color_pt_funcs_c();
}

static void
op_add_rel_shutdown(void)
{
}

static RGBA_Gfx_Func
add_rel_gfx_span_func_cpu(int s, int m, int c, int d)
{
   RGBA_Gfx_Func  func = NULL;
   int cpu = CPU_N;
#ifdef BUILD_MMX
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
     {
	cpu = CPU_MMX;
	func = op_add_rel_span_funcs[s][m][c][d][cpu];
	if (func) return func;
     }
#endif
   cpu = CPU_C;
   func = op_add_rel_span_funcs[s][m][c][d][cpu];
   if (func) return func;
   return func;
}

static RGBA_Gfx_Func
op_add_rel_pixel_span_get(RGBA_Image *src, RGBA_Image *dst, int pixels EINA_UNUSED)
{
   int  s = SP_AN, m = SM_N, c = SC_N, d = DP_AN;

   if (src && src->cache_entry.flags.alpha)
	s = SP;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return add_rel_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_add_rel_color_span_get(DATA32 col, RGBA_Image *dst, int pixels EINA_UNUSED)
{
   int  s = SP_N, m = SM_N, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return add_rel_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_add_rel_pixel_color_span_get(RGBA_Image *src, DATA32 col, RGBA_Image *dst, int pixels EINA_UNUSED)
{
   int  s = SP_AN, m = SM_N, c = SC_AN, d = DP_AN;

   if (src && src->cache_entry.flags.alpha)
	s = SP;
   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return add_rel_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_add_rel_mask_color_span_get(DATA32 col, RGBA_Image *dst, int pixels EINA_UNUSED)
{
   int  s = SP_N, m = SM_AS, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return add_rel_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Func
op_add_rel_pixel_mask_span_get(RGBA_Image *src, RGBA_Image *dst, int pixels EINA_UNUSED)
{
   int  s = SP_AN, m = SM_AS, c = SC_N, d = DP_AN;

   if (src && src->cache_entry.flags.alpha)
	s = SP;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return add_rel_gfx_span_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
add_rel_gfx_pt_func_cpu(int s, int m, int c, int d)
{
   RGBA_Gfx_Pt_Func  func = NULL;
   int cpu = CPU_N;
#ifdef BUILD_MMX
   if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
     {
	cpu = CPU_MMX;
	func = op_add_rel_pt_funcs[s][m][c][d][cpu];
	if (func) return func;
     }
#endif
   cpu = CPU_C;
   func = op_add_rel_pt_funcs[s][m][c][d][cpu];
   if (func) return func;
   return func;
}

static RGBA_Gfx_Pt_Func
op_add_rel_pixel_pt_get(Image_Entry_Flags src_flags, RGBA_Image *dst)
{
   int  s = SP_AN, m = SM_N, c = SC_N, d = DP_AN;

   if (src_flags.alpha)
	s = SP;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return add_rel_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_add_rel_color_pt_get(DATA32 col, RGBA_Image *dst)
{
   int  s = SP_N, m = SM_N, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return add_rel_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_add_rel_pixel_color_pt_get(Image_Entry_Flags src_flags, DATA32 col, RGBA_Image *dst)
{
   int  s = SP_AN, m = SM_N, c = SC_AN, d = DP_AN;

   if (src_flags.alpha)
	s = SP;
   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return add_rel_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_add_rel_mask_color_pt_get(DATA32 col, RGBA_Image *dst)
{
   int  s = SP_N, m = SM_AS, c = SC_AN, d = DP_AN;

   if ((col >> 24) < 255)
	c = SC;
   if (col == (col | 0x00ffffff))
	c = SC_AA;
   if (col == 0xffffffff)
	c = SC_N;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return add_rel_gfx_pt_func_cpu(s, m, c, d);
}

static RGBA_Gfx_Pt_Func
op_add_rel_pixel_mask_pt_get(Image_Entry_Flags src_flags, RGBA_Image *dst)
{
   int  s = SP_AN, m = SM_AS, c = SC_N, d = DP_AN;

   if (src_flags.alpha)
	s = SP;
   if (dst && dst->cache_entry.flags.alpha)
	d = DP;
   return add_rel_gfx_pt_func_cpu(s, m, c, d);
}
