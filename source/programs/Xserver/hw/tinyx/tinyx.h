/*
 * Copyright � 1999 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/hw/tinyx/tinyx.h,v 1.4 2006/11/02 21:55:25 tsi Exp $ */
/*
 * Copyright (c) 2004 by The XFree86 Project, Inc.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 *   1.  Redistributions of source code must retain the above copyright
 *       notice, this list of conditions, and the following disclaimer.
 *
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer
 *       in the documentation and/or other materials provided with the
 *       distribution, and in the same place and form as other copyright,
 *       license and disclaimer information.
 *
 *   3.  The end-user documentation included with the redistribution,
 *       if any, must include the following acknowledgment: "This product
 *       includes software developed by The XFree86 Project, Inc
 *       (http://www.xfree86.org/) and its contributors", in the same
 *       place and form as other third-party acknowledgments.  Alternately,
 *       this acknowledgment may appear in the software itself, in the
 *       same form and location as other such third-party acknowledgments.
 *
 *   4.  Except as contained in this notice, the name of The XFree86
 *       Project, Inc shall not be used in advertising or otherwise to
 *       promote the sale, use or other dealings in this Software without
 *       prior written authorization from The XFree86 Project, Inc.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE XFREE86 PROJECT, INC OR ITS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <X11/X.h>
#define NEED_EVENTS
#include <X11/Xproto.h>
#include <X11/Xos.h>
#include "scrnintstr.h"
#include "pixmapstr.h"
#include "windowstr.h"
#include "servermd.h"
#include "mibstore.h"
#include "colormapst.h"
#include "gcstruct.h"
#include "input.h"
#include "mipointer.h"
#include "mi.h"
#include "dix.h"
#include "fb.h"
#include "fboverlay.h"
#include "shadow.h"
#include "randrstr.h"

#define KD_DPMS_NORMAL	    0
#define KD_DPMS_STANDBY	    1
#define KD_DPMS_SUSPEND	    2
#define KD_DPMS_POWERDOWN   3
#define KD_DPMS_MAX	    KD_DPMS_POWERDOWN

#ifndef KD_MAX_FB
#define KD_MAX_FB   FB_OVERLAY_MAX
#endif

#ifndef KD_MAX_CARD_ADDRESS
#define KD_MAX_CARD_ADDRESS 8
#endif

/*
 * Configuration information per video card
 */

typedef struct _KdCardAttr {
    CARD32  io;
    CARD32  address[KD_MAX_CARD_ADDRESS];
    int	    naddr;
    int     bus;
} KdCardAttr;

typedef struct _KdCardInfo {
    struct _KdCardFuncs	    *cfuncs;
    void		    *closure;
    KdCardAttr		    attr;
    void		    *driver;
    struct _KdScreenInfo    *screenList;
    int			    selected;
    struct _KdCardInfo	    *next;
    Bool		    needSync;
} KdCardInfo;

extern KdCardInfo	*kdCardInfo;

/*
 * Configuration information per X screen
 */
typedef struct _KdFrameBuffer {
    CARD8	*frameBuffer;
    int		depth;
    int		bitsPerPixel;
    int		pixelStride;
    int		byteStride;
    unsigned long   visuals;
    Pixel       redMask, greenMask, blueMask;
    void	*closure;
} KdFrameBuffer;

#define RR_Rotate_All	(RR_Rotate_0|RR_Rotate_90|RR_Rotate_180|RR_Rotate_270)
#define RR_Reflect_All	(RR_Reflect_X|RR_Reflect_Y)

typedef struct _KdScreenInfo {
    struct _KdScreenInfo    *next;
    KdCardInfo	*card;
    ScreenPtr	pScreen;
    void	*driver;
    Rotation	randr;	/* rotation and reflection */
    int		width;
    int		height;
    int		rate;
    int		width_mm;
    int		height_mm;
    int		subpixel_order;
    Bool        dumb;
    Bool        softCursor;
    int		mynum;
    DDXPointRec	origin;
    KdFrameBuffer   fb[KD_MAX_FB];
} KdScreenInfo;

