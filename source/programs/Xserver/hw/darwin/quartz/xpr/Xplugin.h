/* Xplugin.h -- windowing API for rootless X11 server

   Copyright (c) 2002 Apple Computer, Inc. All rights reserved.

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation files
   (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT
   HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.

   Except as contained in this notice, the name(s) of the above
   copyright holders shall not be used in advertising or otherwise to
   promote the sale, use or other dealings in this Software without
   prior written authorization.

   Note that these interfaces are provided solely for the use of the
   X11 server. Any other uses are unsupported and strongly discouraged. */
/* $XFree86: xc/programs/Xserver/hw/darwin/quartz/xpr/Xplugin.h,v 1.5 2007/04/03 00:21:10 tsi Exp $ */

#ifndef XPLUGIN_H
#define XPLUGIN_H 1

#include <stdint.h>

/* By default we use the X server definition of BoxRec to define xp_box,
   so that the compiler can silently convert between the two. But if
   XP_NO_X_HEADERS is defined, we'll define it ourselves. */

#ifndef XP_NO_X_HEADERS
# include "miscstruct.h"
  typedef BoxRec xp_box;
#else
  struct xp_box_struct {
      short x1, y1, x2, y2;
  };
  typedef struct xp_box_struct xp_box;
#endif

typedef unsigned int xp_resource_id;
typedef xp_resource_id xp_window_id;
typedef xp_resource_id xp_surface_id;
typedef unsigned int xp_client_id;
typedef unsigned int xp_request_type;
typedef int xp_error;
typedef int xp_bool;


/* Error codes that the functions declared here may return. They all
   numerically match their X equivalents, i.e. the XP_ can be dropped
   if <X11/X.h> has been included. */

enum xp_error_enum {
    XP_Success			= 0,
    XP_BadRequest		= 1,
    XP_BadValue			= 2,
    XP_BadWindow		= 3,
    XP_BadMatch			= 8,
    XP_BadAccess		= 10,
    XP_BadImplementation	= 17,
};    


/* Event types generated by the plugin. */

enum xp_event_type_enum {
    /* The global display configuration changed somehow. */
    XP_EVENT_DISPLAY_CHANGED	= 1 << 0,

    /* A window changed state. Argument is xp_window_state_event */
    XP_EVENT_WINDOW_STATE_CHANGED = 1 << 1,

    /* An async request encountered an error. Argument is of type
       xp_async_error_event */
    XP_EVENT_ASYNC_ERROR	= 1 << 2,

    /* Sent when a surface is destroyed as a side effect of destroying
       a window. Arg is of type xp_surface_id. */
    XP_EVENT_SURFACE_DESTROYED	= 1 << 3,

    /* Sent when any GL contexts pointing at the given surface need to
       call xp_update_gl_context () to refresh their state (because the
       window moved or was resized. Arg is of type xp_surface_id. */
    XP_EVENT_SURFACE_CHANGED	= 1 << 4,

    /* Sent when a window has been moved. Arg is of type xp_window_id. */
    XP_EVENT_WINDOW_MOVED	= 1 << 5,
};

/* Function type used to receive events. */

typedef void (xp_event_fun) (unsigned int type, const void *arg,
			     unsigned int arg_size, void *user_data);


/* Operation types. Used when reporting errors asynchronously. */

enum xp_request_type_enum {
    XP_REQUEST_NIL = 0,
    XP_REQUEST_DESTROY_WINDOW = 1,
    XP_REQUEST_CONFIGURE_WINDOW = 2,
    XP_REQUEST_FLUSH_WINDOW = 3,
    XP_REQUEST_COPY_WINDOW = 4,
    XP_REQUEST_UNLOCK_WINDOW = 5,
    XP_REQUEST_DISABLE_UPDATE = 6,
    XP_REQUEST_REENABLE_UPDATE = 7,
    XP_REQUEST_HIDE_CURSOR = 8,
    XP_REQUEST_SHOW_CURSOR = 9,
    XP_REQUEST_FRAME_DRAW = 10,
};

/* Structure used to report an error asynchronously. Passed as the "arg"
   of an XP_EVENT_ASYNC_ERROR event. */

struct xp_async_error_event_struct {
    xp_request_type request_type;
    xp_resource_id id;
    xp_error error;
};

typedef struct xp_async_error_event_struct xp_async_error_event;


