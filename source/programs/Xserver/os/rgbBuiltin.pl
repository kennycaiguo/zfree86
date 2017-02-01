#!/usr/bin/perl
#
# $XFree86: xc/programs/Xserver/os/rgbBuiltin.pl,v 1.3 2005/06/22 03:42:13 dawes Exp $
#
# Read rgb.txt on stdin and write a hashed C source file to stdout.
# The hashing algorithm must match that in oscolor.c.
#
#
# Copyright � 2005 by The XFree86 Project, Inc.
# All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject
# to the following conditions:
#
#   1.  Redistributions of source code must retain the above copyright
#       notice, this list of conditions, and the following disclaimer.
#
#   2.  Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer
#       in the documentation and/or other materials provided with the
#       distribution, and in the same place and form as other copyright,
#       license and disclaimer information.
#
#   3.  The end-user documentation included with the redistribution,
#       if any, must include the following acknowledgment: "This product
#       includes software developed by The XFree86 Project, Inc
#       (http://www.xfree86.org/) and its contributors", in the same
#       place and form as other third-party acknowledgments.  Alternately,
#       this acknowledgment may appear in the software itself, in the
#       same form and location as other such third-party acknowledgments.
#
#   4.  Except as contained in this notice, the name of The XFree86
#       Project, Inc shall not be used in advertising or otherwise to
#       promote the sale, use or other dealings in this Software without
#       prior written authorization from The XFree86 Project, Inc.
#
# THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE XFREE86 PROJECT, INC OR ITS CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
# OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
# OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Author: David Dawes
#


$HASHSIZE = 511;
$tmp = 1;

sub hash {
	my ($name) = @_;
	@chars, $ch, $h;

	@chars = split(//, $name);
	$h = 0;
	for $ch (@chars) {
		$h = (($h << 4) + ord($ch)) & 0xffffffff;
		$g = $h;
		if ($h & 0xf0000000) {
			$h = ($h ^ ($g >> 24));
		}
		if ($h != $g) {
			$h = ($h & $g);
		}
	}
	# Needed to workound a bug in some perl versions.
	$h = int($h);
	$h = $h % $HASHSIZE;
	return $h;
}

while(<>) {
	$line = $_;
	if ($line =~ /^[!#]/) {
		next;
	}
	if ($line =~ /^\s*(\d+)\s+(\d+)\s+(\d+)\s+(\S.*\S)\s*$/) {
		$red = $1;
		$green = $2;
		$blue = $3;
		$name = lc($4);
		$hash = &hash($name);
		$hashEntry{$name} = [ $red, $green, $blue ];
		push @{$hashTab[$hash]}, $name;
	}
}

$proj = "XFree86";

print "/* \$$proj\$ */

/*
 * THIS FILE IS AUTOMATICALLY GENERATED -- DO NOT EDIT
 *
 * It is generated by rgbBuiltin.pl using data from the following file:
 *
 *    ../../rgb/rgb.txt
 *
 * Copyright � 2005 by The XFree86 Project, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * \"Software\"), to deal in the Software without restriction, including
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
 *       if any, must include the following acknowledgment: \"This product
 *       includes software developed by The XFree86 Project, Inc
 *       (http://www.xfree86.org/) and its contributors\", in the same
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
 *
 */

#include <stdio.h>
#include \"oscolor.h\"

#define C 0x101\n\n";

for $i (0 .. $HASHSIZE - 1) {
	if (defined($hashTab[$i])) {
		$link = "NULL";
		$j = 0;
		for $n (@{$hashTab[$i]}) {
			print "static const char c${i}_${j}[] = \"$n\";\n";
			print "static dbEntry e${i}_$j = " .
				"{$link, $hashEntry{$n}[0] * C," .
				" $hashEntry{$n}[1] * C," .
				" $hashEntry{$n}[2] * C," .
				" c${i}_$j};\n";
			$link = "&e${i}_$j";
			$entrySize[$i] = $j;
			$j++;
		}
	}
}
print "\n#undef C\n\n";
print "dbEntryPtr builtinRGBhashTab[HASHSIZE] = {\n";
for $i (0 .. $HASHSIZE - 1) {
	if (defined($hashTab[$i])) {
		print "    &e${i}_$entrySize[$i],\n";
	} else {
		print "    NULL,\n";
	}
}
print "};\n";


