/* $XFree86: xc/programs/Xserver/hw/xfree86/parser/Keyboard.c,v 1.21 2006/08/09 20:53:16 dawes Exp $ */
/* 
 * 
 * Copyright (c) 1997  Metro Link Incorporated
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of the Metro Link shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from Metro Link.
 * 
 */
/*
 * Copyright (c) 1997-2006 by The XFree86 Project, Inc.
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


/* View/edit this file with tab stops set to 4 */

#include "xf86Parser.h"
#include "xf86tokens.h"
#include "Configint.h"
#include "ctype.h"

extern LexRec val;

static xf86ConfigSymTabRec KeyboardTab[] =
{
	{ENDSECTION, "endsection"},
	{KPROTOCOL, "protocol"},
	{AUTOREPEAT, "autorepeat"},
	{XLEDS, "xleds"},
	{PANIX106, "panix106"},
	{XKBKEYMAP, "xkbkeymap"},
	{XKBCOMPAT, "xkbcompat"},
	{XKBTYPES, "xkbtypes"},
	{XKBKEYCODES, "xkbkeycodes"},
	{XKBGEOMETRY, "xkbgeometry"},
	{XKBSYMBOLS, "xkbsymbols"},
	{XKBDISABLE, "xkbdisable"},
	{XKBRULES, "xkbrules"},
	{XKBMODEL, "xkbmodel"},
	{XKBLAYOUT, "xkblayout"},
	{XKBVARIANT, "xkbvariant"},
	{XKBOPTIONS, "xkboptions"},
	/* The next two have become ServerFlags options */
	{VTINIT, "vtinit"},
	{VTSYSREQ, "vtsysreq"},
	/* Obsolete keywords */
	{SERVERNUM, "servernumlock"},
	{LEFTALT, "leftalt"},
	{RIGHTALT, "rightalt"},
	{RIGHTALT, "altgr"},
	{SCROLLLOCK_TOK, "scrolllock"},
	{RIGHTCTL, "rightctl"},
	{-1, ""},
};

#define CLEANUP xf86freeInputList