/* Possible window states. */

enum xp_window_state_enum {
    /* The window is not in the global list of possibly-visible windows. */
    XP_WINDOW_STATE_OFFSCREEN	= 1 << 0,

    /* Parts of the window may be obscured by other windows. */
    XP_WINDOW_STATE_OBSCURED	= 1 << 1,
};

/* Structure passed as argument of an XP_EVENT_WINDOW_STATE_CHANGED event. */

struct xp_window_state_event_struct {
    xp_window_id id;
    unsigned int state;
};

typedef struct xp_window_state_event_struct xp_window_state_event;


/* Function type used to supply a colormap for indexed drawables. */

typedef xp_error (xp_colormap_fun) (void *data, int first_color,
				    int n_colors, uint32_t *colors);


/* Window attributes structure. Used when creating and configuring windows.
   Also used when configuring surfaces attached to windows. Functions that
   take one of these structures also take a bit mask defining which
   fields are set to meaningful values. */

enum xp_window_changes_enum {
    XP_ORIGIN			= 1 << 0,
    XP_SIZE			= 1 << 1,
    XP_BOUNDS			= XP_ORIGIN | XP_SIZE,
    XP_SHAPE			= 1 << 2,
    XP_STACKING			= 1 << 3,
    XP_DEPTH			= 1 << 4,
    XP_COLORMAP			= 1 << 5,
    XP_WINDOW_LEVEL		= 1 << 6,
};

struct xp_window_changes_struct {
    /* XP_ORIGIN */
    int x, y;

    /* XP_SIZE */
    unsigned int width, height;
    int bit_gravity;			/* how to resize the backing store */

    /* XP_SHAPE */
    int shape_nrects;			/* -1 = remove shape */
    xp_box *shape_rects;
    int shape_tx, shape_ty;		/* translation for shape */

    /* XP_STACKING */
    int stack_mode;
    xp_window_id sibling;		/* may be zero; in ABOVE/BELOW modes
					   it may specify a relative window */
    /* XP_DEPTH, window-only */
    unsigned int depth;

    /* XP_COLORMAP, window-only */
    xp_colormap_fun *colormap;
    void *colormap_data;

    /* XP_WINDOW_LEVEL, window-only */
    int window_level;
};

typedef struct xp_window_changes_struct xp_window_changes;

/* Values for bit_gravity field */

enum xp_bit_gravity_enum {
    XP_GRAVITY_NONE		= 0,	/* no gravity, fill everything */
    XP_GRAVITY_NORTH_WEST	= 1,	/* anchor to top-left corner */
    XP_GRAVITY_NORTH_EAST	= 2,	/* anchor to top-right corner */
    XP_GRAVITY_SOUTH_EAST	= 3,	/* anchor to bottom-right corner */
    XP_GRAVITY_SOUTH_WEST	= 4,	/* anchor to bottom-left corner */
};

/* Values for stack_mode field */

enum xp_window_stack_mode_enum {
    XP_UNMAPPED			= 0,	/* remove the window */
    XP_MAPPED_ABOVE		= 1,	/* display the window on top */
    XP_MAPPED_BELOW		= 2,	/* display the window at bottom */
};

/* Data formats for depth field and composite functions */

enum xp_depth_enum {
    XP_DEPTH_NIL = 0,			/* null source when compositing */
    XP_DEPTH_ARGB8888,
    XP_DEPTH_RGB555,
    XP_DEPTH_A8,			/* for masks when compositing */
    XP_DEPTH_INDEX8,
};

/* Options that may be passed to the xp_init () function. */

enum xp_init_options_enum {
    /* Don't mark that this process can be in the foreground. */
    XP_IN_BACKGROUND		= 1 << 0,

    /* Deliver background pointer events to this process. */
    XP_BACKGROUND_EVENTS	= 1 << 1,
};



/* Miscellaneous functions */

/* Initialize the plugin library. Only the copy/fill/composite functions
   may be called without having previously called xp_init () */

extern xp_error xp_init (unsigned int options);

/* Sets the current set of requested notifications to MASK. When any of
   these arrive, CALLBACK will be invoked with CALLBACK-DATA. Note that
   calling this function cancels any previously requested notifications
   that aren't set in MASK. */

extern xp_error xp_select_events (unsigned int mask,
				  xp_event_fun *callback,
				  void *callback_data);