typedef struct _KdCardFuncs {
    Bool	(*cardinit) (KdCardInfo *); /* detect and map device */
    Bool	(*scrinit) (KdScreenInfo *);/* initialize screen information */
    Bool	(*initScreen) (ScreenPtr);  /* initialize ScreenRec */
    void	(*preserve) (KdCardInfo *); /* save graphics card state */
    Bool        (*enable) (ScreenPtr);      /* set up for rendering */
    Bool	(*dpms) (ScreenPtr, int);   /* set DPMS screen saver */
    void        (*disable) (ScreenPtr);     /* turn off rendering */
    void	(*restore) (KdCardInfo *);  /* restore graphics card state */
    void	(*scrfini) (KdScreenInfo *);/* close down screen */
    void        (*cardfini) (KdCardInfo *); /* close down */

    Bool        (*initCursor) (ScreenPtr);      /* detect and map cursor */
    void        (*enableCursor) (ScreenPtr);    /* enable cursor */
    void        (*disableCursor) (ScreenPtr);   /* disable cursor */
    void        (*finiCursor) (ScreenPtr);      /* close down */
    void        (*recolorCursor) (ScreenPtr, int, xColorItem *);

    Bool        (*initAccel) (ScreenPtr);
    void        (*enableAccel) (ScreenPtr);
    void	(*syncAccel) (ScreenPtr);
    void        (*disableAccel) (ScreenPtr);
    void        (*finiAccel) (ScreenPtr);

    void        (*getColors) (ScreenPtr, int, int, xColorItem *);
    void        (*putColors) (ScreenPtr, int, int, xColorItem *);

    Bool	(*finishInitScreen) (ScreenPtr pScreen);
} KdCardFuncs;

#define KD_MAX_PSEUDO_DEPTH 8
#define KD_MAX_PSEUDO_SIZE	    (1 << KD_MAX_PSEUDO_DEPTH)

typedef struct {
    KdScreenInfo    *screen;
    KdCardInfo	    *card;

    Bool	    enabled;
    Bool	    closed;
    int		    bytesPerPixel[KD_MAX_FB];

    int		    dpmsState;
    
    ColormapPtr     pInstalledmap[KD_MAX_FB];         /* current colormap */
    xColorItem      systemPalette[KD_MAX_PSEUDO_SIZE];/* saved windows colors */

    CloseScreenProcPtr  CloseScreen;
#ifdef FB_OLD_SCREEN
    miBSFuncRec	    BackingStoreFuncs;
#endif
} KdPrivScreenRec, *KdPrivScreenPtr;

typedef enum _kdMouseState {
    start,
    button_1_pend,
    button_1_down,
    button_2_down,
    button_3_pend,
    button_3_down,
    synth_2_down_13,
    synth_2_down_3,
    synth_2_down_1,
    num_input_states
} KdMouseState;

#define KD_MAX_BUTTON  7

typedef struct _KdMouseInfo {
    struct _KdMouseInfo	*next;
    void		*driver;
    void		*closure;
    char		*name;
    char		*prot;
    char		map[KD_MAX_BUTTON];
    int			nbutton;
    Bool		emulateMiddleButton;
    unsigned long	emulationTimeout;
    Bool		timeoutPending;
    KdMouseState	mouseState;
    Bool		eventHeld;
    xEvent		heldEvent;
    unsigned char	buttonState;
    int			emulationDx, emulationDy;
    int			inputType;
} KdMouseInfo;

extern KdMouseInfo	*kdMouseInfo;
extern Bool		kdNoSerialMouse;

#ifdef TOUCHSCREEN
/* 
 * HACK! Send absolute events when touch screen is current,
 * else send relative events.  Used to drive pointers on
 * alternate screens with the touch screen
 */
extern int KdTsCurScreen;
extern int KdTsPhyScreen;
#endif

KdMouseInfo *KdMouseInfoAdd (void);
void	    KdParseMouse (const char *);

typedef struct _KdMouseFuncs {
    int		    (*Init) (void);
    void	    (*Fini) (void);
} KdMouseFuncs;

