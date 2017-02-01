/* $XFree86: xc/programs/Xserver/ilbm/ilbmsetsp.c,v 3.3 2006/01/09 15:00:32 dawes Exp $ */
/* Combined Purdue/PurduePlus patches, level 2.0, 1/17/89 */
/***********************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* Modified jun 95 by Geert Uytterhoeven (Geert.Uytterhoeven@cs.kuleuven.ac.be)
   to use interleaved bitplanes instead of normal bitplanes */

#include <X11/X.h>
#include <X11/Xmd.h>

#include "misc.h"
#include "regionstr.h"
#include "gcstruct.h"
#include "windowstr.h"
#include "pixmapstr.h"
#include "scrnintstr.h"

#include "ilbm.h"
#include "maskbits.h"

#include "servermd.h"


/* ilbmSetScanline -- copies the bits from psrc to the drawable starting at
 * (xStart, y) and continuing to (xEnd, y).  xOrigin tells us where psrc
 * starts on the scanline. (I.e., if this scanline passes through multiple
 * boxes, we may not want to start grabbing bits at psrc but at some offset
 * further on.)
 */
ilbmSetScanline(y, xOrigin, xStart, xEnd, psrc, alu, pdstBase, widthDst, auxDst,
					 depthDst, widthSrc)
	int y;
	int xOrigin;					/* where this scanline starts */
	int xStart;						/* first bit to use from scanline */
	int xEnd;						/* last bit to use from scanline + 1 */
	register PixelType *psrc;
	register int alu;				/* raster op */
	PixelType *pdstBase;			/* start of the drawable */
	int widthDst;					/* width of drawable in words */
	int auxDst;
	int depthDst;
	int widthSrc;					/* width of drawable in words */
{
	int w;							/* width of scanline in bits */
	register PixelType *pdst;	/* where to put the bits */
	register PixelType tmpSrc;	/* scratch buffer to collect bits in */
	int dstBit;						/* offset in bits from beginning of
										 * word */
	register int nstart; 		/* number of bits from first partial */
	register int nend; 			/* " " last partial word */
	int offSrc;
	PixelType startmask, endmask;
	PixelType *savePsrc = psrc + ((xStart - xOrigin) >> PWSH);
	int nlMiddle, nl;
	int d;

	for (d = 0; d < depthDst; d++) {
		pdst = ilbmScanline(pdstBase, xStart, y, auxDst) + widthDst * d;	/* @@@ NEXT PLANE @@@ */
		psrc = savePsrc + widthSrc * d;	/* @@@ NEXT PLANE @@@ */
		offSrc = (xStart - xOrigin) & PIM;
		w = xEnd - xStart;
		dstBit = xStart & PIM;

		if (dstBit + w <= PPW) {
			getandputrop(psrc, offSrc, dstBit, w, pdst, alu)
		} else {
			maskbits(xStart, w, startmask, endmask, nlMiddle);
			if (startmask)
				nstart = PPW - dstBit;
			else
				nstart = 0;
			if (endmask)
				nend = xEnd & PIM;
			else
				nend = 0;
			if (startmask) {
				getandputrop(psrc, offSrc, dstBit, nstart, pdst, alu)
				pdst++;
				offSrc += nstart;
				if (offSrc > PLST) {
					psrc++;
					offSrc -= PPW;
				}
			}
			nl = nlMiddle;
			while (nl--) {
				getbits(psrc, offSrc, PPW, tmpSrc);
				DoRop(*pdst, alu, tmpSrc, *pdst);
				pdst++;
				psrc++;
			}
			if (endmask) {
				getandputrop0(psrc, offSrc, nend, pdst, alu);
			}
		}
	}
}

/* SetSpans -- for each span copy pwidth[i] bits from psrc to pDrawable at
 * ppt[i] using the raster op from the GC.  If fSorted is TRUE, the scanlines
 * are in increasing Y order.
 * Source bit lines are server scanline padded so that they always begin
 * on a word boundary.
 */
