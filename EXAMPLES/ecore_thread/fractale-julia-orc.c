
/* autogenerated from fractale-julia.orc */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Eina.h>

#ifndef _ORC_INTEGER_TYPEDEFS_
#define _ORC_INTEGER_TYPEDEFS_
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#include <stdint.h>
typedef int8_t orc_int8;
typedef int16_t orc_int16;
typedef int32_t orc_int32;
typedef int64_t orc_int64;
typedef uint8_t orc_uint8;
typedef uint16_t orc_uint16;
typedef uint32_t orc_uint32;
typedef uint64_t orc_uint64;
#define ORC_UINT64_C(x) UINT64_C(x)
#elif defined(_MSC_VER)
typedef signed __int8 orc_int8;
typedef signed __int16 orc_int16;
typedef signed __int32 orc_int32;
typedef signed __int64 orc_int64;
typedef unsigned __int8 orc_uint8;
typedef unsigned __int16 orc_uint16;
typedef unsigned __int32 orc_uint32;
typedef unsigned __int64 orc_uint64;
#define ORC_UINT64_C(x) (x##Ui64)
#define inline __inline
#else
#include <limits.h>
typedef signed char orc_int8;
typedef short orc_int16;
typedef int orc_int32;
typedef unsigned char orc_uint8;
typedef unsigned short orc_uint16;
typedef unsigned int orc_uint32;
#if INT_MAX == LONG_MAX
typedef long long orc_int64;
typedef unsigned long long orc_uint64;
#define ORC_UINT64_C(x) (x##ULL)
#else
typedef long orc_int64;
typedef unsigned long orc_uint64;
#define ORC_UINT64_C(x) (x##UL)
#endif
#endif
typedef union { orc_int16 i; orc_int8 x2[2]; } orc_union16;
typedef union { orc_int32 i; float f; orc_int16 x2[2]; orc_int8 x4[4]; } orc_union32;
typedef union { orc_int64 i; double f; orc_int32 x2[2]; float x2f[2]; orc_int16 x4[4]; } orc_union64;
#endif
#ifndef ORC_RESTRICT
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define ORC_RESTRICT restrict
#elif defined(__GNUC__) && __GNUC__ >= 4
#define ORC_RESTRICT __restrict__
#else
#define ORC_RESTRICT
#endif
#endif

#ifndef DISABLE_ORC
#include <orc/orc.h>
#endif
void _fractal_outerloop_fp_za (Eina_F16p16 * ORC_RESTRICT d1, const orc_uint32 * ORC_RESTRICT s1, int p1, int n);
void _fractal_outerloop_fp_zb (Eina_F16p16 * ORC_RESTRICT d1, const orc_uint32 * ORC_RESTRICT s1, int p1, int n);
void _fractal_inerloop (orc_uint8 * ORC_RESTRICT d1, orc_uint8 * ORC_RESTRICT d2, Eina_F16p16 * ORC_RESTRICT d3, Eina_F16p16 * ORC_RESTRICT d4, const orc_uint8 * ORC_RESTRICT s1, const Eina_F16p16 * ORC_RESTRICT s2, const Eina_F16p16 * ORC_RESTRICT s3, int p1, int p2, int n);
void color_conversion (orc_uint32 * ORC_RESTRICT d1, const orc_uint8 * ORC_RESTRICT s1, int p1, int p2, int n);


/* begin Orc C target preamble */
#define ORC_CLAMP(x,a,b) ((x)<(a) ? (a) : ((x)>(b) ? (b) : (x)))
#define ORC_ABS(a) ((a)<0 ? -(a) : (a))
#define ORC_MIN(a,b) ((a)<(b) ? (a) : (b))
#define ORC_MAX(a,b) ((a)>(b) ? (a) : (b))
#define ORC_SB_MAX 127
#define ORC_SB_MIN (-1-ORC_SB_MAX)
#define ORC_UB_MAX 255
#define ORC_UB_MIN 0
#define ORC_SW_MAX 32767
#define ORC_SW_MIN (-1-ORC_SW_MAX)
#define ORC_UW_MAX 65535
#define ORC_UW_MIN 0
#define ORC_SL_MAX 2147483647
#define ORC_SL_MIN (-1-ORC_SL_MAX)
#define ORC_UL_MAX 4294967295U
#define ORC_UL_MIN 0
#define ORC_CLAMP_SB(x) ORC_CLAMP(x,ORC_SB_MIN,ORC_SB_MAX)
#define ORC_CLAMP_UB(x) ORC_CLAMP(x,ORC_UB_MIN,ORC_UB_MAX)
#define ORC_CLAMP_SW(x) ORC_CLAMP(x,ORC_SW_MIN,ORC_SW_MAX)
#define ORC_CLAMP_UW(x) ORC_CLAMP(x,ORC_UW_MIN,ORC_UW_MAX)
#define ORC_CLAMP_SL(x) ORC_CLAMP(x,ORC_SL_MIN,ORC_SL_MAX)
#define ORC_CLAMP_UL(x) ORC_CLAMP(x,ORC_UL_MIN,ORC_UL_MAX)
#define ORC_SWAP_W(x) ((((x)&0xff)<<8) | (((x)&0xff00)>>8))
#define ORC_SWAP_L(x) ((((x)&0xff)<<24) | (((x)&0xff00)<<8) | (((x)&0xff0000)>>8) | (((x)&0xff000000)>>24))
#define ORC_SWAP_Q(x) ((((x)&ORC_UINT64_C(0xff))<<56) | (((x)&ORC_UINT64_C(0xff00))<<40) | (((x)&ORC_UINT64_C(0xff0000))<<24) | (((x)&ORC_UINT64_C(0xff000000))<<8) | (((x)&ORC_UINT64_C(0xff00000000))>>8) | (((x)&ORC_UINT64_C(0xff0000000000))>>24) | (((x)&ORC_UINT64_C(0xff000000000000))>>40) | (((x)&ORC_UINT64_C(0xff00000000000000))>>56))
#define ORC_PTR_OFFSET(ptr,offset) ((void *)(((unsigned char *)(ptr)) + (offset)))
#define ORC_DENORMAL(x) ((x) & ((((x)&0x7f800000) == 0) ? 0xff800000 : 0xffffffff))
#define ORC_ISNAN(x) ((((x)&0x7f800000) == 0x7f800000) && (((x)&0x007fffff) != 0))
#define ORC_DENORMAL_DOUBLE(x) ((x) & ((((x)&ORC_UINT64_C(0x7ff0000000000000)) == 0) ? ORC_UINT64_C(0xfff0000000000000) : ORC_UINT64_C(0xffffffffffffffff)))
#define ORC_ISNAN_DOUBLE(x) ((((x)&ORC_UINT64_C(0x7ff0000000000000)) == ORC_UINT64_C(0x7ff0000000000000)) && (((x)&ORC_UINT64_C(0x000fffffffffffff)) != 0))
#ifndef ORC_RESTRICT
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define ORC_RESTRICT restrict
#elif defined(__GNUC__) && __GNUC__ >= 4
#define ORC_RESTRICT __restrict__
#else
#define ORC_RESTRICT
#endif
#endif
/* end Orc C target preamble */