typedef struct _KdKeyboardFuncs {
    void	    (*Load) (void);
    int		    (*Init) (void);
    void	    (*Leds) (int);
    void	    (*Bell) (int, int, int);
    void	    (*Fini) (void);
    int		    LockLed;
} KdKeyboardFuncs;

typedef struct _KdOsFuncs {
    int		    (*Init) (void);
    void	    (*Enable) (void);
    Bool	    (*SpecialKey) (KeySym);
    void	    (*Disable) (void);
    void	    (*Fini) (void);
} KdOsFuncs;

typedef enum _KdSyncPolarity {
    KdSyncNegative, KdSyncPositive
} KdSyncPolarity;

typedef struct _KdMonitorTiming {
    /* label */
    int		    horizontal;
    int		    vertical;
    int		    rate;
    /* pixel clock */
    int		    clock;  /* in KHz */
    /* horizontal timing */
    int		    hfp;    /* front porch */
    int		    hbp;    /* back porch */
    int		    hblank; /* blanking */
    KdSyncPolarity  hpol;   /* polarity */
    /* vertical timing */
    int		    vfp;    /* front porch */
    int		    vbp;    /* back porch */
    int		    vblank; /* blanking */
    KdSyncPolarity  vpol;   /* polarity */
} KdMonitorTiming;

extern const KdMonitorTiming	kdMonitorTimings[];
extern const int		kdNumMonitorTimings;

typedef struct _KdMouseMatrix {
    int	    matrix[2][3];
} KdMouseMatrix;

typedef struct _KaaScreenPriv {
    Bool	(*PrepareSolid) (DrawablePtr	pDrawable,
				 int		alu,
				 Pixel		planemask,
				 Pixel		fg);
    void	(*Solid) (int x1, int y1, int x2, int y2);
    void	(*DoneSolid) (void);

    Bool	(*PrepareCopy) (DrawablePtr	pSrcDrawable,
				DrawablePtr	pDstDrawable,
				Bool		upsidedown,
				Bool		reverse,
				int		alu,
				Pixel		planemask);
    void	(*Copy) (int	srcX,
			 int	srcY,
			 int	dstX,
			 int	dstY,
			 int	width,
			 int	height);
    void	(*DoneCopy) (void);
} KaaScreenPrivRec, *KaaScreenPrivPtr;

Bool
KaaInit (ScreenPtr	    pScreen,
	 KaaScreenPrivPtr   pScreenPriv);

/*
 * This is the only completely portable way to
 * compute this info.
 */

#ifndef BitsPerPixel
#define BitsPerPixel(d) (\
    PixmapWidthPaddingInfo[d].notPower2 ? \
    (PixmapWidthPaddingInfo[d].bytesPerPixel * 8) : \
    ((1 << PixmapWidthPaddingInfo[d].padBytesLog2) * 8 / \
    (PixmapWidthPaddingInfo[d].padRoundUp+1)))
#endif

extern int		kdScreenPrivateIndex;
extern unsigned long	kdGeneration;
extern Bool		kdEnabled;
extern Bool		kdSwitchPending;
extern Bool		kdEmulateMiddleButton;
extern Bool		kdDisableZaphod;
extern int		kdVirtualTerminal;
extern KdOsFuncs	*kdOsFuncs;

#define KdGetScreenPriv(pScreen) ((KdPrivScreenPtr) \
				  (pScreen)->devPrivates[kdScreenPrivateIndex].ptr)
#define KdSetScreenPriv(pScreen,v) ((pScreen)->devPrivates[kdScreenPrivateIndex].ptr = \
				    (pointer) v)
#define KdScreenPriv(pScreen) KdPrivScreenPtr pScreenPriv = KdGetScreenPriv(pScreen)

#define KdCheckSync(s)	{ \
    KdScreenPriv(s); \
    KdCardInfo	*card = pScreenPriv->card; \
    if (card->needSync) { \
	card->needSync = FALSE; \
	(*card->cfuncs->syncAccel) (s); \
    } \
}

#define KdMarkSync(s)	(KdGetScreenPriv(s)->card->needSync = TRUE)