void
ilbmSetSpans(pDrawable, pGC, pcharsrc, ppt, pwidth, nspans, fSorted)
	DrawablePtr pDrawable;
	GCPtr pGC;
	char *pcharsrc;
	register DDXPointPtr ppt;
	int *pwidth;
	int nspans;
	int fSorted;
{
	PixelType *psrc = (PixelType *)pcharsrc;
	PixelType *pdstBase;				/* start of dst bitmap */
	int widthDst;						/* width of bitmap in words */
	int auxDst;
	int depthDst;
	int widthSrc;
	register BoxPtr pbox, pboxLast, pboxTest;
	register DDXPointPtr pptLast;
	int alu;
	RegionPtr prgnDst;
	int xStart, xEnd;
	int yMax;

	alu = pGC->alu;
	prgnDst = pGC->pCompositeClip;

	pptLast = ppt + nspans;

	yMax = pDrawable->y + (int) pDrawable->height;
	ilbmGetPixelWidthAuxDepthAndPointer(pDrawable, widthDst, auxDst, depthDst,
													pdstBase);

	pbox =  REGION_RECTS(prgnDst);
	pboxLast = pbox + REGION_NUM_RECTS(prgnDst);

	if (fSorted) {
	/* scan lines sorted in ascending order. Because they are sorted, we
	 * don't have to check each scanline against each clip box.  We can be
	 * sure that this scanline only has to be clipped to boxes at or after the
	 * beginning of this y-band
	 */
		pboxTest = pbox;
		while (ppt < pptLast) {
			pbox = pboxTest;
			if (ppt->y >= yMax)
				break;
			while (pbox < pboxLast) {
				if (pbox->y1 > ppt->y) {
					/* scanline is before clip box */
					break;
				} else if (pbox->y2 <= ppt->y) {
					/* clip box is before scanline */
					pboxTest = ++pbox;
					continue;
				} else if (pbox->x1 > ppt->x + *pwidth)  {
					/* clip box is to right of scanline */
					break;
				} else if (pbox->x2 <= ppt->x) {
					/* scanline is to right of clip box */
					pbox++;
					continue;
				}

				/* at least some of the scanline is in the current clip box */
				xStart = max(pbox->x1, ppt->x);
				xEnd = min(ppt->x + *pwidth, pbox->x2);
				widthSrc = PixmapWidthInPadUnits(*pwidth, 1);
				ilbmSetScanline(ppt->y, ppt->x, xStart, xEnd, psrc, alu, pdstBase,
									 widthDst, auxDst, depthDst, widthSrc);
				if (ppt->x + *pwidth <= pbox->x2) {
					/* End of the line, as it were */
					break;
				} else
					pbox++;
			}
			/* We've tried this line against every box; it must be outside them
			 * all.  move on to the next point */
			ppt++;
			psrc += widthSrc * depthDst;
			pwidth++;
		}
	} else {
		/* scan lines not sorted. We must clip each line against all the boxes */
		while (ppt < pptLast) {
			if (ppt->y >= 0 && ppt->y < yMax) {
				for (pbox = REGION_RECTS(prgnDst); pbox< pboxLast; pbox++) {
					if (pbox->y1 > ppt->y) {
						/* rest of clip region is above this scanline,
						 * skip it */
						break;
					}
					if (pbox->y2 <= ppt->y) {
						/* clip box is below scanline */
						pbox++;
						break;
					}
					if (pbox->x1 <= ppt->x + *pwidth &&
					   pbox->x2 > ppt->x) {
						xStart = max(pbox->x1, ppt->x);
						xEnd = min(pbox->x2, ppt->x + *pwidth);
						widthSrc = PixmapWidthInPadUnits(*pwidth, 1);
						ilbmSetScanline(ppt->y, ppt->x, xStart, xEnd, psrc, alu,
											 pdstBase, widthDst, auxDst, depthDst,
											 widthSrc);
					}

				}
			}
		psrc += widthSrc * depthDst;
		ppt++;
		pwidth++;
		}
	}
}