/* _fractal_outerloop_fp_za */
#ifdef DISABLE_ORC
void
_fractal_outerloop_fp_za (Eina_F16p16 * ORC_RESTRICT d1, const orc_uint32 * ORC_RESTRICT s1, int p1, int n){
  int i;
  orc_union32 * ORC_RESTRICT ptr0;
  const orc_union32 * ORC_RESTRICT ptr4;
  orc_union32 var33;
  orc_union32 var34;
  orc_union32 var35;
  orc_union32 var36;
  orc_union32 var37;
  orc_union32 var38;
  orc_union32 var39;

  ptr0 = (orc_union32 *)d1;
  ptr4 = (orc_union32 *)s1;

    /* 1: loadpl */
    var34.i = (int)0x00000004; /* 4 or 1.97626e-323f */
    /* 3: loadpl */
    var35.i = p1;
    /* 5: loadpl */
    var36.i = (int)0x00020000; /* 131072 or 6.47582e-319f */

  for (i = 0; i < n; i++) {
    /* 0: loadl */
    var33 = ptr4[i];
    /* 2: mulll */
    var38.i = (var33.i * var34.i) & 0xffffffff;
    /* 4: mulll */
    var39.i = (var38.i * var35.i) & 0xffffffff;
    /* 6: subssl */
    var37.i = ORC_CLAMP_SL((orc_int64)var39.i - (orc_int64)var36.i);
    /* 7: storel */
    ptr0[i] = var37;
  }

}

#else
static void
_backup__fractal_outerloop_fp_za (OrcExecutor * ORC_RESTRICT ex)
{
  int i;
  int n = ex->n;
  orc_union32 * ORC_RESTRICT ptr0;
  const orc_union32 * ORC_RESTRICT ptr4;
  orc_union32 var33;
  orc_union32 var34;
  orc_union32 var35;
  orc_union32 var36;
  orc_union32 var37;
  orc_union32 var38;
  orc_union32 var39;

  ptr0 = (orc_union32 *)ex->arrays[0];
  ptr4 = (orc_union32 *)ex->arrays[4];

    /* 1: loadpl */
    var34.i = (int)0x00000004; /* 4 or 1.97626e-323f */
    /* 3: loadpl */
    var35.i = ex->params[24];
    /* 5: loadpl */
    var36.i = (int)0x00020000; /* 131072 or 6.47582e-319f */

  for (i = 0; i < n; i++) {
    /* 0: loadl */
    var33 = ptr4[i];
    /* 2: mulll */
    var38.i = (var33.i * var34.i) & 0xffffffff;
    /* 4: mulll */
    var39.i = (var38.i * var35.i) & 0xffffffff;
    /* 6: subssl */
    var37.i = ORC_CLAMP_SL((orc_int64)var39.i - (orc_int64)var36.i);
    /* 7: storel */
    ptr0[i] = var37;
  }

}

void
_fractal_outerloop_fp_za (Eina_F16p16 * ORC_RESTRICT d1, const orc_uint32 * ORC_RESTRICT s1, int p1, int n)
{
  OrcExecutor _ex, *ex = &_ex;
  static volatile int p_inited = 0;
  static OrcCode *c = 0;
  void (*func) (OrcExecutor *);

  if (!p_inited) {
    orc_once_mutex_lock ();
    if (!p_inited) {
      OrcProgram *p;

      p = orc_program_new ();
      orc_program_set_name (p, "_fractal_outerloop_fp_za");
      orc_program_set_backup_function (p, _backup__fractal_outerloop_fp_za);
      orc_program_add_destination (p, 4, "d1");
      orc_program_add_source (p, 4, "s1");
      orc_program_add_constant (p, 4, 0x00000004, "c1");
      orc_program_add_constant (p, 4, 0x00020000, "c2");
      orc_program_add_parameter (p, 4, "p1");
      orc_program_add_temporary (p, 4, "t1");

      orc_program_append_2 (p, "mulll", 0, ORC_VAR_T1, ORC_VAR_S1, ORC_VAR_C1, ORC_VAR_D1);
      orc_program_append_2 (p, "mulll", 0, ORC_VAR_T1, ORC_VAR_T1, ORC_VAR_P1, ORC_VAR_D1);
      orc_program_append_2 (p, "subssl", 0, ORC_VAR_D1, ORC_VAR_T1, ORC_VAR_C2, ORC_VAR_D1);

      orc_program_compile (p);
      c = orc_program_take_code (p);
      orc_program_free (p);
    }
    p_inited = TRUE;
    orc_once_mutex_unlock ();
  }
  ex->arrays[ORC_VAR_A2] = c;
  ex->program = 0;

  ex->n = n;
  ex->arrays[ORC_VAR_D1] = d1;
  ex->arrays[ORC_VAR_S1] = (void *)s1;
  ex->params[ORC_VAR_P1] = p1;

  func = c->exec;
  func (ex);
}
#endif


/* _fractal_outerloop_fp_zb */
#ifdef DISABLE_ORC
void
_fractal_outerloop_fp_zb (Eina_F16p16 * ORC_RESTRICT d1, const orc_uint32 * ORC_RESTRICT s1, int p1, int n){
  int i;
  orc_union32 * ORC_RESTRICT ptr0;
  const orc_union32 * ORC_RESTRICT ptr4;
  orc_union32 var33;
  orc_union32 var34;
  orc_union32 var35;
  orc_union32 var36;
  orc_union32 var37;
  orc_union32 var38;
  orc_union32 var39;

  ptr0 = (orc_union32 *)d1;
  ptr4 = (orc_union32 *)s1;

    /* 1: loadpl */
    var34.i = (int)0xfffffffc; /* -4 or -nanf */
    /* 3: loadpl */
    var35.i = p1;
    /* 5: loadpl */
    var36.i = (int)0x00020000; /* 131072 or 6.47582e-319f */

  for (i = 0; i < n; i++) {
    /* 0: loadl */
    var33 = ptr4[i];
    /* 2: mulll */
    var38.i = (var33.i * var34.i) & 0xffffffff;
    /* 4: mulll */
    var39.i = (var38.i * var35.i) & 0xffffffff;
    /* 6: addssl */
    var37.i = ORC_CLAMP_SL((orc_int64)var39.i + (orc_int64)var36.i);
    /* 7: storel */
    ptr0[i] = var37;
  }

}