/* Waits for all initiated operations to complete. */

extern xp_error xp_synchronize (void);

/* Causes any display update initiated through the plugin libary to be
   queued until update is reenabled. Note that calls to these functions
   nest. */
  
extern xp_error xp_disable_update (void);
extern xp_error xp_reenable_update (void);



/* Cursor functions. */

/* Installs the specified cursor. ARGB-DATA should point to 32-bit
   premultiplied big-endian ARGB data. The HOT-X,HOT-Y parameters
   specify the offset to the cursor's hot spot from its top-left
   corner. */

extern xp_error xp_set_cursor (unsigned int width, unsigned int height,
			       unsigned int hot_x, unsigned int hot_y,
			       const uint32_t *argb_data,
			       unsigned int rowbytes);

/* Hide and show the cursor if it's owned by the current process. Calls
   to these functions nest. */

extern xp_error xp_hide_cursor (void);
extern xp_error xp_show_cursor (void);



/* Window functions. */

/* Create a new window as defined by MASK and VALUES. MASK must contain
   XP_BOUNDS or an error is raised. The id of the newly created window
   is stored in *RET-ID if this function returns XP_Success. */

extern xp_error xp_create_window (unsigned int mask,
				  const xp_window_changes *values,
				  xp_window_id *ret_id);

/* Destroys the window identified by ID. */

extern xp_error xp_destroy_window (xp_window_id id);

/* Reconfigures the given window according to MASK and VALUES. */

extern xp_error xp_configure_window (xp_window_id id, unsigned int mask,
				     const xp_window_changes *values);


/* Returns true if NATIVE-ID is a window created by the plugin library.
   If so and RET-ID is non-null, stores the id of the window in *RET-ID. */

extern xp_bool xp_lookup_native_window (unsigned int native_id,
					xp_window_id *ret_id);

/* If ID names a window created by the plugin library, stores it's native
   window id in *RET-NATIVE-ID. */

extern xp_error xp_get_native_window (xp_window_id id,
				      unsigned int *ret_native_id);


/* Locks the rectangle IN-RECT (or, if null, the entire window) of the
   given window's backing store. Any other non-null parameters are filled
   in as follows:

   DEPTH = format of returned data. Currently either XP_DEPTH_ARGB8888
   or XP_DEPTH_RGB565 (possibly with 8 bit planar alpha). Data is
   always stored in native byte order.

   BITS[0] = pointer to top-left pixel of locked color data
   BITS[1] = pointer to top-left of locked alpha data, or null if window
   has no alpha. If the alpha data is meshed, then BITS[1] = BITS[0].

   ROWBYTES[0,1] = size in bytes of each row of color,alpha data

   OUT-RECT = rectangle specifying the current position and size of the
   locked region relative to the window origin.

   Note that an error is raised when trying to lock an already locked
   window. While the window is locked, the only operations that may
   be performed on it are to modify, access or flush its marked region. */

extern xp_error xp_lock_window (xp_window_id id,
				const xp_box *in_rect,
				unsigned int *depth,
				void *bits[2],
				unsigned int rowbytes[2],
				xp_box *out_rect);

/* Mark that the region specified by SHAPE-NRECTS, SHAPE-RECTS,
   SHAPE-TX, and SHAPE-TY in the specified window has been updated, and
   will need to subsequently be redisplayed. */

extern xp_error xp_mark_window (xp_window_id id, int shape_nrects,
				const xp_box *shape_rects,
				int shape_tx, int shape_ty);

/* Unlocks the specified window. If FLUSH is true, then any marked
   regions are immediately redisplayed. Note that it's an error to
   unlock an already unlocked window. */

extern xp_error xp_unlock_window (xp_window_id id, xp_bool flush);

/* If anything is marked in the given window for redisplay, do it now. */

extern xp_error xp_flush_window (xp_window_id id);

/* Moves the contents of the region DX,DY pixels away from that specified
   by DST_RECTS and DST_NRECTS in the window with SRC-ID to the
   destination region in the window DST-ID. Note that currently source
   and destination windows must be the same. */

extern xp_error xp_copy_window (xp_window_id src_id, xp_window_id dst_id,
				int dst_nrects, const xp_box *dst_rects,
				int dx, int dy);

