#ifndef _pthreadgridVisi_h_
#define _pthreadgridVisi_h_

#include "color3.h"
#include <pthread.h>

#ifdef __APPLE__
#include "pthread_barrier.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * To use this interface you need to first define:
 * ----------------------------------------------
 *  a struct type containing fields for application-specific state
 *  that will be passed to each thread via pthread_create.  One field
 *  in this struct should to be the (color3 *) buffer returned by
 *  get_animation_buffer.  This is the buffer used by threads to
 *  update the visualization.
 *
 * The main program control flow should look like:
 * ----------------------------------------------
 *  (1) initialize all program state (in fields of struct variable)
 *      including any pthread synch primitives
 *  (2) main thread: call init_pthread_animation (only one thread should call)
 *  (3) main thread: call get_animation_buffer to get the color3 buff
 *      (only one thread should call this)
 *  (4) create worker threads (call pthread_create), each should:
 *       (a) init any thread-specific state
 *       (b) loop:
 *           do next computation step
 *           update color3 buf
 *           call draw_ready
 *  (5) main thread: call run_animation to run the visi animation
 *                   (only one thread should call this)
 *  (6) wait for worker threads to exit, then clean-up
 */

/* exported typedef for internal state passed between library functions */
struct visi_struct;
typedef struct visi_struct *visi_handle;

/* initialize the visualization:
 * this should only be called once (by one thread)
 *
 *     num_tids: total number of threads participating in the computation
 *     rows: number of rows
 *     cols: number of cols
 *     name: name for visi window
 *     returns: a visi_handle or NULL on error
 */
visi_handle init_pthread_animation(int num_tids, int rows, int cols,
   char *name);

/* None of this below is really pthreads specific, but since
   it would be difficult to call draw_ready in an update loop
   and separately call run_animation without having at least
   two separate threads of execution, we keep these functions
   here in pthreadGridVisi.h until another framework can also
   use them. */

/*
 *  get the color3 buffer associated with a visualization
 *   handle:  a handle to a visualization
 *   returns: pointer to color3 buffer for the visualization,
 *   or NULL on error
 */
color3 *get_animation_buffer(visi_handle handle);

/*  Directly call update outside of the main animation loop.
 *  Usually used by multi-threaded application to indicate each thread
 *  has processed its portion of the buffer.
 */
void draw_ready(visi_handle handle);

/*
 * runs the a previously initialized animation:
 *   handle: value returned by call to init_*_animation that sets up
 *           a particular type of animation
 *   iters: run for specified number of iterations, or if 0 run forever
 */
void run_animation(visi_handle handle, int iters);

#ifdef __cplusplus
}
#endif

/* TODO: Add free_handle() function to complement init? */
#endif