#else
static void
_backup__fractal_outerloop_fp_zb (OrcExecutor * ORC_RESTRICT ex)
{
  int i;
  int n = ex->n;
  orc_union32 * ORC_RESTRICT ptr0;
  const orc_union32 * ORC_RESTRICT ptr4;
  orc_union32 var33;
  orc_union32 var34;
  orc_union32 var35;
  orc_union32 var36;
  orc_union32 var37;
  orc_union32 var38;
  orc_union32 var39;

  ptr0 = (orc_union32 *)ex->arrays[0];
  ptr4 = (orc_union32 *)ex->arrays[4];

    /* 1: loadpl */
    var34.i = (int)0xfffffffc; /* -4 or -nanf */
    /* 3: loadpl */
    var35.i = ex->params[24];
    /* 5: loadpl */
    var36.i = (int)0x00020000; /* 131072 or 6.47582e-319f */

  for (i = 0; i < n; i++) {
    /* 0: loadl */
    var33 = ptr4[i];
    /* 2: mulll */
    var38.i = (var33.i * var34.i) & 0xffffffff;
    /* 4: mulll */
    var39.i = (var38.i * var35.i) & 0xffffffff;
    /* 6: addssl */
    var37.i = ORC_CLAMP_SL((orc_int64)var39.i + (orc_int64)var36.i);
    /* 7: storel */
    ptr0[i] = var37;
  }

}

void
_fractal_outerloop_fp_zb (Eina_F16p16 * ORC_RESTRICT d1, const orc_uint32 * ORC_RESTRICT s1, int p1, int n)
{
  OrcExecutor _ex, *ex = &_ex;
  static volatile int p_inited = 0;
  static OrcCode *c = 0;
  void (*func) (OrcExecutor *);

  if (!p_inited) {
    orc_once_mutex_lock ();
    if (!p_inited) {
      OrcProgram *p;

      p = orc_program_new ();
      orc_program_set_name (p, "_fractal_outerloop_fp_zb");
      orc_program_set_backup_function (p, _backup__fractal_outerloop_fp_zb);
      orc_program_add_destination (p, 4, "d1");
      orc_program_add_source (p, 4, "s1");
      orc_program_add_constant (p, 4, 0xfffffffc, "c1");
      orc_program_add_constant (p, 4, 0x00020000, "c2");
      orc_program_add_parameter (p, 4, "p1");
      orc_program_add_temporary (p, 4, "t1");

      orc_program_append_2 (p, "mulll", 0, ORC_VAR_T1, ORC_VAR_S1, ORC_VAR_C1, ORC_VAR_D1);
      orc_program_append_2 (p, "mulll", 0, ORC_VAR_T1, ORC_VAR_T1, ORC_VAR_P1, ORC_VAR_D1);
      orc_program_append_2 (p, "addssl", 0, ORC_VAR_D1, ORC_VAR_T1, ORC_VAR_C2, ORC_VAR_D1);

      orc_program_compile (p);
      c = orc_program_take_code (p);
      orc_program_free (p);
    }
    p_inited = TRUE;
    orc_once_mutex_unlock ();
  }
  ex->arrays[ORC_VAR_A2] = c;
  ex->program = 0;

  ex->n = n;
  ex->arrays[ORC_VAR_D1] = d1;
  ex->arrays[ORC_VAR_S1] = (void *)s1;
  ex->params[ORC_VAR_P1] = p1;

  func = c->exec;
  func (ex);
}
#endif