/* kaa.c */
Bool
kaaDrawInit (ScreenPtr		pScreen,
	     KaaScreenPrivPtr	pScreenPriv);

void
kaaWrapGC (GCPtr pGC);

void
kaaUnwrapGC (GCPtr pGC);

/* kasync.c */
void
KdCheckFillSpans  (DrawablePtr pDrawable, GCPtr pGC, int nspans,
		   DDXPointPtr ppt, int *pwidth, int fSorted);

void
KdCheckSetSpans (DrawablePtr pDrawable, GCPtr pGC, char *psrc,
		 DDXPointPtr ppt, int *pwidth, int nspans, int fSorted);

void
KdCheckPutImage (DrawablePtr pDrawable, GCPtr pGC, int depth,
		 int x, int y, int w, int h, int leftPad, int format,
		 char *bits);

RegionPtr
KdCheckCopyArea (DrawablePtr pSrc, DrawablePtr pDst, GCPtr pGC,
		 int srcx, int srcy, int w, int h, int dstx, int dsty);

RegionPtr
KdCheckCopyPlane (DrawablePtr pSrc, DrawablePtr pDst, GCPtr pGC,
		  int srcx, int srcy, int w, int h, int dstx, int dsty,
		  unsigned long bitPlane);

void
KdCheckPolyPoint (DrawablePtr pDrawable, GCPtr pGC, int mode, int npt,
		  DDXPointPtr pptInit);

void
KdCheckPolylines (DrawablePtr pDrawable, GCPtr pGC,
		  int mode, int npt, DDXPointPtr ppt);

void
KdCheckPolySegment (DrawablePtr pDrawable, GCPtr pGC,
		    int nsegInit, xSegment *pSegInit);

void
KdCheckPolyRectangle (DrawablePtr pDrawable, GCPtr pGC, 
		      int nrects, xRectangle *prect);

void
KdCheckPolyArc (DrawablePtr pDrawable, GCPtr pGC, 
		int narcs, xArc *pArcs);

#define KdCheckFillPolygon	miFillPolygon

void
KdCheckPolyFillRect (DrawablePtr pDrawable, GCPtr pGC,
		     int nrect, xRectangle *prect);

void
KdCheckPolyFillArc (DrawablePtr pDrawable, GCPtr pGC, 
		    int narcs, xArc *pArcs);

void
KdCheckImageGlyphBlt (DrawablePtr pDrawable, GCPtr pGC,
		      int x, int y, unsigned int nglyph,
		      CharInfoPtr *ppci, pointer pglyphBase);

void
KdCheckPolyGlyphBlt (DrawablePtr pDrawable, GCPtr pGC,
		     int x, int y, unsigned int nglyph,
		     CharInfoPtr *ppci, pointer pglyphBase);

void
KdCheckPushPixels (GCPtr pGC, PixmapPtr pBitmap,
		   DrawablePtr pDrawable,
		   int w, int h, int x, int y);

void
KdCheckGetImage (DrawablePtr pDrawable,
		 int x, int y, int w, int h,
		 unsigned int format, unsigned long planeMask,
		 char *d);

void
KdCheckGetSpans (DrawablePtr pDrawable,
		 int wMax,
		 DDXPointPtr ppt,
		 int *pwidth,
		 int nspans,
		 char *pdstStart);

void
KdCheckSaveAreas (PixmapPtr	pPixmap,
		  RegionPtr	prgnSave,
		  int		xorg,
		  int		yorg,
		  WindowPtr	pWin);

void
KdCheckRestoreAreas (PixmapPtr	pPixmap,
		     RegionPtr	prgnSave,
		     int	xorg,
		     int    	yorg,
		     WindowPtr	pWin);

void
KdCheckCopyWindow (WindowPtr pWin, DDXPointRec ptOldOrg, RegionPtr prgnSrc);

void
KdCheckPaintKey(DrawablePtr  pDrawable,
		RegionPtr    pRegion,
		CARD32       pixel,
		int          layer);

void
KdCheckOverlayCopyWindow  (WindowPtr pWin, DDXPointRec ptOldOrg, RegionPtr prgnSrc);

