/* $XFree86: xc/programs/Xserver/hw/xfree86/dummylib/pcitestmulti.c,v 1.2 2005/10/14 15:16:50 tsi Exp $ */

#include <X11/X.h>
#include "os.h"
#include "xf86.h"
#include "xf86Priv.h"

/*
 * Utility functions required by libxf86_os. 
 */

int
pciTestMultiDeviceCard(int bus, int dev, int func, PCITAG** pTag)
{
    return 0;
}