/* _fractal_inerloop */
#ifdef DISABLE_ORC
void
_fractal_inerloop (orc_uint8 * ORC_RESTRICT d1, orc_uint8 * ORC_RESTRICT d2, Eina_F16p16 * ORC_RESTRICT d3, Eina_F16p16 * ORC_RESTRICT d4, const orc_uint8 * ORC_RESTRICT s1, const Eina_F16p16 * ORC_RESTRICT s2, const Eina_F16p16 * ORC_RESTRICT s3, int p1, int p2, int n){
  int i;
  orc_int8 * ORC_RESTRICT ptr0;
  orc_int8 * ORC_RESTRICT ptr1;
  orc_union32 * ORC_RESTRICT ptr2;
  orc_union32 * ORC_RESTRICT ptr3;
  const orc_int8 * ORC_RESTRICT ptr4;
  const orc_union32 * ORC_RESTRICT ptr5;
  const orc_union32 * ORC_RESTRICT ptr6;
  orc_union32 var51;
  orc_union32 var52;
  orc_union32 var53;
  orc_union32 var54;
  orc_union32 var55;
  orc_union32 var56;
  orc_union32 var57;
  orc_union32 var58;
  orc_union32 var59;
  orc_union32 var60;
  orc_union32 var61;
  orc_union32 var62;
  orc_union32 var63;
  orc_union32 var64;
  orc_union32 var65;
  orc_union32 var66;
  orc_int8 var67;
  orc_int8 var68;
  orc_int8 var69;
  orc_union32 var70;
  orc_union32 var71;
  orc_union32 var72;
  orc_union32 var73;
  orc_union32 var74;
  orc_union32 var75;
  orc_union32 var76;
  orc_union32 var77;
  orc_union32 var78;
  orc_union32 var79;
  orc_int8 var80;
  orc_union32 var81;
  orc_union32 var82;
  orc_union32 var83;
  orc_union32 var84;
  orc_union32 var85;
  orc_union32 var86;
  orc_union16 var87;
  orc_union16 var88;
  orc_union32 var89;
  orc_union32 var90;
  orc_union32 var91;
  orc_union16 var92;
  orc_union16 var93;
  orc_union32 var94;
  orc_union32 var95;
  orc_union32 var96;
  orc_union32 var97;
  orc_union32 var98;
  orc_union32 var99;
  orc_union32 var100;
  orc_union32 var101;
  orc_union32 var102;
  orc_union32 var103;
  orc_union16 var104;
  orc_union16 var105;
  orc_int8 var106;
  orc_int8 var107;

  ptr0 = (orc_int8 *)d1;
  ptr1 = (orc_int8 *)d2;
  ptr2 = (orc_union32 *)d3;
  ptr3 = (orc_union32 *)d4;
  ptr4 = (orc_int8 *)s1;
  ptr5 = (orc_union32 *)s2;
  ptr6 = (orc_union32 *)s3;

    /* 17: loadpl */
    var59.i = p1;
    /* 30: loadpl */
    var65.i = p2;
    /* 34: loadpb */
    var68 = (int)0x00000001; /* 1 or 4.94066e-324f */
    /* 51: loadpl */
    var78.i = (int)0x00040000; /* 262144 or 1.29516e-318f */
    /* 53: loadpl */
    var79.i = (int)0x00000000; /* 0 or 0f */

  for (i = 0; i < n; i++) {
    /* 0: loadl */
    var51 = ptr5[i];
    /* 1: loadl */
    var52 = ptr5[i];
    /* 2: mulll */
    var81.i = (var51.i * var52.i) & 0xffffffff;
    /* 3: loadl */
    var53 = ptr5[i];
    /* 4: loadl */
    var54 = ptr5[i];
    /* 5: mulhul */
    var82.i = ((orc_uint64)(orc_uint32)var53.i * (orc_uint64)(orc_uint32)var54.i) >> 32;
    /* 6: loadl */
    var55 = ptr6[i];
    /* 7: loadl */
    var56 = ptr6[i];
    /* 8: mulll */
    var83.i = (var55.i * var56.i) & 0xffffffff;
    /* 9: loadl */
    var57 = ptr6[i];
    /* 10: loadl */
    var58 = ptr6[i];
    /* 11: mulhul */
    var84.i = ((orc_uint64)(orc_uint32)var57.i * (orc_uint64)(orc_uint32)var58.i) >> 32;
    /* 12: subl */
    var85.i = var81.i - var83.i;
    /* 13: subl */
    var86.i = var82.i - var84.i;
    /* 14: convhlw */
    var87.i = ((orc_uint32)var85.i)>>16;
    /* 15: convlw */
    var88.i = var86.i;
    /* 16: mergewl */
    {
       orc_union32 _dest;
       _dest.x2[0] = var88.i;
       _dest.x2[1] = var87.i;
       var89.i = _dest.i;
    }
    /* 18: addl */
    var60.i = var89.i + var59.i;
    /* 19: storel */
    ptr2[i] = var60;
    /* 20: loadl */
    var61 = ptr5[i];
    /* 21: loadl */
    var62 = ptr6[i];
    /* 22: mulll */
    var90.i = (var61.i * var62.i) & 0xffffffff;
    /* 23: loadl */
    var63 = ptr5[i];
    /* 24: loadl */
    var64 = ptr6[i];
    /* 25: mulhsl */
    var91.i = ((orc_int64)var63.i * (orc_int64)var64.i) >> 32;
    /* 26: convhlw */
    var92.i = ((orc_uint32)var90.i)>>16;
    /* 27: convssslw */
    var93.i = ORC_CLAMP_SW(var91.i);
    /* 28: mergewl */
    {
       orc_union32 _dest;
       _dest.x2[0] = var93.i;
       _dest.x2[1] = var92.i;
       var94.i = _dest.i;
    }
    /* 29: shll */
    var95.i = var94.i << 1;
    /* 31: addl */
    var66.i = var95.i + var65.i;
    /* 32: storel */
    ptr3[i] = var66;
    /* 33: loadb */
    var67 = ptr4[i];
    /* 35: addb */
    var69 = var67 + var68;
    /* 36: storeb */
    ptr0[i] = var69;
    /* 37: loadl */
    var70 = ptr2[i];
    /* 38: loadl */
    var71 = ptr2[i];
    /* 39: mulll */
    var96.i = (var70.i * var71.i) & 0xffffffff;
    /* 40: loadl */
    var72 = ptr2[i];
    /* 41: loadl */
    var73 = ptr2[i];
    /* 42: mulhul */
    var97.i = ((orc_uint64)(orc_uint32)var72.i * (orc_uint64)(orc_uint32)var73.i) >> 32;
    /* 43: loadl */
    var74 = ptr3[i];
    /* 44: loadl */
    var75 = ptr3[i];
    /* 45: mulll */
    var98.i = (var74.i * var75.i) & 0xffffffff;
    /* 46: loadl */
    var76 = ptr3[i];
    /* 47: loadl */
    var77 = ptr3[i];
    /* 48: mulhul */
    var99.i = ((orc_uint64)(orc_uint32)var76.i * (orc_uint64)(orc_uint32)var77.i) >> 32;
    /* 49: addl */
    var100.i = var96.i + var98.i;
    /* 50: addl */
    var101.i = var97.i + var99.i;
    /* 52: cmpgtsl */
    var102.i = (var100.i > var78.i) ? (~0) : 0;
    /* 54: cmpgtsl */
    var103.i = (var101.i > var79.i) ? (~0) : 0;
    /* 55: convlw */
    var104.i = var102.i;
    /* 56: convlw */
    var105.i = var103.i;
    /* 57: convwb */
    var106 = var104.i;
    /* 58: convwb */
    var107 = var105.i;
    /* 59: orb */
    var80 = var106 | var107;
    /* 60: storeb */
    ptr1[i] = var80;
  }

}

