/* $XFree86: xc/lib/font/include/fontutil.h,v 1.3 2005/10/14 15:16:06 tsi Exp $ */

#ifndef _FONTUTIL_H_
#define _FONTUTIL_H_

#include <X11/fonts/FSproto.h>

extern int FontCouldBeTerminal(FontInfoPtr);
extern int CheckFSFormat(fsBitmapFormat, fsBitmapFormatMask, int *, int *,
			 int *, int *, int *);
extern void FontComputeInfoAccelerators(FontInfoPtr);

extern void GetGlyphs ( FontPtr font, unsigned long count, 
			unsigned char *chars, FontEncoding fontEncoding, 
			unsigned long *glyphcount, CharInfoPtr *glyphs );
extern void QueryGlyphExtents ( FontPtr pFont, CharInfoPtr *charinfo, 
				unsigned long count, ExtentInfoRec *info );
extern Bool QueryTextExtents ( FontPtr pFont, unsigned long count, 
			       unsigned char *chars, ExtentInfoRec *info );
extern Bool ParseGlyphCachingMode ( char *str );
extern void InitGlyphCaching ( void );
extern void SetGlyphCachingMode ( int newmode );
extern int add_range ( fsRange *newrange, int *nranges, fsRange **range, 
		       Bool charset_subset );

#endif /* _FONTUTIL_H_ */