/* Returns true if the given window has any regions marked for
   redisplay. */

extern xp_bool xp_is_window_marked (xp_window_id id);

/* If successful returns a superset of the region marked for update in
   the given window. Use xp_free_region () to release the returned data. */

extern xp_error xp_get_marked_shape (xp_window_id id,
				     int *ret_nrects, xp_box **ret_rects);

extern void xp_free_shape (int nrects, xp_box *rects);

/* Searches for the first window below ABOVE-ID containing the point X,Y,
   and returns it's window id in *RET-ID. If no window is found, *RET-ID
   is set to zero. If ABOVE-ID is zero, finds the topmost window
   containing the given point. */

extern xp_error xp_find_window (int x, int y, xp_window_id above_id,
				xp_window_id *ret_id);

/* Returns the current origin and size of the window ID in *BOUNDS-RET if
   successful. */
extern xp_error xp_get_window_bounds (xp_window_id id, xp_box *bounds_ret);



/* Window surface functions. */

/* Create a new VRAM surface on the specified window. If successful,
   returns the identifier of the new surface in *RET-SID. */

extern xp_error xp_create_surface (xp_window_id id, xp_surface_id *ret_sid);

/* Destroys the specified surface. */

extern xp_error xp_destroy_surface (xp_surface_id sid);

/* Reconfigures the specified surface as defined by MASK and VALUES.
   Note that specifying XP_DEPTH is an error. */

extern xp_error xp_configure_surface (xp_surface_id sid, unsigned int mask,
				      const xp_window_changes *values);

/* If successful, places the client identifier of the current process
   in *RET-CLIENT. */

extern xp_error xp_get_client_id (xp_client_id *ret_client);

/* Given a valid window,surface combination created by the current
   process, attempts to allow the specified external client access
   to that surface. If successful, returns two integers in RET-KEY
   which the client can use to import the surface into their process. */

extern xp_error xp_export_surface (xp_window_id wid, xp_surface_id sid,
				   xp_client_id client,
				   unsigned int ret_key[2]);

/* Given a two integer key returned from xp_export_surface (), tries
   to import the surface into the current process. If successful the
   local surface identifier is stored in *SID-RET. */

extern xp_error xp_import_surface (const unsigned int key[2],
				   xp_surface_id *sid_ret);

/* If successful, stores the number of surfaces attached to the
   specified window in *RET. */

extern xp_error xp_get_window_surface_count (xp_window_id id,
					     unsigned int *ret);

/* Attaches the CGLContextObj CGL-CTX to the specified surface. */

extern xp_error xp_attach_gl_context (void *cgl_ctx, xp_surface_id sid);

/* Updates the CGLContextObj CGL-CTX to reflect any recent changes to
   the surface it's attached to. */

extern xp_error xp_update_gl_context (void *cgl_ctx);



/* Window frame functions. */

/* Possible arguments to xp_frame_get_rect (). */

enum xp_frame_rect_enum {
    XP_FRAME_RECT_TITLEBAR		= 1,
    XP_FRAME_RECT_TRACKING		= 2,
    XP_FRAME_RECT_GROWBOX		= 3,
};

/* Classes of window frame. */

enum xp_frame_class_enum {
    XP_FRAME_CLASS_DOCUMENT		= 1 << 0,
    XP_FRAME_CLASS_DIALOG		= 1 << 1,
    XP_FRAME_CLASS_MODAL_DIALOG		= 1 << 2,
    XP_FRAME_CLASS_SYSTEM_MODAL_DIALOG	= 1 << 3,
    XP_FRAME_CLASS_UTILITY		= 1 << 4,
    XP_FRAME_CLASS_TOOLBAR		= 1 << 5,
    XP_FRAME_CLASS_MENU			= 1 << 6,
    XP_FRAME_CLASS_SPLASH		= 1 << 7,
    XP_FRAME_CLASS_BORDERLESS		= 1 << 8,
};

/* Attributes of window frames. */