#else
static void
_backup__fractal_inerloop (OrcExecutor * ORC_RESTRICT ex)
{
  int i;
  int n = ex->n;
  orc_int8 * ORC_RESTRICT ptr0;
  orc_int8 * ORC_RESTRICT ptr1;
  orc_union32 * ORC_RESTRICT ptr2;
  orc_union32 * ORC_RESTRICT ptr3;
  const orc_int8 * ORC_RESTRICT ptr4;
  const orc_union32 * ORC_RESTRICT ptr5;
  const orc_union32 * ORC_RESTRICT ptr6;
  orc_union32 var51;
  orc_union32 var52;
  orc_union32 var53;
  orc_union32 var54;
  orc_union32 var55;
  orc_union32 var56;
  orc_union32 var57;
  orc_union32 var58;
  orc_union32 var59;
  orc_union32 var60;
  orc_union32 var61;
  orc_union32 var62;
  orc_union32 var63;
  orc_union32 var64;
  orc_union32 var65;
  orc_union32 var66;
  orc_int8 var67;
  orc_int8 var68;
  orc_int8 var69;
  orc_union32 var70;
  orc_union32 var71;
  orc_union32 var72;
  orc_union32 var73;
  orc_union32 var74;
  orc_union32 var75;
  orc_union32 var76;
  orc_union32 var77;
  orc_union32 var78;
  orc_union32 var79;
  orc_int8 var80;
  orc_union32 var81;
  orc_union32 var82;
  orc_union32 var83;
  orc_union32 var84;
  orc_union32 var85;
  orc_union32 var86;
  orc_union16 var87;
  orc_union16 var88;
  orc_union32 var89;
  orc_union32 var90;
  orc_union32 var91;
  orc_union16 var92;
  orc_union16 var93;
  orc_union32 var94;
  orc_union32 var95;
  orc_union32 var96;
  orc_union32 var97;
  orc_union32 var98;
  orc_union32 var99;
  orc_union32 var100;
  orc_union32 var101;
  orc_union32 var102;
  orc_union32 var103;
  orc_union16 var104;
  orc_union16 var105;
  orc_int8 var106;
  orc_int8 var107;

  ptr0 = (orc_int8 *)ex->arrays[0];
  ptr1 = (orc_int8 *)ex->arrays[1];
  ptr2 = (orc_union32 *)ex->arrays[2];
  ptr3 = (orc_union32 *)ex->arrays[3];
  ptr4 = (orc_int8 *)ex->arrays[4];
  ptr5 = (orc_union32 *)ex->arrays[5];
  ptr6 = (orc_union32 *)ex->arrays[6];

    /* 17: loadpl */
    var59.i = ex->params[24];
    /* 30: loadpl */
    var65.i = ex->params[25];
    /* 34: loadpb */
    var68 = (int)0x00000001; /* 1 or 4.94066e-324f */
    /* 51: loadpl */
    var78.i = (int)0x00040000; /* 262144 or 1.29516e-318f */
    /* 53: loadpl */
    var79.i = (int)0x00000000; /* 0 or 0f */

  for (i = 0; i < n; i++) {
    /* 0: loadl */
    var51 = ptr5[i];
    /* 1: loadl */
    var52 = ptr5[i];
    /* 2: mulll */
    var81.i = (var51.i * var52.i) & 0xffffffff;
    /* 3: loadl */
    var53 = ptr5[i];
    /* 4: loadl */
    var54 = ptr5[i];
    /* 5: mulhul */
    var82.i = ((orc_uint64)(orc_uint32)var53.i * (orc_uint64)(orc_uint32)var54.i) >> 32;
    /* 6: loadl */
    var55 = ptr6[i];
    /* 7: loadl */
    var56 = ptr6[i];
    /* 8: mulll */
    var83.i = (var55.i * var56.i) & 0xffffffff;
    /* 9: loadl */
    var57 = ptr6[i];
    /* 10: loadl */
    var58 = ptr6[i];
    /* 11: mulhul */
    var84.i = ((orc_uint64)(orc_uint32)var57.i * (orc_uint64)(orc_uint32)var58.i) >> 32;
    /* 12: subl */
    var85.i = var81.i - var83.i;
    /* 13: subl */
    var86.i = var82.i - var84.i;
    /* 14: convhlw */
    var87.i = ((orc_uint32)var85.i)>>16;
    /* 15: convlw */
    var88.i = var86.i;
    /* 16: mergewl */
    {
       orc_union32 _dest;
       _dest.x2[0] = var88.i;
       _dest.x2[1] = var87.i;
       var89.i = _dest.i;
    }
    /* 18: addl */
    var60.i = var89.i + var59.i;
    /* 19: storel */
    ptr2[i] = var60;
    /* 20: loadl */
    var61 = ptr5[i];
    /* 21: loadl */
    var62 = ptr6[i];
    /* 22: mulll */
    var90.i = (var61.i * var62.i) & 0xffffffff;
    /* 23: loadl */
    var63 = ptr5[i];
    /* 24: loadl */
    var64 = ptr6[i];
    /* 25: mulhsl */
    var91.i = ((orc_int64)var63.i * (orc_int64)var64.i) >> 32;
    /* 26: convhlw */
    var92.i = ((orc_uint32)var90.i)>>16;
    /* 27: convssslw */
    var93.i = ORC_CLAMP_SW(var91.i);
    /* 28: mergewl */
    {
       orc_union32 _dest;
       _dest.x2[0] = var93.i;
       _dest.x2[1] = var92.i;
       var94.i = _dest.i;
    }
    /* 29: shll */
    var95.i = var94.i << 1;
    /* 31: addl */
    var66.i = var95.i + var65.i;
    /* 32: storel */
    ptr3[i] = var66;
    /* 33: loadb */
    var67 = ptr4[i];
    /* 35: addb */
    var69 = var67 + var68;
    /* 36: storeb */
    ptr0[i] = var69;
    /* 37: loadl */
    var70 = ptr2[i];
    /* 38: loadl */
    var71 = ptr2[i];
    /* 39: mulll */
    var96.i = (var70.i * var71.i) & 0xffffffff;
    /* 40: loadl */
    var72 = ptr2[i];
    /* 41: loadl */
    var73 = ptr2[i];
    /* 42: mulhul */
    var97.i = ((orc_uint64)(orc_uint32)var72.i * (orc_uint64)(orc_uint32)var73.i) >> 32;
    /* 43: loadl */
    var74 = ptr3[i];
    /* 44: loadl */
    var75 = ptr3[i];
    /* 45: mulll */
    var98.i = (var74.i * var75.i) & 0xffffffff;
    /* 46: loadl */
    var76 = ptr3[i];
    /* 47: loadl */
    var77 = ptr3[i];
    /* 48: mulhul */
    var99.i = ((orc_uint64)(orc_uint32)var76.i * (orc_uint64)(orc_uint32)var77.i) >> 32;
    /* 49: addl */
    var100.i = var96.i + var98.i;
    /* 50: addl */
    var101.i = var97.i + var99.i;
    /* 52: cmpgtsl */
    var102.i = (var100.i > var78.i) ? (~0) : 0;
    /* 54: cmpgtsl */
    var103.i = (var101.i > var79.i) ? (~0) : 0;
    /* 55: convlw */
    var104.i = var102.i;
    /* 56: convlw */
    var105.i = var103.i;
    /* 57: convwb */
    var106 = var104.i;
    /* 58: convwb */
    var107 = var105.i;
    /* 59: orb */
    var80 = var106 | var107;
    /* 60: storeb */
    ptr1[i] = var80;
  }

}

