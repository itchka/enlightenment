#ifdef E_TYPEDEFS
#else
#ifndef E_SCALE_H
#define E_SCALE_H

EAPI int  e_scale_init(void);
EAPI int  e_scale_shutdown(void);
EAPI void e_scale_update(void);

extern EAPI double e_scale;

#endif
#endif
