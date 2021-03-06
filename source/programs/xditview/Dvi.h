/* $XFree86: xc/programs/xditview/Dvi.h,v 1.4 2006/01/09 15:01:02 dawes Exp $ */

#ifndef _XtDvi_h
#define _XtDvi_h

/***********************************************************************
 *
 * Dvi Widget
 *
 ***********************************************************************/

/* Parameters:

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------
 background	     Background		pixel		White
 foreground	     Foreground		Pixel		Black
 fontMap	     FontMap		char *		...
 pageNumber	     PageNumber		int		1
*/

#define XtNfontMap	"fontMap"
#define XtNpageNumber	"pageNumber"
#define XtNlastPageNumber   "lastPageNumber"
#define XtNnoPolyText	"noPolyText"
#define XtNseek		"seek"
#define XtNscreenResolution "screenResolution"
#define XtNpageWidth	"pageWidth"
#define XtNpageHeight	"pageHeight"
#define XtNsizeScale	"sizeScale"

#define XtCFontMap	"FontMap"
#define XtCPageNumber	"PageNumber"
#define XtCLastPageNumber   "LastPageNumber"
#define XtCNoPolyText	"NoPolyText"
#define XtCSeek		"Seek"
#define XtCScreenResolution "ScreenResolution"
#define XtCPageWidth	"PageWidth"
#define XtCPageHeight	"PageHeight"
#define XtCSizeScale	"SizeScale"

typedef struct _DviRec *DviWidget;  /* completely defined in DviPrivate.h */
typedef struct _DviClassRec *DviWidgetClass;    /* completely defined in DviPrivate.h */

extern WidgetClass dviWidgetClass;

#endif /* _XtDvi_h */
/* DON'T ADD STUFF AFTER THIS #endif */