void
_fractal_inerloop (orc_uint8 * ORC_RESTRICT d1, orc_uint8 * ORC_RESTRICT d2, Eina_F16p16 * ORC_RESTRICT d3, Eina_F16p16 * ORC_RESTRICT d4, const orc_uint8 * ORC_RESTRICT s1, const Eina_F16p16 * ORC_RESTRICT s2, const Eina_F16p16 * ORC_RESTRICT s3, int p1, int p2, int n)
{
  OrcExecutor _ex, *ex = &_ex;
  static volatile int p_inited = 0;
  static OrcCode *c = 0;
  void (*func) (OrcExecutor *);

  if (!p_inited) {
    orc_once_mutex_lock ();
    if (!p_inited) {
      OrcProgram *p;

      p = orc_program_new ();
      orc_program_set_name (p, "_fractal_inerloop");
      orc_program_set_backup_function (p, _backup__fractal_inerloop);
      orc_program_add_destination (p, 1, "d1");
      orc_program_add_destination (p, 1, "d2");
      orc_program_add_destination (p, 4, "d3");
      orc_program_add_destination (p, 4, "d4");
      orc_program_add_source (p, 1, "s1");
      orc_program_add_source (p, 4, "s2");
      orc_program_add_source (p, 4, "s3");
      orc_program_add_constant (p, 4, 0x00000001, "c1");
      orc_program_add_constant (p, 4, 0x00040000, "c2");
      orc_program_add_constant (p, 4, 0x00000000, "c3");
      orc_program_add_parameter (p, 4, "p1");
      orc_program_add_parameter (p, 4, "p2");
      orc_program_add_temporary (p, 4, "t1");
      orc_program_add_temporary (p, 4, "t2");
      orc_program_add_temporary (p, 4, "t3");
      orc_program_add_temporary (p, 4, "t4");
      orc_program_add_temporary (p, 4, "t5");
      orc_program_add_temporary (p, 4, "t6");
      orc_program_add_temporary (p, 4, "t7");
      orc_program_add_temporary (p, 4, "t8");
      orc_program_add_temporary (p, 4, "t9");
      orc_program_add_temporary (p, 2, "t10");
      orc_program_add_temporary (p, 2, "t11");
      orc_program_add_temporary (p, 4, "t12");
      orc_program_add_temporary (p, 4, "t13");
      orc_program_add_temporary (p, 4, "t14");
      orc_program_add_temporary (p, 4, "t15");
      orc_program_add_temporary (p, 4, "t16");

      orc_program_append_2 (p, "mulll", 0, ORC_VAR_T1, ORC_VAR_S2, ORC_VAR_S2, ORC_VAR_D1);
      orc_program_append_2 (p, "mulhul", 0, ORC_VAR_T2, ORC_VAR_S2, ORC_VAR_S2, ORC_VAR_D1);
      orc_program_append_2 (p, "mulll", 0, ORC_VAR_T3, ORC_VAR_S3, ORC_VAR_S3, ORC_VAR_D1);
      orc_program_append_2 (p, "mulhul", 0, ORC_VAR_T4, ORC_VAR_S3, ORC_VAR_S3, ORC_VAR_D1);
      orc_program_append_2 (p, "subl", 0, ORC_VAR_T5, ORC_VAR_T1, ORC_VAR_T3, ORC_VAR_D1);
      orc_program_append_2 (p, "subl", 0, ORC_VAR_T6, ORC_VAR_T2, ORC_VAR_T4, ORC_VAR_D1);
      orc_program_append_2 (p, "convhlw", 0, ORC_VAR_T10, ORC_VAR_T5, ORC_VAR_D1, ORC_VAR_D1);
      orc_program_append_2 (p, "convlw", 0, ORC_VAR_T11, ORC_VAR_T6, ORC_VAR_D1, ORC_VAR_D1);
      orc_program_append_2 (p, "mergewl", 0, ORC_VAR_T7, ORC_VAR_T11, ORC_VAR_T10, ORC_VAR_D1);
      orc_program_append_2 (p, "addl", 0, ORC_VAR_D3, ORC_VAR_T7, ORC_VAR_P1, ORC_VAR_D1);
      orc_program_append_2 (p, "mulll", 0, ORC_VAR_T12, ORC_VAR_S2, ORC_VAR_S3, ORC_VAR_D1);
      orc_program_append_2 (p, "mulhsl", 0, ORC_VAR_T13, ORC_VAR_S2, ORC_VAR_S3, ORC_VAR_D1);
      orc_program_append_2 (p, "convhlw", 0, ORC_VAR_T10, ORC_VAR_T12, ORC_VAR_D1, ORC_VAR_D1);
      orc_program_append_2 (p, "convssslw", 0, ORC_VAR_T11, ORC_VAR_T13, ORC_VAR_D1, ORC_VAR_D1);
      orc_program_append_2 (p, "mergewl", 0, ORC_VAR_T8, ORC_VAR_T11, ORC_VAR_T10, ORC_VAR_D1);
      orc_program_append_2 (p, "shll", 0, ORC_VAR_T9, ORC_VAR_T8, ORC_VAR_C1, ORC_VAR_D1);
      orc_program_append_2 (p, "addl", 0, ORC_VAR_D4, ORC_VAR_T9, ORC_VAR_P2, ORC_VAR_D1);
      orc_program_append_2 (p, "addb", 0, ORC_VAR_D1, ORC_VAR_S1, ORC_VAR_C1, ORC_VAR_D1);
      orc_program_append_2 (p, "mulll", 0, ORC_VAR_T1, ORC_VAR_D3, ORC_VAR_D3, ORC_VAR_D1);
      orc_program_append_2 (p, "mulhul", 0, ORC_VAR_T2, ORC_VAR_D3, ORC_VAR_D3, ORC_VAR_D1);
      orc_program_append_2 (p, "mulll", 0, ORC_VAR_T3, ORC_VAR_D4, ORC_VAR_D4, ORC_VAR_D1);
      orc_program_append_2 (p, "mulhul", 0, ORC_VAR_T4, ORC_VAR_D4, ORC_VAR_D4, ORC_VAR_D1);
      orc_program_append_2 (p, "addl", 0, ORC_VAR_T14, ORC_VAR_T1, ORC_VAR_T3, ORC_VAR_D1);
      orc_program_append_2 (p, "addl", 0, ORC_VAR_T15, ORC_VAR_T2, ORC_VAR_T4, ORC_VAR_D1);
      orc_program_append_2 (p, "cmpgtsl", 0, 47, ORC_VAR_T14, ORC_VAR_C2, ORC_VAR_D1);
      orc_program_append_2 (p, "cmpgtsl", 0, 48, ORC_VAR_T15, ORC_VAR_C3, ORC_VAR_D1);
      orc_program_append_2 (p, "convlw", 0, ORC_VAR_T10, 47, ORC_VAR_D1, ORC_VAR_D1);
      orc_program_append_2 (p, "convlw", 0, ORC_VAR_T11, 48, ORC_VAR_D1, ORC_VAR_D1);
      orc_program_append_2 (p, "convwb", 0, 49, ORC_VAR_T10, ORC_VAR_D1, ORC_VAR_D1);
      orc_program_append_2 (p, "convwb", 0, 50, ORC_VAR_T11, ORC_VAR_D1, ORC_VAR_D1);
      orc_program_append_2 (p, "orb", 0, ORC_VAR_D2, 49, 50, ORC_VAR_D1);

      orc_program_compile (p);
      c = orc_program_take_code (p);
      orc_program_free (p);
    }
    p_inited = TRUE;
    orc_once_mutex_unlock ();
  }
  ex->arrays[ORC_VAR_A2] = c;
  ex->program = 0;

  ex->n = n;
  ex->arrays[ORC_VAR_D1] = d1;
  ex->arrays[ORC_VAR_D2] = d2;
  ex->arrays[ORC_VAR_D3] = d3;
  ex->arrays[ORC_VAR_D4] = d4;
  ex->arrays[ORC_VAR_S1] = (void *)s1;
  ex->arrays[ORC_VAR_S2] = (void *)s2;
  ex->arrays[ORC_VAR_S3] = (void *)s3;
  ex->params[ORC_VAR_P1] = p1;
  ex->params[ORC_VAR_P2] = p2;

  func = c->exec;
  func (ex);
}
#endif


