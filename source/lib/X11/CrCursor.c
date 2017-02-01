/*

Copyright 1986, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/
/* $XFree86: xc/lib/X11/CrCursor.c,v 1.5 2006/01/09 14:58:24 dawes Exp $ */

#include "Xlibint.h"

#ifdef USE_DYNAMIC_XCURSOR
Cursor
_XTryShapeBitmapCursor (Display		*dpy,
			Pixmap		source,
			Pixmap		mask,
			XColor		*foreground,
			XColor		*background,
			unsigned int	x,
			unsigned int	y);
#endif
    
Cursor XCreatePixmapCursor(dpy, source, mask, foreground, background, x, y)
     register Display *dpy;
     Pixmap source, mask;
     XColor *foreground, *background;
     unsigned int  x, y;

{       
    register xCreateCursorReq *req;
    Cursor cid;

#ifdef USE_DYNAMIC_XCURSOR
    cid = _XTryShapeBitmapCursor (dpy, source, mask, 
				  foreground, background, x, y);
    if (cid)
	return cid;
#endif
    LockDisplay(dpy);
    GetReq(CreateCursor, req);
    req->cid = cid = XAllocID(dpy);
    req->source = source;
    req->mask = mask;
    req->foreRed = foreground->red;
    req->foreGreen = foreground->green;
    req->foreBlue = foreground->blue;
    req->backRed = background->red;
    req->backGreen = background->green;
    req->backBlue = background->blue;
    req->x = x;
    req->y = y;
    UnlockDisplay(dpy);
    SyncHandle();
    return (cid);
}