XF86ConfInputPtr
xf86parseKeyboardSection (void)
{
	char *s = NULL, *s1;
	int n1, n2, l;
	int token;
	parsePrologue (XF86ConfInputPtr, XF86ConfInputRec)

	while ((token = xf86getToken (KeyboardTab)) != ENDSECTION)
	{
		switch (token)
		{
		case COMMENT:
			ptr->inp_comment = xf86addComment(ptr->inp_comment, val.str);
			break;
		case KPROTOCOL:
			if (xf86getSubToken (&(ptr->inp_comment)) != STRING)
				Error (QUOTE_MSG, "Protocol");
			ptr->inp_option_lst = xf86addNewOption(ptr->inp_option_lst,
													"Protocol", val.str);
			break;
		case AUTOREPEAT:
			if (xf86getSubToken (&(ptr->inp_comment)) != NUMBER)
				Error (AUTOREPEAT_MSG, NULL);
			n1 = val.num;
			if (xf86getSubToken (&(ptr->inp_comment)) != NUMBER)
				Error (AUTOREPEAT_MSG, NULL);
			n2 = val.num;
			xf86configAsprintf(&s, "%d %d", n1, n2);
			if (s) {
				ptr->inp_option_lst = xf86addNewOption(ptr->inp_option_lst,
														"AutoRepeat", s);
				xf86conffree(s);
			}
			break;
		case XLEDS:
			if (xf86getSubToken (&(ptr->inp_comment)) != NUMBER)
				Error (XLEDS_MSG, NULL);
			s = xf86uLongToString(val.num);
			l = strlen(s) + 1;
			while ((token = xf86getSubToken (&(ptr->inp_comment))) == NUMBER)
			{
				s1 = xf86uLongToString(val.num);
				l += (1 + strlen(s1));
				s = xf86confrealloc(s, l);
				strcat(s, " ");
				strcat(s, s1);
				xf86conffree(s1);
			}
			xf86unGetToken (token);
			if (s) {
				ptr->inp_option_lst = xf86addNewOption(ptr->inp_option_lst,
														"xleds", s);
				xf86conffree(s);
			}
			break;
		case SERVERNUM:
			xf86parseWarning(OBSOLETE_MSG, xf86tokenString());
			break;
		case LEFTALT:
		case RIGHTALT:
		case SCROLLLOCK_TOK:
		case RIGHTCTL:
			xf86parseWarning(OBSOLETE_MSG, xf86tokenString());
			break;
		case VTINIT:
			if (xf86getSubToken (&(ptr->inp_comment)) != STRING)
				Error (QUOTE_MSG, "VTInit");
			xf86parseWarning(MOVED_TO_FLAGS_MSG, "VTInit");
			break;
		case VTSYSREQ:
			xf86parseWarning(MOVED_TO_FLAGS_MSG, "VTSysReq");
			break;
		case XKBDISABLE:
			ptr->inp_option_lst = xf86addNewOption(ptr->inp_option_lst,
												"XkbDisable", NULL);
			break;
		case XKBKEYMAP:
			if (xf86getSubToken (&(ptr->inp_comment)) != STRING)
				Error (QUOTE_MSG, "XKBKeymap");
			ptr->inp_option_lst = xf86addNewOption(ptr->inp_option_lst,
												"XkbKeymap", val.str);
			break;
		case XKBCOMPAT:
			if (xf86getSubToken (&(ptr->inp_comment)) != STRING)
				Error (QUOTE_MSG, "XKBCompat");
			ptr->inp_option_lst = xf86addNewOption(ptr->inp_option_lst,
												"XkbCompat", val.str);
			break;
		case XKBTYPES:
			if (xf86getSubToken (&(ptr->inp_comment)) != STRING)
				Error (QUOTE_MSG, "XKBTypes");
			ptr->inp_option_lst = xf86addNewOption(ptr->inp_option_lst,
												"XkbTypes", val.str);
			break;
		case XKBKEYCODES:
			if (xf86getSubToken (&(ptr->inp_comment)) != STRING)
				Error (QUOTE_MSG, "XKBKeycodes");
			ptr->inp_option_lst = xf86addNewOption(ptr->inp_option_lst,
												"XkbKeycodes", val.str);
			break;
		case XKBGEOMETRY:
			if (xf86getSubToken (&(ptr->inp_comment)) != STRING)
				Error (QUOTE_MSG, "XKBGeometry");
			ptr->inp_option_lst = xf86addNewOption(ptr->inp_option_lst,
												"XkbGeometry", val.str);
			break;
		case XKBSYMBOLS:
			if (xf86getSubToken (&(ptr->inp_comment)) != STRING)
				Error (QUOTE_MSG, "XKBSymbols");
			ptr->inp_option_lst = xf86addNewOption(ptr->inp_option_lst,
												"XkbSymbols", val.str);
			break;
		case XKBRULES:
			if (xf86getSubToken (&(ptr->inp_comment)) != STRING)
				Error (QUOTE_MSG, "XKBRules");
			ptr->inp_option_lst = xf86addNewOption(ptr->inp_option_lst,
												"XkbRules", val.str);
			break;
		case XKBMODEL:
			if (xf86getSubToken (&(ptr->inp_comment)) != STRING)
				Error (QUOTE_MSG, "XKBModel");
			ptr->inp_option_lst = xf86addNewOption(ptr->inp_option_lst,
												"XkbModel", val.str);
			break;
		case XKBLAYOUT:
			if (xf86getSubToken (&(ptr->inp_comment)) != STRING)
				Error (QUOTE_MSG, "XKBLayout");
			ptr->inp_option_lst = xf86addNewOption(ptr->inp_option_lst,
												"XkbLayout", val.str);
			break;
		case XKBVARIANT:
			if (xf86getSubToken (&(ptr->inp_comment)) != STRING)
				Error (QUOTE_MSG, "XKBVariant");
			ptr->inp_option_lst = xf86addNewOption(ptr->inp_option_lst,
												"XkbVariant", val.str);
			break;
		case XKBOPTIONS:
			if (xf86getSubToken (&(ptr->inp_comment)) != STRING)
				Error (QUOTE_MSG, "XKBOptions");
			ptr->inp_option_lst = xf86addNewOption(ptr->inp_option_lst,
												"XkbOptions", val.str);
			break;
		case PANIX106:
			ptr->inp_option_lst = xf86addNewOption(ptr->inp_option_lst,
												"Panix106", NULL);
			break;
		case EOF_TOKEN:
			Error (UNEXPECTED_EOF_MSG, NULL);
			break;
		default:
			Error (INVALID_KEYWORD_MSG, xf86tokenString ());
			break;
		}
	}

	ptr->inp_identifier = xf86configStrdup(CONF_IMPLICIT_KEYBOARD);
	ptr->inp_driver = xf86configStrdup("keyboard");
	ptr->inp_option_lst = xf86addNewOption(ptr->inp_option_lst,
										"CoreKeyboard", NULL);

#ifdef DEBUG
	printf ("Keyboard section parsed\n");
#endif

	return ptr;
}