/* color_conversion */
#ifdef DISABLE_ORC
void
color_conversion (orc_uint32 * ORC_RESTRICT d1, const orc_uint8 * ORC_RESTRICT s1, int p1, int p2, int n){
  int i;
  orc_union32 * ORC_RESTRICT ptr0;
  const orc_int8 * ORC_RESTRICT ptr4;
  orc_int8 var46;
  orc_union32 var47;
  orc_union32 var48;
  orc_union32 var49;
  orc_union32 var50;
  orc_union32 var51;
  orc_int8 var52;
  orc_union32 var53;
  orc_union16 var54;
  orc_union32 var55;
  orc_union32 var56;
  orc_union32 var57;
  orc_union16 var58;
  orc_int8 var59;
  orc_union32 var60;
  orc_union32 var61;
  orc_union16 var62;
  orc_int8 var63;
  orc_union32 var64;
  orc_union16 var65;
  orc_int8 var66;
  orc_union16 var67;
  orc_union16 var68;

  ptr0 = (orc_union32 *)d1;
  ptr4 = (orc_int8 *)s1;

    /* 3: loadpl */
    var47.i = p1;
    /* 5: loadpl */
    var48.i = (int)0x00800000; /* 8388608 or 4.14452e-317f */
    /* 9: loadpl */
    var49.i = p2;
    /* 11: loadpl */
    var50.i = (int)0x00ae0000; /* 11403264 or 5.63396e-317f */
    /* 15: loadpl */
    var51.i = (int)0x00800000; /* 8388608 or 4.14452e-317f */
    /* 20: loadpb */
    var52 = (int)0x000000ff; /* 255 or 1.25987e-321f */

  for (i = 0; i < n; i++) {
    /* 0: loadb */
    var46 = ptr4[i];
    /* 1: convubw */
    var54.i = (orc_uint8)var46;
    /* 2: convuwl */
    var55.i = (orc_uint16)var54.i;
    /* 4: mulll */
    var56.i = (var47.i * var55.i) & 0xffffffff;
    /* 6: subl */
    var57.i = var48.i - var56.i;
    /* 7: convhlw */
    var58.i = ((orc_uint32)var57.i)>>16;
    /* 8: convwb */
    var59 = var58.i;
    /* 10: mulll */
    var60.i = (var49.i * var55.i) & 0xffffffff;
    /* 12: subl */
    var61.i = var50.i - var60.i;
    /* 13: convhlw */
    var62.i = ((orc_uint32)var61.i)>>16;
    /* 14: convwb */
    var63 = var62.i;
    /* 16: subl */
    var64.i = var51.i - var60.i;
    /* 17: convhlw */
    var65.i = ((orc_uint32)var64.i)>>16;
    /* 18: convwb */
    var66 = var65.i;
    /* 19: mergebw */
    {
       orc_union16 _dest;
       _dest.x2[0] = var59;
       _dest.x2[1] = var63;
       var67.i = _dest.i;
    }
    /* 21: mergebw */
    {
       orc_union16 _dest;
       _dest.x2[0] = var66;
       _dest.x2[1] = var52;
       var68.i = _dest.i;
    }
    /* 22: mergewl */
    {
       orc_union32 _dest;
       _dest.x2[0] = var67.i;
       _dest.x2[1] = var68.i;
       var53.i = _dest.i;
    }
    /* 23: storel */
    ptr0[i] = var53;
  }

}

#else
static void
_backup_color_conversion (OrcExecutor * ORC_RESTRICT ex)
{
  int i;
  int n = ex->n;
  orc_union32 * ORC_RESTRICT ptr0;
  const orc_int8 * ORC_RESTRICT ptr4;
  orc_int8 var46;
  orc_union32 var47;
  orc_union32 var48;
  orc_union32 var49;
  orc_union32 var50;
  orc_union32 var51;
  orc_int8 var52;
  orc_union32 var53;
  orc_union16 var54;
  orc_union32 var55;
  orc_union32 var56;
  orc_union32 var57;
  orc_union16 var58;
  orc_int8 var59;
  orc_union32 var60;
  orc_union32 var61;
  orc_union16 var62;
  orc_int8 var63;
  orc_union32 var64;
  orc_union16 var65;
  orc_int8 var66;
  orc_union16 var67;
  orc_union16 var68;

  ptr0 = (orc_union32 *)ex->arrays[0];
  ptr4 = (orc_int8 *)ex->arrays[4];

    /* 3: loadpl */
    var47.i = ex->params[24];
    /* 5: loadpl */
    var48.i = (int)0x00800000; /* 8388608 or 4.14452e-317f */
    /* 9: loadpl */
    var49.i = ex->params[25];
    /* 11: loadpl */
    var50.i = (int)0x00ae0000; /* 11403264 or 5.63396e-317f */
    /* 15: loadpl */
    var51.i = (int)0x00800000; /* 8388608 or 4.14452e-317f */
    /* 20: loadpb */
    var52 = (int)0x000000ff; /* 255 or 1.25987e-321f */

  for (i = 0; i < n; i++) {
    /* 0: loadb */
    var46 = ptr4[i];
    /* 1: convubw */
    var54.i = (orc_uint8)var46;
    /* 2: convuwl */
    var55.i = (orc_uint16)var54.i;
    /* 4: mulll */
    var56.i = (var47.i * var55.i) & 0xffffffff;
    /* 6: subl */
    var57.i = var48.i - var56.i;
    /* 7: convhlw */
    var58.i = ((orc_uint32)var57.i)>>16;
    /* 8: convwb */
    var59 = var58.i;
    /* 10: mulll */
    var60.i = (var49.i * var55.i) & 0xffffffff;
    /* 12: subl */
    var61.i = var50.i - var60.i;
    /* 13: convhlw */
    var62.i = ((orc_uint32)var61.i)>>16;
    /* 14: convwb */
    var63 = var62.i;
    /* 16: subl */
    var64.i = var51.i - var60.i;
    /* 17: convhlw */
    var65.i = ((orc_uint32)var64.i)>>16;
    /* 18: convwb */
    var66 = var65.i;
    /* 19: mergebw */
    {
       orc_union16 _dest;
       _dest.x2[0] = var59;
       _dest.x2[1] = var63;
       var67.i = _dest.i;
    }
    /* 21: mergebw */
    {
       orc_union16 _dest;
       _dest.x2[0] = var66;
       _dest.x2[1] = var52;
       var68.i = _dest.i;
    }
    /* 22: mergewl */
    {
       orc_union32 _dest;
       _dest.x2[0] = var67.i;
       _dest.x2[1] = var68.i;
       var53.i = _dest.i;
    }
    /* 23: storel */
    ptr0[i] = var53;
  }

}

