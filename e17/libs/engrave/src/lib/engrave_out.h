#ifndef ENGRAVE_OUT_H
#define ENGRAVE_OUT_H

/**
 * @file engrave_out.h Functions to faciliate outputing the Engrave information.
 * @brief Provided the needed functions to output the Engrave information into various formats.
 */

/**
 * @defgroup Engrave_Out Functions needed to output the Engrave data into
 * different file formats.
 *
 * @{
 */

int engrave_eet_output(Engrave_File *engrave_file, const char *path);
int engrave_edc_output(Engrave_File *engrave_file, const char *path);

/**
 * @}
 */

#endif

