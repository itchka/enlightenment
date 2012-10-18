#ifndef _ELOCATION_PRIVATE_H
#define _ELOCATION_PRIVATE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include <Eina.h>
#include <Ecore.h>
#include <EDBus.h>

#ifndef ELOCATION_COLOR_DEFAULT
#define ELOCATION_COLOR_DEFAULT EINA_COLOR_BLUE
#endif
extern int _elocation_log_dom;
#ifdef CRI
#undef CRI
#endif

#ifdef ERR
#undef ERR
#endif
#ifdef INF
#undef INF
#endif
#ifdef WARN
#undef WARN
#endif
#ifdef DBG
#undef DBG
#endif

#define CRI(...)  EINA_LOG_DOM_CRIT(_elocation_log_dom, __VA_ARGS__)
#define DBG(...)  EINA_LOG_DOM_DBG(_elocation_log_dom, __VA_ARGS__)
#define INF(...)  EINA_LOG_DOM_INFO(_elocation_log_dom, __VA_ARGS__)
#define WARN(...) EINA_LOG_DOM_WARN(_elocation_log_dom, __VA_ARGS__)
#define ERR(...)  EINA_LOG_DOM_ERR(_elocation_log_dom, __VA_ARGS__)

/* Provider bus names and object paths. Master is the generic one which should
 * pick up the best one internally based on given requirements. It is also still
 * possible to use providers directly */
#define GEOCLUE_DBUS_NAME "org.freedesktop.Geoclue.Master"
#define GEOCLUE_OBJECT_PATH "/org/freedesktop/Geoclue/Master"
#define GSMLOC_DBUS_NAME "org.freedesktop.Geoclue.Providers.Gsmloc"
#define GSMLOC_OBJECT_PATH "/org/freedesktop/Geoclue/Providers/Gsmloc"
#define HOSTIP_DBUS_NAME "org.freedesktop.Geoclue.Providers.Hostip"
#define HOSTIP_OBJECT_PATH "/org/freedesktop/Geoclue/Providers/Hostip"
#define SKYHOOK_DBUS_NAME "org.freedesktop.Geoclue.Providers.Skyhook"
#define SKYHOOK_OBJECT_PATH "/org/freedesktop/Geoclue/Providers/Skyhook"
#define UBUNTU_DBUS_NAME "org.freedesktop.Geoclue.Providers.UbuntuGeoIP"
#define UBUNTU_OBJECT_PATH "/org/freedesktop/Geoclue/Providers/UbuntuGeoIP"
#define GEONAMES_DBUS_NAME "org.freedesktop.Geoclue.Providers.Geonames"
#define GEONAMES_OBJECT_PATH "/org/freedesktop/Geoclue/Providers/Geonames"
#define PLAZES_DBUS_NAME "org.freedesktop.Geoclue.Providers.Plazes"
#define PLAZES_OBJECT_PATH "/org/freedesktop/Geoclue/Providers/Plazes"
#define YAHOO_DBUS_NAME "org.freedesktop.Geoclue.Providers.Yahoo"
#define YAHOO_OBJECT_PATH "/org/freedesktop/Geoclue/Providers/Yahoo"

/* Master interfaces to control geoclue */
#define GEOCLUE_MASTER_IFACE "org.freedesktop.Geoclue.Master"
#define GEOCLUE_MASTERCLIENT_IFACE "org.freedesktop.Geoclue.MasterClient"

/* Provider interfaces to access location data*/
#define GEOCLUE_GEOCLUE_IFACE "org.freedesktop.Geoclue"
#define GEOCLUE_POSITION_IFACE "org.freedesktop.Geoclue.Position"
#define GEOCLUE_ADDRESS_IFACE "org.freedesktop.Geoclue.Address"
#define GEOCLUE_VELOCITY_IFACE "org.freedesktop.Geoclue.Velocity"
#define GEOCLUE_GEOCODE_IFACE "org.freedesktop.Geoclue.Geocode"
#define GEOCLUE_REVERSEGEOCODE_IFACE "org.freedesktop.Geoclue.ReverseGeocode"

#define GEOCLUE_ADDRESS_KEY_AREA "area"
#define GEOCLUE_ADDRESS_KEY_COUNTRY "country"
#define GEOCLUE_ADDRESS_KEY_COUNTRYCODE "countrycode"
#define GEOCLUE_ADDRESS_KEY_LOCALITY "locality"
#define GEOCLUE_ADDRESS_KEY_POSTALCODE "postalcode"
#define GEOCLUE_ADDRESS_KEY_REGION "region"
#define GEOCLUE_ADDRESS_KEY_STREET "street"

extern int ELOCATION_EVENT_IN;
extern int ELOCATION_EVENT_OUT;

typedef enum {
   GEOCLUE_POSITION_FIELDS_NONE = 0,
   GEOCLUE_POSITION_FIELDS_LATITUDE = 1 << 0,
   GEOCLUE_POSITION_FIELDS_LONGITUDE = 1 << 1,
   GEOCLUE_POSITION_FIELDS_ALTITUDE = 1 << 2
} GeocluePositionFields;

typedef enum {
   GEOCLUE_CONNECTIVITY_UNKNOWN,
   GEOCLUE_CONNECTIVITY_OFFLINE,
   GEOCLUE_CONNECTIVITY_ACQUIRING,
   GEOCLUE_CONNECTIVITY_ONLINE,
} GeoclueNetworkStatus;

typedef enum {
   GEOCLUE_STATUS_ERROR,
   GEOCLUE_STATUS_UNAVAILABLE,
   GEOCLUE_STATUS_ACQUIRING,
   GEOCLUE_STATUS_AVAILABLE
} GeoclueStatus;

typedef enum {
   GEOCLUE_VELOCITY_FIELDS_NONE = 0,
   GEOCLUE_VELOCITY_FIELDS_SPEED = 1 << 0,
   GEOCLUE_VELOCITY_FIELDS_DIRECTION = 1 << 1,
   GEOCLUE_VELOCITY_FIELDS_CLIMB = 1 << 2
} GeoclueVelocityFields;

typedef struct _Elocation_Provider
{
   char *name;
   char *description;
   char *service;
   char *path;
   GeoclueStatus status;
} Elocation_Provider;

#endif