void
KdScreenInitAsync (ScreenPtr pScreen);

void
KdCheckPaintWindow (WindowPtr pWin, RegionPtr pRegion, int what);
    
extern const GCOps	kdAsyncPixmapGCOps;

/* knoop.c */
extern GCOps		kdNoopOps;

/* kcmap.c */
void
KdSetColormap (ScreenPtr pScreen, int fb);

void
KdEnableColormap (ScreenPtr pScreen);

void
KdDisableColormap (ScreenPtr pScreen);

void
KdInstallColormap (ColormapPtr pCmap);

void
KdUninstallColormap (ColormapPtr pCmap);

int
KdListInstalledColormaps (ScreenPtr pScreen, Colormap *pCmaps);

void
KdStoreColors (ColormapPtr pCmap, int ndef, xColorItem *pdefs);

/* kcurscol.c */
void
KdAllocateCursorPixels (ScreenPtr	pScreen,
			int		fb,
			CursorPtr	pCursor, 
			Pixel		*source,
			Pixel		*mask);

int
KdComputeCmapShift (unsigned long mask);


/* tinyx.c */
extern miPointerScreenFuncRec kdPointerScreenFuncs;

void
KdSetRootClip (ScreenPtr pScreen, BOOL enable);

void
KdDisableScreen (ScreenPtr pScreen);

void
KdDisableScreens (void);

Bool
KdEnableScreen (ScreenPtr pScreen);

void
KdEnableScreens (void);

void
KdSuspend (void);

void
KdResume (void);

void
KdProcessSwitch (void);

Rotation
KdAddRotation (Rotation a, Rotation b);

Rotation
KdSubRotation (Rotation a, Rotation b);

void
KdParseScreen (KdScreenInfo *screen,
	       const char    *arg);

char *
KdSaveString (char *str);

void
KdOsInit (KdOsFuncs *pOsFuncs);

Bool
KdAllocatePrivates (ScreenPtr pScreen);

Bool
KdCloseScreen (int index, ScreenPtr pScreen);

Bool
KdSaveScreen (ScreenPtr pScreen, int on);

Bool
KdScreenInit(int index, ScreenPtr pScreen, int argc, const char **argv);

void
KdInitScreen (ScreenInfo    *pScreenInfo,
	      KdScreenInfo  *screen,
	      int	    argc,
	      const char    **argv);

void
KdInitCard (ScreenInfo	    *pScreenInfo,
	    KdCardInfo	    *card,
	    int		    argc,
	    const char	    **argv);

void
KdInitOutput (ScreenInfo    *pScreenInfo,
	      int	    argc,
	      const char    **argv);

void
KdSetSubpixelOrder (ScreenPtr pScreen, Rotation randr);

Bool
KdCreateWindow (WindowPtr pWin);

int
KdProcessArgument (int argc, const char **argv, int i);

void
KdUseMsg (void);

    
/* kinfo.c */
KdCardInfo *
KdCardInfoAdd (KdCardFuncs  *funcs,
	       KdCardAttr   *attr,
	       void	    *closure);

KdCardInfo *
KdCardInfoLast (void);

void
KdCardInfoDispose (KdCardInfo *ci);

KdScreenInfo *
KdScreenInfoAdd (KdCardInfo *ci);

void
KdScreenInfoDispose (KdScreenInfo *si);

void
KdMouseInfoDispose (KdMouseInfo *mi);


/* kinput.c */
void
KdInitInput(KdMouseFuncs *, KdKeyboardFuncs *);

int
KdAllocInputType (void);

Bool
KdRegisterFd (int type, int fd, void (*read) (int fd, void *closure), void *closure);

void
KdRegisterFdEnableDisable (int fd, 
			   int (*enable) (int fd, void *closure),
			   void (*disable) (int fd, void *closure));

void
KdUnregisterFds (int type, Bool do_close);

#ifdef TOUCHSCREEN
void
KdInitTouchScreen(KdMouseFuncs *pTsFuncs);
#endif

