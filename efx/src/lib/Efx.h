#ifndef EFX_H
#define EFX_H

#include <Evas.h>
#include <Ecore.h>

#ifdef EAPI
# undef EAPI
#endif /* ifdef EAPI */

#ifdef _WIN32
# ifdef EFL_EFX_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else /* ifdef DLL_EXPORT */
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else /* ifdef EFL_BUILD */
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_BUILD */
#else /* ifdef _WIN32 */
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else /* if __GNUC__ >= 4 */
#   define EAPI
#  endif /* if __GNUC__ >= 4 */
# else /* ifdef __GNUC__ */
#  define EAPI
# endif /* ifdef __GNUC__ */
#endif /* ! _WIN32 */

#ifdef __cplusplus
extern "C" {
#endif /* ifdef __cplusplus */

/**
 * @mainpage Efx Library Documentation
 *
 * @version 1.7.99
 * @date 2012
 *
 * Efx is the effects libraries.
 *
 * For a better reference, check the following groups:
 * @li @ref Efx
 * @li @ref Efx_Queue
 * @li @ref Efx_Follow
 * @li @ref Efx_Fade
 * @li @ref Efx_Rotation
 * @li @ref Efx_Move
 * @li @ref Efx_Resize
 *
 * Please see the @ref authors page for contact details.
 */

/**
 * @page authors Authors
 *
 * @author Mike Blumenkrantz <michael.blumenkrantz@gmail.com>
 * @author Vincent "caro" Torri  <vtorri at univ-evry dot fr>
 *
 * Please contact <enlightenment-devel@lists.sourceforge.net> to get in
 * contact with the developers and maintainers.
 */

/**
 * @file
 * @brief These routines are used for Efx library interaction.
 */

/**
 * @defgroup Efx General types and functions.
 *
 * @{
 */

/**
 * Helper macro for simplifying specifying coordinate points as parameters
 * @param X The x coordinate
 * @param Y The y coordinate
 * @ingroup Efx
 */
#define EFX_POINT(X, Y) \
  &(Evas_Point){(X), (Y)}

/**
 * @typedef Efx_Map_Data
 * @ingroup Efx
 */
typedef struct Efx_Map_Data Efx_Map_Data;

/**
 * @struct Efx_Map_Data
 *
 * This struct is provided to callbacks upon the completion of certain types
 * of effects.
 * It contains information about the current position of an object.
 *
 * @ingroup Efx
 */
struct Efx_Map_Data
{
   double rotation; /**< The current rotation (in degrees) of the object */
   Evas_Point *rotate_center; /**< The current rotation center for the object */
   double zoom; /**< The current zoom amount of an object */
   Evas_Point *zoom_center; /**< The current zoom center for the object */
   Evas_Point *move_center; /**< The current move vertex for the object */
   Evas_Point pan; /**< The current pan for the object */
};

/**
 * @typedef Efx_End_Cb
 *
 * This is the callback type used to notify a user about the end of an effect.
 * It is called instantly upon an effect terminating, but only if the effect
 * has run to its full duration. Ending callbacks are NOT called upon
 * stopping/resetting an effect.
 *
 * @param data The data passed when starting the effect
 * @param e The current map data for an object
 * @param obj The object
 *
 * @ingroup Efx
 */
typedef void (*Efx_End_Cb)(void *data, Efx_Map_Data *e, Evas_Object *obj);

/**
 * @enum _Efx_Effect_Speed
 * @typedef Efx_Effect_Speed
 *
 * These values are used to set the speed at which an effect will occur.
 * More information can be found by reading about Ecore_Animator objects.
 *
 * @ingroup Efx
 */
typedef enum _Efx_Effect_Speed
{
   EFX_EFFECT_SPEED_LINEAR = ECORE_POS_MAP_LINEAR,
   EFX_EFFECT_SPEED_ACCELERATE = ECORE_POS_MAP_ACCELERATE,
   EFX_EFFECT_SPEED_DECELERATE = ECORE_POS_MAP_DECELERATE,
   EFX_EFFECT_SPEED_SINUSOIDAL = ECORE_POS_MAP_SINUSOIDAL
} Efx_Effect_Speed;

/**
 * @brief
 * Initialize the library
 *
 * This function must be called before any other efx functions.
 * @return The number of times the library has been initialized,
 * or @c 0 on failure.
 *
 * @see efx_shutdown()
 *
 * @ingroup Efx
 */
EAPI int efx_init(void);

/**
 * @brief
 * Uninitialize the library
 *
 * This function unregisters the log domain and performs other cleanup
 * routines.
 *
 * @see efx_init()
 *
 * @ingroup Efx
 */
EAPI void efx_shutdown(void);

/**
 * @brief
 * Attempt to automatically move+resize an object according to its map
 *
 * This function attempts to move an object to its mapped position and resize
 * it according to its mapped zoom. It will, after calculating, reset the zoom
 * and rotat effects on the object,
 * preserving only its orientation. Ideally, you will not notice any visible
 * change after running this function,
 * but it should be used carefully, as successive effects on a realized
 * object will likely not behave as intended.
 *
 * @param obj The object on which to realize effects
 *
 * @ingroup Efx
 */
EAPI void efx_realize(Evas_Object *obj);

/**
 * @brief
 * Add bumpmapping on an object at a point
 *
 * This function will generate a bumpmap effect on @p obj at the coordinates
 * given.
 * @note The map is generated and displayed immediately upon calling this
 * function
 * @warning The calculations for this function are slow, and should not be used
 * for realtime bumpmap generation; see the bumpmap test for details.
 *
 * @param obj The object to generate the map for
 * @param x The X coordinate to center the map on
 * @param y The Y coordinate to center the map on
 * @return @c EINA_TRUE on successful map generation, else @c EINA_FALSE
 *
 * @ingroup Efx
 */
EAPI Eina_Bool efx_bumpmap(Evas_Object *obj, Evas_Coord x, Evas_Coord y);

/**
 * @}
 */

/**
 * @typedef Efx_Color
 *
 * Handle to color used in effects.
 *
 * @ingroup Efx_Fade
 */
typedef struct Efx_Color Efx_Color;

/**
 * @struct Efx_Color
 *
 * This struct contains RGB data for setting colors in effects.
 *
 * @ingroup Efx_Fade
 */
struct Efx_Color
{
   unsigned char r; /**< Red */
   unsigned char g; /**< Green */
   unsigned char b; /**< Blue */
};


/**
 * @defgroup Efx_Queue Efx Effects Queue
 * @ingroup Efx
 *
 * @{
 *
 * With Efx it's possible to queue many effects with @ref efx_queue_append()
 * and @ref efx_queue_prepend() and run them all with @ref efx_queue_run(),
 * making complex animations much easier to be acomplished.
 */

/**
 * @typedef Efx_Queued_Effect
 *
 * Effect handle, used to create and queue an effect.
 *
 * @ingroup Efx_Queue
 */
typedef struct Efx_Queued_Effect Efx_Queued_Effect;

/**
 * @typedef Efx_Queue_Data
 *
 * Queue data handle, used to specify a queued effect.
 *
 * @ingroup Efx_Queue
 */
typedef struct Efx_Queue_Data Efx_Queue_Data;

/**
 * Helper macro to perform a C99 cast which simplifies the queue
 * effect parameter in queue-related functions.
 * @param EFFECT The effect
 * @ingroup Efx_Queue
 */
#define EFX_QUEUED_EFFECT(EFFECT) \
  &(Efx_Queued_Effect){EFFECT}

/**
 * @enum _Efx_Effect_Type
 * @typedef Efx_Effect_Type
 *
 * These values are used to set the speed at which an effect will occur.
 * More information can be found by reading about Ecore_Animator objects.
 *
 * @ingroup Efx_Queue
 */
typedef enum _Efx_Effect_Type
{
   EFX_EFFECT_TYPE_ROTATE,
   EFX_EFFECT_TYPE_ZOOM,
   EFX_EFFECT_TYPE_MOVE,
   EFX_EFFECT_TYPE_PAN,
   EFX_EFFECT_TYPE_FADE,
   EFX_EFFECT_TYPE_RESIZE
} Efx_Effect_Type;

/**
 * @struct Efx_Queued_Effect
 *
 * This struct contains all the data necessary to create and queue
 * an effect.
 *
 * @ingroup Efx_Queue
 */
struct Efx_Queued_Effect
{
   Efx_Effect_Type type;
   union
     {
        struct
        {
           double degrees;
           Evas_Point *center;
        } rotation;
        struct
        {
           double start;
           double end;
           Evas_Point *center;
        } zoom;
        struct
        {
           Evas_Point point;
        } movement;
        struct
        {
           Efx_Color color;
           unsigned char alpha;
        } fade;
        struct
        {
           Evas_Point *point;
           int w, h;
        } resize;
     } effect;
};

/**
 * @brief
 * Begin processing the queue
 *
 * Call this function after queuing some effects to start the animation
 * sequence.
 *
 * @param obj The object to process the queue for
 *
 * @ingroup Efx_Queue
 */
EAPI void efx_queue_run(Evas_Object *obj);

/**
 * @brief
 * Append an effect to the end of the queue
 *
 * Use this function to add a new effect to the end of the queue.
 * The queue can be empty when using this function.
 *
 * @param obj The object to queue an effect for
 * @param speed The effect speed
 * @param effect The parameters of the desired effect
 * @param total_time The time the effect should occur over
 * @param cb The optional callback to call after this particular effect has
 * completed
 * @param data The data to pass to the callback
 * @return The queued effect, or @c NULL on failure
 *
 * @see efx_queue_prepend()
 * @see efx_queue_delete()
 *
 * @ingroup Efx_Queue
 */
EAPI Efx_Queue_Data *efx_queue_append(Evas_Object *obj, Efx_Effect_Speed speed, const Efx_Queued_Effect *effect, double total_time, Efx_End_Cb cb, const void *data);

/**
 * @brief
 * Add an effect to the start of the queue
 *
 * Use this function to add a new effect to the start of the queue.
 * The queue can be empty when using this function.
 *
 * @note If a queued effect is currently executing, the new effect will be
 * added after it
 *
 * @param obj The object to queue an effect for
 * @param speed The effect speed
 * @param effect The parameters of the desired effect
 * @param total_time The time the effect should occur over
 * @param cb The optional callback to call after this particular effect has
 * completed
 * @param data The data to pass to the callback
 * @return The queued effect, or @c NULL on failure
 *
 * @see efx_queue_append()
 * @see efx_queue_delete()
 *
 * @ingroup Efx_Queue
 */
EAPI Efx_Queue_Data *efx_queue_prepend(Evas_Object *obj, Efx_Effect_Speed speed, const Efx_Queued_Effect *effect, double total_time, Efx_End_Cb cb, const void *data);

/**
 * @brief
 * Promote an inactive effect to the start of the queue
 *
 * Use this function on an effect that is not currently executing to
 * move it to the head of the queue.
 * @note If another effect is currently executing, @p eqd will be moved
 * directly after it.
 * @param obj The object owning the effect
 * @param eqd The effect
 *
 * @ingroup Efx_Queue
 */
EAPI void efx_queue_promote(Evas_Object *obj, Efx_Queue_Data *eqd);

/**
 * @brief
 * Demote an inactive effect to the end of the queue
 *
 * Use this function on an effect that is not currently executing to
 * move it to the tail of the queue.
 *
 * @param obj The object owning the effect
 * @param eqd The effect
 *
 * @ingroup Efx_Queue
 */
EAPI void efx_queue_demote(Evas_Object *obj, Efx_Queue_Data *eqd);

/**
 * @brief
 * Delete an effect from the queue
 *
 * This function will delete and unqueue a previously queued effect.
 * @note Currently executing effects cannot be deleted; they must be manually
 * stopped using the proper stop/reset function.
 * @param obj The object owning the effect
 * @param eqd The effect
 *
 * @ingroup Efx_Queue
 */
EAPI void efx_queue_delete(Evas_Object *obj, Efx_Queue_Data *eqd);

/**
 * @brief
 * Delete all queued effects which are not currently executing
 *
 * This function will delete and unqueue all previously queued effects.
 *
 * @note Currently executing effects cannot be deleted; they must be manually
 * stopped using the proper stop/reset function.
 *
 * @param obj The object owning the effects
 *
 * @ingroup Efx_Queue
 */
EAPI void efx_queue_clear(Evas_Object *obj);

/**
 * @brief
 * Attach an effect to other.
 *
 * Append @p effect to subeffects list of @p eqd.
 *
 * @param eqd The effect
 * @param speed The effect speed
 * @param effect The parameters of the desired effect
 * @param total_time The time the effect should occur over
 * @param cb The optional callback to call after this particular effect has
 * completed
 * @param data The data to pass to the callback
 * @return @c EINA_TRUE on success or @c EINA_FALSE on error.
 *
 * @ingroup Efx_Queue
 */
EAPI Eina_Bool efx_queue_effect_attach(Efx_Queue_Data *eqd, Efx_Effect_Speed speed, const Efx_Queued_Effect *effect, double total_time, Efx_End_Cb cb, const void *data);

/**
 * @}
 */

/**
 * @defgroup Efx_Follow Efx Follow
 * @ingroup Efx
 *
 * @{
 *
 * Using @ref efx_follow() it's possible to cause objects to mimic others.
 */

/**
 * @brief
 * Cause one object to mimic the actions of another
 *
 * Using this function, @p follower will copy every effect set on @p obj
 * until @ref efx_unfollow() is called on @p follower. Note that passing
 * a "follower" object as @p obj will cause the object passed as @p follower
 * to be chained to the follower's top-most "owner" object.
 *
 * @note Effects from an "owner" object will supercede any similar effects
 * explicitly set on a "follower" object, and they will also probably
 * break each other. Don't chain owners to other owners unless you know
 * what you are doing.
 *
 * @param obj The object to follow the actions of
 * @param follower The object to do the following
 * @return @c EINA_TRUE on success, else @c EINA_FALSE
 *
 * @ingroup Efx_Follow
 */
EAPI Eina_Bool efx_follow(Evas_Object *obj, Evas_Object *follower);

/**
 * @brief
 * Unchain a following object from its owner
 *
 * This function will cause @p obj to stop following its owner object.
 * It takes effect immediately and cannot fail.
 *
 * @param obj The follower object
 *
 * @ingroup Efx_Follow
 */
EAPI void efx_unfollow(Evas_Object *obj);

/**
 * @brief
 * Retrieve the list of following objects
 *
 * This function returns a copy of the list of objects following @p obj.
 * The returned list must be manually freed with eina_list_free.
 *
 * @param obj The owner object
 * @return A list of follower Evas_Objects, or @c NULL
 *
 * @ingroup Efx_Follow
 */
EAPI Eina_List *efx_followers_get(Evas_Object *obj);

/**
 * @brief
 * Retrieve an object's leader object
 *
 * Use this function to return the object which @p obj is currently following.
 *
 * @param obj The object
 * @return The leader object or NULL on failure
 *
 * @ingroup Efx_Follow
 */
EAPI Evas_Object *efx_leader_get(Evas_Object *obj);

/**
 * @}
 */

/**
 * @defgroup Efx_Fade Efx Fade Effect
 * @ingroup Efx
 *
 * @{
 *
 * Fade effects are very frequently on elaborated interfaces. Using
 * @ref efx_fade() it's possible to change color and alpha channel leading
 * to fade in / out transitions.
 */

/**
 * Helper macro to simplify specifying a fade effect for queue
 * @param R Red
 * @param G Green
 * @param B Blue
 * @param A Alpha
 * @ingroup Efx_Fade
 */
#define EFX_EFFECT_FADE(R, G, B, A) \
  .type = EFX_EFFECT_TYPE_FADE, .effect.fade = { .color = { .r = (R), .g = (G), .b = (B) }, .alpha = (A) }

/**
 * @brief
 * Commence a fade effect
 *
 * Fade is an effect which allows the changing of color tint and alpha through
 * the use of a clip set on an object. With this effect it is possible to
 * have an object fade in/out or become tinted with another color.
 *
 * @note Black objects will only have their alpha values affected.
 *
 * @param obj The object to use for fading
 * @param speed The speed to fade at
 * @param ec The color to fade to
 * @param alpha The alpha to fade to
 * @param total_time The time the effect should occur over
 * @param cb The optional callback to call when the effect completes
 * @param data Optional data to pass to @p cb
 * @return @c EINA_TRUE on successful queue of the animation, else
 * @c EINA_FALSE
 *
 * @ingroup Efx_Fade
 */
EAPI Eina_Bool efx_fade(Evas_Object *obj, Efx_Effect_Speed speed, Efx_Color *ec, unsigned char alpha, double total_time, Efx_End_Cb cb, const void *data);

/**
 * @brief
 * Stop fading of an object and remove the clip
 *
 * Use this function to immediately stop the fade of an object and restore
 * its original color/alpha.
 *
 * @param obj An object
 *
 * @ingroup Efx_Fade
 */
EAPI void efx_fade_reset(Evas_Object *obj);

/**
 * Stop fading of an object
 *
 * Use this function to immediately stop the fade of an object without
 * restoring its original color/alpha.
 *
 * @param obj An object
 *
 * @ingroup Efx_Fade
 */
EAPI void efx_fade_stop(Evas_Object *obj);

/**
 * @}
 */

/**
 * @defgroup Efx_Rotation Efx Rotation Effects
 * @ingroup Efx
 *
 * @{
 *
 * There are two ways of rotation an object: using @ref efx_rotate(),
 * where the object will rotate per specified degrees around a  central point,
 * or @ref efx_spin_start(), making the object rotates specified degrees per
 * second until it's explicitly stopped.
 */

/**
 * Helper macro to simplify specifying a rotation effect for queue
 * @param DEGREES Number of degrees to rotate
 * @param CENTER An Evas_Point* to rotate around, or @c NULL
 * @ingroup Efx_Rotation
 */
#define EFX_EFFECT_ROTATE(DEGREES, CENTER) \
  .type = EFX_EFFECT_TYPE_ROTATE, .effect.rotation = { .degrees = (DEGREES), .center = (CENTER) }

/**
 * @brief
 * Rotate an object
 *
 * This function allows rotation of an object around an optional point with
 * specified rotation amount, speed of effect, and duration of effect.
 *
 * @note @p cb will ONLY be called upon successful completion of the effect.
 * @note The actual location of the object will not change; this is a map effect
 *
 * @param obj The object to rotate
 * @param speed The speed to rotate at
 * @param degrees The amount to rotate
 * @param center The optional point to rotate around
 * @param total_time The time that the effect should occur over
 * @param cb The optional callback to call when the effect completes
 * @param data Optional data to pass to @p cb
 * @return @c EINA_TRUE on successful queue of the animation, else
 * @c EINA_FALSE
 *
 * @ingroup Efx_Rotation
 */
EAPI Eina_Bool efx_rotate(Evas_Object *obj, Efx_Effect_Speed speed, double degrees, const Evas_Point *center, double total_time, Efx_End_Cb cb, const void *data);

/**
 * @brief
 * Stop rotation of an object and remove the map
 *
 * Use this function to immediately stop the rotation of an object and restore
 * its original position
 *
 * @param obj An object
 *
 * @ingroup Efx_Rotation
 */
EAPI void efx_rotate_reset(Evas_Object *obj);

/**
 * @brief
 * Stop rotation of an object
 *
 * Use this function to immediately stop the rotation of an object without restoring its original position
 * @param obj An object
 *
 * @ingroup Efx_Rotation
 */
EAPI void efx_rotate_stop(Evas_Object *obj);

/**
 * @brief
 * Spin an object
 *
 * This function allows rotation of an object around an optional point.
 * The object
 * will rotate at @p dps degrees per second until manually stopped.
 * @note The actual location of the object will not change; this is a map effect
 *
 * @param obj The object to rotate
 * @param dps The degrees per second to rotate
 * @param center The optional point to rotate around
 * @return @c EINA_TRUE on successful queue of the animation, else
 * @c EINA_FALSE
 *
 * @ingroup Efx_Rotation
 */
EAPI Eina_Bool efx_spin_start(Evas_Object *obj, long dps, const Evas_Point *center);

/**
 * @brief
 * Stop rotation of an object and remove the map
 *
 * Use this function to immediately stop the rotation of an object and restore
 * its original position
 *
 * @param obj An object
 *
 * @ingroup Efx_Rotation
 */
EAPI void efx_spin_reset(Evas_Object *obj);

/**
 * @brief
 * Stop rotation of an object
 *
 * Use this function to immediately stop the rotation of an object without
 * restoring its original position
 *
 * @param obj An object
 *
 * @ingroup Efx_Rotation
 */
EAPI void efx_spin_stop(Evas_Object *obj);

/**
 * @}
 */

/**
 * @defgroup Efx_Zoom Efx Zoom Effects
 * @ingroup Efx
 *
 * @{
 *
 * It's possible to add zoom in / out effects using @ref efx_zoom()
 */

/**
 * Helper macro to simplify specifying a zoom effect for queue
 * @param START Starting zoom factor
 * @param END Ending zoom factor
 * @param CENTER An Evas_Point* to zoom at, or @c NULL
 * @ingroup Efx_Zoom
 */
#define EFX_EFFECT_ZOOM(START, END, CENTER) \
  .type = EFX_EFFECT_TYPE_ZOOM, .effect.zoom = { .start = (START), .end = (END), .center = (CENTER) }

/**
 * @brief
 * Zoom an object
 *
 * This function allows zooming of an object at an optional point with
 * specified zoom amount, starting amount, speed of effect, and time to
 * complete effect.
 *
 * @note @p cb will ONLY be called upon successful completion of the effect.
 * @note The actual location of the object will not change; this is a map effect
 *
 * @param obj The object to zoom
 * @param speed The speed to zoom at
 * @param starting_zoom The zoom amount to start at, or 0 to use the
 * previously existing zoom amount (defaults to 1.0)
 * @param ending_zoom The zoom amount to end at
 * @param zoom_point The optional point to center the zoom on
 * @param total_time The time that the effect should occur over
 * @param cb The optional callback to call when the effect completes
 * @param data Optional data to pass to @p cb
 * @return @c EINA_TRUE on successful queue of the animation, else
 * @c EINA_FALSE
 *
 * @ingroup Efx_Zoom
 */
EAPI Eina_Bool efx_zoom(Evas_Object *obj, Efx_Effect_Speed speed, double starting_zoom, double ending_zoom, const Evas_Point *zoom_point, double total_time, Efx_End_Cb cb, const void *data);

/**
 * @brief
 * Stop zooming of an object and remove the map
 *
 * Use this function to immediately stop the zoom of an object and restore its
 * original position
 *
 * @param obj An object
 *
 * @ingroup Efx_Zoom
 */
EAPI void efx_zoom_reset(Evas_Object *obj);

/**
 * @brief
 * Stop zooming of an object
 *
 * Use this function to immediately stop the zoom of an object without
 * restoring its original position
 *
 * @param obj An object
 *
 * @ingroup Efx_Zoom
 */
EAPI void efx_zoom_stop(Evas_Object *obj);

/**
 * @}
 */

/**
 * @defgroup Efx_Move Efx Movement Effects
 * @ingroup Efx
 *
 * @{
 *
 * Object movement can be done directly using @ref efx_move() or using
 * @ref efx_pan();
 * A "pan" is a motion which shifts the view of an object from one point to
 * another.
 */

/**
 * Helper macro to simplify specifying a movement effect for queue
 * @param X x coordinate to move to
 * @param Y y coordinate to move to
 * @ingroup Efx_Move
 */
#define EFX_EFFECT_MOVE(X, Y) \
  .type = EFX_EFFECT_TYPE_MOVE, .effect.movement.point = { .x = (X), .y = (Y) }

/**
 * Helper macro to simplify specifying a pan effect for queue
 * @param X horizontal distance to pan
 * @param Y vertical distance to pan
 * @ingroup Efx_Move
 */
#define EFX_EFFECT_PAN(X, Y) \
  .type = EFX_EFFECT_TYPE_PAN, .effect.movement.point = { .x = (X), .y = (Y) }

/**
 * @brief
 * Move an object
 *
 * This function animates the movement of an object to a specified point using
 * a designated speed of effect and time to complete the effect.
 *
 * @note @p cb will ONLY be called upon successful completion of the effect.
 * @note The actual location of the object WILL change; this is NOT a map effect
 *
 * @param obj The object to zoom
 * @param speed The speed to move at
 * @param end_point The point to move to
 * @param total_time The time that the effect should occur over
 * @param cb The optional callback to call when the effect completes
 * @param data Optional data to pass to @p cb
 * @return @c EINA_TRUE on successful queue of the animation, else
 * @c EINA_FALSE
 *
 * @ingroup Efx_Move
 */
EAPI Eina_Bool efx_move(Evas_Object *obj, Efx_Effect_Speed speed, const Evas_Point *end_point, double total_time, Efx_End_Cb cb, const void *data);

/**
 * @brief
 * Move an object in circle
 *
 * This function animates the movement of an object around a specified
 * point using a designated degrees, speed of effect and time to complete
 * the effect.
 *
 * @note @p cb will ONLY be called upon successful completion of the effect.
 * @note The actual location of the object WILL change; this is NOT a map effect
 *
 * @param obj The object to zoom
 * @param speed The speed to move at
 * @param center The center point to move around
 * @param degrees The amount of degrees to move around the center
 * @param total_time The time that the effect should occur over
 * @param cb The optional callback to call when the effect completes
 * @param data Optional data to pass to @p cb
 * @return @c EINA_TRUE on successful queue of the animation, else
 * @c EINA_FALSE
 *
 * @ingroup Efx_Move
 */
EAPI Eina_Bool efx_move_circle(Evas_Object *obj, Efx_Effect_Speed speed, const Evas_Point *center, int degrees, double total_time, Efx_End_Cb cb, const void *data);

/**
 * @brief
 * Initialize an object for panning
 *
 * Use this function immediately after creating an object if you plan
 * to place other layers on top of it. This prevents the newly-created pan
 * object from obscuring other objects. No visible changes will occur from
 * calling this function.
 *
 * @param obj The object to set up for panning
 * @return @c EINA_TRUE on success, else @c EINA_FALSE
 *
 * @ingroup Efx_Move
 */
EAPI Eina_Bool efx_pan_init(Evas_Object *obj);

/**
 * @brief
 * Commence a pan effect
 *
 * A "pan" is a motion which shifts the view of an object from one point to
 * another.
 * This function sets up (calling efx_pan_init() if necessary) and runs a pan
 * effect
 * which will shift the view of the canvas in the directions specified by
 * @p distance.
 *
 * @param obj The object to use for panning
 * @param speed The speed to pan at
 * @param distance The relative X and Y distance to move during the pan
 * @param total_time The time that the effect should occur over
 * @param cb The optional callback to call when the effect completes
 * @param data Optional data to pass to @p cb
 * @return @c EINA_TRUE on successful queue of the animation, else
 * @c EINA_FALSE
 *
 * @ingroup Efx_Move
 */
EAPI Eina_Bool efx_pan(Evas_Object *obj, Efx_Effect_Speed speed, const Evas_Point *distance, double total_time, Efx_End_Cb cb, const void *data);

/**
 * @}
 */

/**
 * @defgroup Efx_Resize Efx Resize Effect
 * @ingroup Efx
 *
 * @{
 *
 * FIXME DESC
 *
 */

/**
 * Helper macro to simplify specifying a resize effect for queue
 * @param POSITION The position for the resized top-left corner to end in
 * @param W The final width of the object
 * @param H The final height of the object
 * @ingroup Efx_Resize
 */
#define EFX_EFFECT_RESIZE(POSITION, W, H) \
  .type = EFX_EFFECT_TYPE_RESIZE, .effect.resize = { .point = (POSITION), .w = (W), .h = (H) }

/**
 * @brief
 * Resize an object
 *
 * This function animates the resizing of an object to a specified width and
 * height using a designated speed of effect and time to complete the effect.
 *
 * @note @p cb will ONLY be called upon successful completion of the effect.
 * @note The actual size of the object WILL change; this is NOT a map effect
 *
 * @param obj The object to zoom
 * @param speed The speed to move at
 * @param position The point to move to. If not provided, the object won't move.
 * @param w The final object width
 * @param h The final object height
 * @param total_time The time that the effect should occur over
 * @param cb The optional callback to call when the effect completes
 * @param data Optional data to pass to @p cb
 * @return @c EINA_TRUE on successful queue of the animation, else
 * @c EINA_FALSE
 *
 * @ingroup Efx_Resize
 */
EAPI Eina_Bool efx_resize(Evas_Object *obj, Efx_Effect_Speed speed, const Evas_Point *position, int w, int h, double total_time, Efx_End_Cb cb, const void *data);

/**
 * @brief
 * Stop resizing of an object
 *
 * Use this function to immediately stop the resize of an object and restore its
 * original size
 *
 * @param obj An object
 *
 * @ingroup Efx_Resize
 */
EAPI void efx_resize_reset(Evas_Object *obj);

/**
 * @brief
 * Stop resizing of an object
 *
 * Use this function to immediately stop the zoom of an object without
 * restoring its original size
 *
 * @param obj An object
 *
 * @ingroup Efx_Resize
 */
EAPI void efx_resize_stop(Evas_Object *obj);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
