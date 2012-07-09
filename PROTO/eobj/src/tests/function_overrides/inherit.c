#include "Eo.h"
#include "simple.h"

#include "inherit.h"

#define MY_CLASS INHERIT_CLASS

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Inherit",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(NULL, NULL, 0),
     NULL,
     0,
     NULL,
     NULL
};

EO_DEFINE_CLASS(inherit_class_get, &class_desc, SIMPLE_CLASS, NULL);