void
color_conversion (orc_uint32 * ORC_RESTRICT d1, const orc_uint8 * ORC_RESTRICT s1, int p1, int p2, int n)
{
  OrcExecutor _ex, *ex = &_ex;
  static volatile int p_inited = 0;
  static OrcCode *c = 0;
  void (*func) (OrcExecutor *);

  if (!p_inited) {
    orc_once_mutex_lock ();
    if (!p_inited) {
      OrcProgram *p;

      p = orc_program_new ();
      orc_program_set_name (p, "color_conversion");
      orc_program_set_backup_function (p, _backup_color_conversion);
      orc_program_add_destination (p, 4, "d1");
      orc_program_add_source (p, 1, "s1");
      orc_program_add_constant (p, 4, 0x00800000, "c1");
      orc_program_add_constant (p, 4, 0x00ae0000, "c2");
      orc_program_add_constant (p, 4, 0x000000ff, "c3");
      orc_program_add_parameter (p, 4, "p1");
      orc_program_add_parameter (p, 4, "p2");
      orc_program_add_temporary (p, 2, "t1");
      orc_program_add_temporary (p, 4, "t2");
      orc_program_add_temporary (p, 4, "t3");
      orc_program_add_temporary (p, 2, "t4");
      orc_program_add_temporary (p, 1, "t5");
      orc_program_add_temporary (p, 4, "t6");
      orc_program_add_temporary (p, 4, "t7");
      orc_program_add_temporary (p, 2, "t8");
      orc_program_add_temporary (p, 1, "t9");
      orc_program_add_temporary (p, 4, "t10");
      orc_program_add_temporary (p, 2, "t11");
      orc_program_add_temporary (p, 1, "t12");
      orc_program_add_temporary (p, 2, "t13");
      orc_program_add_temporary (p, 2, "t14");

      orc_program_append_2 (p, "convubw", 0, ORC_VAR_T1, ORC_VAR_S1, ORC_VAR_D1, ORC_VAR_D1);
      orc_program_append_2 (p, "convuwl", 0, ORC_VAR_T2, ORC_VAR_T1, ORC_VAR_D1, ORC_VAR_D1);
      orc_program_append_2 (p, "mulll", 0, ORC_VAR_T3, ORC_VAR_P1, ORC_VAR_T2, ORC_VAR_D1);
      orc_program_append_2 (p, "subl", 0, ORC_VAR_T3, ORC_VAR_C1, ORC_VAR_T3, ORC_VAR_D1);
      orc_program_append_2 (p, "convhlw", 0, ORC_VAR_T4, ORC_VAR_T3, ORC_VAR_D1, ORC_VAR_D1);
      orc_program_append_2 (p, "convwb", 0, ORC_VAR_T5, ORC_VAR_T4, ORC_VAR_D1, ORC_VAR_D1);
      orc_program_append_2 (p, "mulll", 0, ORC_VAR_T6, ORC_VAR_P2, ORC_VAR_T2, ORC_VAR_D1);
      orc_program_append_2 (p, "subl", 0, ORC_VAR_T7, ORC_VAR_C2, ORC_VAR_T6, ORC_VAR_D1);
      orc_program_append_2 (p, "convhlw", 0, ORC_VAR_T8, ORC_VAR_T7, ORC_VAR_D1, ORC_VAR_D1);
      orc_program_append_2 (p, "convwb", 0, ORC_VAR_T9, ORC_VAR_T8, ORC_VAR_D1, ORC_VAR_D1);
      orc_program_append_2 (p, "subl", 0, ORC_VAR_T10, ORC_VAR_C1, ORC_VAR_T6, ORC_VAR_D1);
      orc_program_append_2 (p, "convhlw", 0, ORC_VAR_T11, ORC_VAR_T10, ORC_VAR_D1, ORC_VAR_D1);
      orc_program_append_2 (p, "convwb", 0, ORC_VAR_T12, ORC_VAR_T11, ORC_VAR_D1, ORC_VAR_D1);
      orc_program_append_2 (p, "mergebw", 0, ORC_VAR_T13, ORC_VAR_T5, ORC_VAR_T9, ORC_VAR_D1);
      orc_program_append_2 (p, "mergebw", 0, ORC_VAR_T14, ORC_VAR_T12, ORC_VAR_C3, ORC_VAR_D1);
      orc_program_append_2 (p, "mergewl", 0, ORC_VAR_D1, ORC_VAR_T13, ORC_VAR_T14, ORC_VAR_D1);

      orc_program_compile (p);
      c = orc_program_take_code (p);
      orc_program_free (p);
    }
    p_inited = TRUE;
    orc_once_mutex_unlock ();
  }
  ex->arrays[ORC_VAR_A2] = c;
  ex->program = 0;

  ex->n = n;
  ex->arrays[ORC_VAR_D1] = d1;
  ex->arrays[ORC_VAR_S1] = (void *)s1;
  ex->params[ORC_VAR_P1] = p1;
  ex->params[ORC_VAR_P2] = p2;

  func = c->exec;
  func (ex);
}
#endif