enum xp_frame_attr_enum {
    XP_FRAME_ACTIVE			= 0x0001,
    XP_FRAME_URGENT			= 0x0002,
    XP_FRAME_TITLE			= 0x0004,
    XP_FRAME_PRELIGHT			= 0x0008,
    XP_FRAME_SHADED			= 0x0010,
    XP_FRAME_CLOSE_BOX			= 0x0100,
    XP_FRAME_COLLAPSE			= 0x0200,
    XP_FRAME_ZOOM			= 0x0400,
    XP_FRAME_ANY_BUTTON			= 0x0700,
    XP_FRAME_CLOSE_BOX_CLICKED		= 0x0800,
    XP_FRAME_COLLAPSE_BOX_CLICKED	= 0x1000,
    XP_FRAME_ZOOM_BOX_CLICKED		= 0x2000,
    XP_FRAME_ANY_CLICKED		= 0x3800,
    XP_FRAME_GROW_BOX			= 0x4000,
};

#define XP_FRAME_ATTR_IS_SET(a,b)	(((a) & (b)) == (b))
#define XP_FRAME_ATTR_IS_CLICKED(a,m)	((a) & ((m) << 3))
#define XP_FRAME_ATTR_SET_CLICKED(a,m)	((a) |= ((m) << 3))
#define XP_FRAME_ATTR_UNSET_CLICKED(a,m) ((a) &= ~((m) << 3))

#define XP_FRAME_POINTER_ATTRS		(XP_FRAME_PRELIGHT		\
					 | XP_FRAME_ANY_BUTTON		\
					 | XP_FRAME_ANY_CLICKED)

extern xp_error xp_frame_get_rect (int type, int class, const xp_box *outer,
				   const xp_box *inner, xp_box *ret);
extern xp_error xp_frame_hit_test (int class, int x, int y,
				   const xp_box *outer,
				   const xp_box *inner, int *ret);
extern xp_error xp_frame_draw (xp_window_id wid, int class, unsigned int attr,
			       const xp_box *outer, const xp_box *inner,
			       unsigned int title_len,
			       const char *title_bytes);



/* Memory manipulation functions. */

enum xp_composite_op_enum {
    XP_COMPOSITE_SRC = 0,
    XP_COMPOSITE_OVER,
};

#define XP_COMPOSITE_FUNCTION(op, src_depth, mask_depth, dest_depth) \
    (((op) << 24) | ((src_depth) << 16) \
     | ((mask_depth) << 8) | ((dest_depth) << 0))

#define XP_COMPOSITE_FUNCTION_OP(f)         (((f) >> 24) & 255)
#define XP_COMPOSITE_FUNCTION_SRC_DEPTH(f)  (((f) >> 16) & 255)
#define XP_COMPOSITE_FUNCTION_MASK_DEPTH(f) (((f) >>  8) & 255)
#define XP_COMPOSITE_FUNCTION_DEST_DEPTH(f) (((f) >>  0) & 255)

/* Composite WIDTH by HEIGHT pixels from source and mask to destination
   using a specified function (if source and destination overlap,
   undefined behavior results).

   For SRC and DEST, the first element of the array is the color data. If
   the second element is non-null it implies that there is alpha data
   (which may be meshed or planar). Data without alpha is assumed to be
   opaque.

   Passing a null SRC-ROWBYTES pointer implies that the data SRC points
   to is a single element.

   Operations that are not supported will return XP_BadImplementation. */

extern xp_error xp_composite_pixels (unsigned int width, unsigned int height,
				     unsigned int function,
				     void *src[2], unsigned int src_rowbytes[2],
				     void *mask, unsigned int mask_rowbytes,
				     void *dest[2], unsigned int dest_rowbytes[2]);

/* Fill HEIGHT rows of data starting at DST. Each row will have WIDTH
   bytes filled with the 32-bit pattern VALUE. Each row is DST-ROWBYTES
   wide in total. */

extern void xp_fill_bytes (unsigned int width,
			   unsigned int height, uint32_t value,
			   void *dst, unsigned int dst_rowbytes);

/* Copy HEIGHT rows of bytes from SRC to DST. Each row will have WIDTH
   bytes copied. SRC and DST may overlap, and the right thing will happen. */

extern void xp_copy_bytes (unsigned int width, unsigned int height,
			   const void *src, unsigned int src_rowbytes,
			   void *dst, unsigned int dst_rowbytes);

/* Suggestions for the minimum number of bytes or pixels for which it
   makes sense to use some of the xp_ functions */

extern unsigned int xp_fill_bytes_threshold, xp_copy_bytes_threshold,
    xp_composite_area_threshold, xp_scroll_area_threshold;


#endif /* XPLUGIN_H */
