/* $XFree86: xc/programs/Xserver/hw/dmx/dmxscrinit.h,v 1.3 2007/01/23 18:02:57 tsi Exp $ */
/*
 * Copyright 2001-2004 Red Hat Inc., Durham, North Carolina.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL RED HAT AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * Authors:
 *   Kevin E. Martin <kem@redhat.com>
 *   David H. Dawes <dawes@xfree86.org>
 *
 */

/** \file
 * Interface for screen initialization.  \see dmxscrinit.c */

#ifndef DMXSCRINIT_H
#define DMXSCRINIT_H

#include "scrnintstr.h"

/** Private index.  \see dmxscrrinit.c \see input/dmxconcole.c */
extern int dmxScreenPrivateIndex;

extern Bool dmxScreenInit(int idx, ScreenPtr pScreen, const int argc, const char *argv[]);

extern void dmxBEScreenInit(int idx, ScreenPtr pScreen);
extern void dmxBECloseScreen(ScreenPtr pScreen);

#endif /* DMXSCRINIT_H */