void
KdEnqueueKeyboardEvent(unsigned char	scan_code,
		       unsigned char	is_up);

#define KD_BUTTON_1	0x01
#define KD_BUTTON_2	0x02
#define KD_BUTTON_3	0x04
#define KD_BUTTON_4	0x08
#define KD_BUTTON_5	0x10
#define KD_MOUSE_DELTA	0x80000000

void
KdEnqueueMouseEvent(KdMouseInfo *mi, unsigned long flags, int x, int y);

void
KdEnqueueMotionEvent (KdMouseInfo *mi, int x, int y);

void
KdReleaseAllKeys (void);
    
void
KdSetLed (int led, Bool on);

void
KdSetMouseMatrix (KdMouseMatrix *matrix);

void
KdComputeMouseMatrix (KdMouseMatrix *matrix, Rotation randr, int width, int height);
    
void
KdBlockHandler (int		screen,
		pointer		blockData,
		pointer		timeout,
		pointer		readmask);

void
KdWakeupHandler (int		screen, 
		 pointer    	data,
		 unsigned long	result,
		 pointer	readmask);

void
KdDisableInput (void);

void
KdEnableInput (void);

extern KdMouseFuncs	LinuxMouseFuncs;
extern KdMouseFuncs	Ps2MouseFuncs;
extern KdMouseFuncs	BusMouseFuncs;
extern KdMouseFuncs	MsMouseFuncs;
#ifdef TOUCHSCREEN
extern KdMouseFuncs	TsFuncs;
#endif
extern KdKeyboardFuncs	LinuxKeyboardFuncs;
extern KdOsFuncs	LinuxFuncs;

extern KdMouseFuncs	VxWorksMouseFuncs;
extern KdKeyboardFuncs	VxWorksKeyboardFuncs;
extern KdOsFuncs	VxWorksFuncs;

/* kmap.c */

#define KD_MAPPED_MODE_REGISTERS    0
#define KD_MAPPED_MODE_FRAMEBUFFER  1

void *
KdMapDevice (CARD32 addr, CARD32 size);

void
KdUnmapDevice (void *addr, CARD32 size);

void
KdSetMappedMode (CARD32 addr, CARD32 size, int mode);

void
KdResetMappedMode (CARD32 addr, CARD32 size, int mode);

/* kmode.c */
const KdMonitorTiming *
KdFindMode (KdScreenInfo    *screen,
	    Bool	    (*supported) (KdScreenInfo *,
					  const KdMonitorTiming *));

Bool
KdTuneMode (KdScreenInfo    *screen,
	    Bool	    (*usable) (KdScreenInfo *),
	    Bool	    (*supported) (KdScreenInfo *,
					  const KdMonitorTiming *));

#ifdef RANDR
Bool
KdRandRGetInfo (ScreenPtr pScreen, 
		int randr,
		Bool (*supported) (ScreenPtr pScreen, 
				   const KdMonitorTiming *));

const KdMonitorTiming *
KdRandRGetTiming (ScreenPtr	    pScreen,
		  Bool		    (*supported) (ScreenPtr pScreen, 
						  const KdMonitorTiming *),
		  int		    rate,
		  RRScreenSizePtr   pSize);
#endif

/* kpict.c */
void
KdCheckComposite (CARD8 op, PicturePtr pSrc, PicturePtr pMask, PicturePtr pDst,
		  INT16 xSrc, INT16 ySrc, INT16 xMask, INT16 yMask,
		  INT16 xDst, INT16 yDst, CARD16 width, CARD16 height);

void
KdPictureInitAsync (ScreenPtr pScreen);

/* kshadow.c */
Bool
KdShadowScreenInit (KdScreenInfo *screen);

Bool
KdShadowInitScreen (ScreenPtr pScreen, ShadowUpdateProc update, ShadowWindowProc window);

void
KdShadowScreenFini (KdScreenInfo *screen);

/* ktest.c */
Bool
KdFrameBufferValid (CARD8 *base, int size);

int
KdFrameBufferSize (CARD8 *base, int max);

/* function prototypes to be imlpemented by the drivers */
void
InitCard (const char *name);