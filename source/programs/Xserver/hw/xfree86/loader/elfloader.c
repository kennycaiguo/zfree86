/* $XFree86: xc/programs/Xserver/hw/xfree86/loader/elfloader.c,v 1.77 2007/04/09 15:37:17 tsi Exp $ */

/*
 *
 * Copyright 1995-1998 by Metro Link, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Metro Link, Inc. not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Metro Link, Inc. makes no
 * representations about the suitability of this software for any purpose.
 *  It is provided "as is" without express or implied warranty.
 *
 * METRO LINK, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL METRO LINK, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/*
 * Copyright (c) 1997-2007 by The XFree86 Project, Inc.
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

/*
 * Copyright 2003-2006 by David H. Dawes.
 * Copyright 2003-2006 by X-Oz Technologies.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions, and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *
 *  3. The end-user documentation included with the redistribution,
 *     if any, must include the following acknowledgment: "This product
 *     includes software developed by X-Oz Technologies
 *     (http://www.x-oz.com/)."  Alternately, this acknowledgment may
 *     appear in the software itself, if and wherever such third-party
 *     acknowledgments normally appear.
 *
 *  4. Except as contained in this notice, the name of X-Oz
 *     Technologies shall not be used in advertising or otherwise to
 *     promote the sale, use or other dealings in this Software without
 *     prior written authorization from X-Oz Technologies.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL X-OZ TECHNOLOGIES OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <sys/types.h>
#ifndef __UNIXOS2__
#include <sys/mman.h>
#endif
#include <unistd.h>
#include <stdlib.h>
#ifdef __QNX__
# include <fcntl.h>
#else
# include <sys/fcntl.h>
#endif
#include <sys/stat.h>
#if defined(linux) && defined(__ia64__)
#include <sys/mman.h>
#endif
#include <errno.h>

#ifdef DBMALLOC
# include <debug/malloc.h>
# define Xalloc(size) malloc(size)
# define Xcalloc(size) calloc(1,(size))
# define Xfree(size) free(size)
#endif

#include <X11/Xos.h>
#include "os.h"
#include "elf.h"
#include "sym.h"

#ifndef LOADERDEBUG
#define LOADERDEBUG 0
#endif
#include "loader.h"
#include "elfloader.h"

#include "compiler.h"

#ifndef EXESYMDEBUG
#define EXESYMDEBUG (0 && LOADERDEBUG)
#endif

#if defined(__ia64__)

/*
 * R_IA64_LTOFF22X and R_IA64_LDXMOV are relocation optimizations for
 * IA64. Conforming implementations must recognize them and may either
 * implement the optimization or may fallback to previous
 * non-optimized behavior by treating R_IA64_LTOFF22X as a
 * R_IA64_LTOFF22 and ignoring R_IA64_LDXMOV. The
 * IA64_LDX_OPTIMIZATION conditional controls the fallback behavior,
 * if defined the optimizations are performed.
 *
 * To implement the optimization we want to change is the sequence on
 * the left to that on the right, without regard to any intervening
 * instructions:
 *
 * 1)  addl    t1=@ltoff(var),gp    ==>    addl    t1=@gprel(var),gp
 * 2)  ld8     t2=[t1]              ==>    mov     t2=t1
 * 3)  ld8     loc0=[t2]            ==>    ld8     loc0=[t2]
 *
 * The relocations that match the above instructions are:
 *
 * 1)  R_IA64_LTOFF22               ==>    R_IA64_LTOFF22X
 * 2)  --                           ==>    R_IA64_LDXMOV
 * 3)  --                           ==>    --
 *
 * First lets look at left hand column to understand the original
 * mechanism. The virtual address of a symbol is stored in the GOT,
 * when that symbol is referenced the following sequence occurs,
 * instruction 1 loads the address of the GOT entry containing the
 * virtural address of the symbol into t1. Instruction 2 loads the
 * virtual address of the symbol into t2 by dereferencing t1. Finally
 * the symbol is loaded in instruction 3 by dereferencing its virtual
 * address in t2.
 *
 * The optimization that LTOFF22X/LDXMOV introduces is based on the
 * observation we are doing an extra load (instruction 2) if we can
 * generate the virtual address for the symbol without doing a lookup in
 * the GOT. This is possible if the virtual address of the symbol can be
 * computed via GP relative addressing. In other words the virtual
 * address of the symbol is a fixed offset from the GP. This fixed offset
 * must be within the limits of the signed 22 bit immediate offset in the
 * ld8 instruction, otherwise the original indirect GOT lookup must be
 * performed (LTOFF22).
 *
 * If we can use GP relative addressing for the symbol then the
 * instruction that loaded the virtual address of the symbol into t2 must
 * also be patched, hence the introduction of the LDXMOV relocation. The
 * LDXMOV essentially turns the GOT lookup into a no-op by changing the
 * ld8 into a register move that preserves the register location of the
 * symbol's virtual address (e.g. t2).
 *
 * The important point to recognize when implementing the LTOFF22X/LDXMOV
 * optimization is that relocations are interdependent, the LDXMOV is
 * only applied if the LTOFF22X is applied. It is also worth noting that
 * there is no relationship between LDXMOV relocations and LTOFF22X in
 * the ELF relocation section other than they share the same
 * symbol+addend value.
 */

#define IA64_LDX_OPTIMIZATION 1
#endif

#ifndef UseMMAP
# if defined(__ia64__) || defined(__sparc__)
#  define MergeSectionAlloc
# endif
#endif

#if defined(DoMMAPedMerge)
# include <sys/mman.h>
# define MergeSectionAlloc
# define MMAP_PROT	(PROT_READ | PROT_WRITE | PROT_EXEC)
# if !defined(linux)
#  error    No MAP_ANON?
# endif
# if !(defined(__amd64__) || defined(__x86_64__)) || !defined(__linux__)
# define MMAP_FLAGS     (MAP_PRIVATE | MAP_ANON)
# else
# define MMAP_FLAGS     (MAP_PRIVATE | MAP_ANON | MAP_32BIT)
# endif
# if defined(MmapPageAlign)
#  define MMAP_ALIGN(size)    do { \
     int pagesize = getpagesize(); \
     size = ( size + pagesize - 1) / pagesize; \
     size *= pagesize; \
   } while (0);
# else
#  define MMAP_ALIGN(size)
# endif
#endif

#if defined(__alpha__) || \
    defined(__ia64__) || \
    defined(__amd64__) || \
    defined(__x86_64__) || \
    (defined(__sparc__) && \
     (defined(__arch64__) || defined(__sparcv9) || defined(__sparcv9__)))
typedef Elf64_Ehdr Elf_Ehdr;
typedef Elf64_Shdr Elf_Shdr;
typedef Elf64_Sym Elf_Sym;
typedef Elf64_Rel Elf_Rel;
typedef Elf64_Rela Elf_Rela;
typedef Elf64_Addr Elf_Addr;
typedef Elf64_Half Elf_Half;
typedef Elf64_Off Elf_Off;
typedef Elf64_Sword Elf_Sword;
typedef Elf64_Word Elf_Word;

#define ELF_ST_BIND ELF64_ST_BIND
#define ELF_ST_TYPE ELF64_ST_TYPE
#define ELF_R_SYM ELF64_R_SYM

#if !defined(__sparc__) || \
    (!defined(__sparcv9) && !defined(__sparcv9__) && !defined(__arch64__))
#define ELF_R_TYPE ELF64_R_TYPE
#else
/*
 * bfd says: Relocations in the 64 bit SPARC ELF ABI are more complex
 * than in standard ELF, because R_SPARC_OLO10 has secondary addend in
 * ELF64_R_TYPE_DATA field.
 */
#define ELF_R_TYPE(info)	((info) & 0xff)
#endif

# if defined(__alpha__) || defined(__ia64__)
/*
 * The GOT is allocated dynamically. We need to keep a list of entries that
 * have already been added to the GOT.
 *
 */
typedef struct _elf_GOT_Entry {
    Elf_Rela *rel;
    int offset;
    struct _elf_GOT_Entry *next;
} ELFGotEntryRec, *ELFGotEntryPtr;

typedef struct _elf_GOT {
    unsigned int size;
    unsigned int nuses;
    unsigned char *freeptr;
    struct _elf_GOT *next;
    unsigned char section[1];
} ELFGotRec, *ELFGotPtr;

#  ifdef MergeSectionAlloc
static ELFGotPtr ELFSharedGOTs;
#  endif
# endif

# if defined(__ia64__)
/*
 * The PLT is allocated dynamically. We need to keep a list of entries that
 * have already been added to the PLT.
 */
typedef struct _elf_PLT_Entry {
    Elf_Rela *rel;
    int offset;
    int gotoffset;
    struct _elf_PLT_Entry *next;
} ELFPltEntryRec, *ELFPltEntryPtr;

/*
 * The OPD is allocated dynamically within the GOT. We need to keep a list
 * of entries that have already been added to the OPD.
 */
typedef struct _elf_OPD {
    LOOKUP *l;
    int index;
    int offset;
    struct _elf_OPD *next;
} ELFOpdRec, *ELFOpdPtr;
# endif

#else
typedef Elf32_Ehdr Elf_Ehdr;
typedef Elf32_Shdr Elf_Shdr;
typedef Elf32_Sym Elf_Sym;
typedef Elf32_Rel Elf_Rel;
typedef Elf32_Rela Elf_Rela;
typedef Elf32_Addr Elf_Addr;
typedef Elf32_Half Elf_Half;
typedef Elf32_Off Elf_Off;
typedef Elf32_Sword Elf_Sword;
typedef Elf32_Word Elf_Word;

#define ELF_ST_BIND ELF32_ST_BIND
#define ELF_ST_TYPE ELF32_ST_TYPE
#define ELF_R_SYM ELF32_R_SYM
#define ELF_R_TYPE ELF32_R_TYPE
#endif

#if defined(__powerpc__) || \
    defined(__mc68000__) || \
    defined(__alpha__) || \
    defined(__sparc__) || \
    defined(__ia64__) || \
    defined(__amd64__) || \
    defined(__x86_64__)
typedef Elf_Rela Elf_Rel_t;
#else
typedef Elf_Rel Elf_Rel_t;
#endif

typedef struct {
    void *saddr;
    char *name;
    int ndx;
    int size;
    int flags;
} LoadSection;

#define RELOC_SECTION 0x1
#define LOADED_SECTION 0x2

/*
 * This structure contains all of the information about a module
 * that has been loaded.
 */

#if defined(__powerpc__)
typedef struct {
    unsigned long relinfo;
    unsigned short code[8];
    void *address;
} pltentry;
#endif

typedef struct {
    int handle;
    int module;
    int fd;
    LoaderDescPtr desc;
    Elf_Ehdr *header;		/* file header */
    int numsh;
    Elf_Shdr *sections;		/* Address of the section header table */
    int secsize;		/* size of the section table */
    unsigned char **saddr;	/* Start addresss of the section pointer table */
    unsigned char *shstraddr;	/* Start address of the section header string table */
    int shstrndx;		/* index of the section header string table */
    int shstrsize;		/* size of the section header string table */
#if defined(__alpha__) || defined(__ia64__)
    unsigned char *got;		/* Start address of the .got section */
    ELFGotEntryPtr got_entries;	/* List of entries in the .got section */
    int gotndx;			/* index of the .got section */
    int gotsize;		/* actual size of the .got section */
    ELFGotPtr shared_got;	/* Pointer to ELFGotRec if shared */
#endif				/*(__alpha__) || (__ia64__) */
#if defined(__ia64__)
    ELFOpdPtr opd_entries;	/* List of entries in the .opd section */
    unsigned char *plt;		/* Start address of the .plt section */
    ELFPltEntryPtr plt_entries;	/* List of entries in the .plt section */
    int pltndx;			/* index of the .plt section */
    int pltsize;		/* size of the .plt section */
#endif /*__ia64__*/
#if defined(__powerpc__)
    pltentry *plt;		/* Start of pseudo PLT */
    int numplt;			/* number of PLT entries */
#endif
    Elf_Sym *symtab;		/* Start address of the .symtab section */
    int symndx;			/* index of the .symtab section */
    unsigned char *common;	/* Start address of the SHN_COMMON space */
    int comsize;		/* size of the SHN_COMMON space */

    unsigned char *base;	/* Alloced address of section block */
    unsigned long baseptr;	/* Pointer to next free space in base */
    int basesize;		/* Size of that allocation */
    unsigned char *straddr;	/* Start address of the string table */
    int strndx;			/* index of the string table */
    int strsize;		/* size of the string table */
    LoadSection *lsection;
    int lsectidx;
} ELFModuleRec, *ELFModulePtr;

/*
 * If a relocation is unable to be satisfied, then put it on a list
 * to try later after more modules have been loaded.
 */
typedef union {
#if !defined(__ia64__)
    unsigned int	d32;
#endif
#if defined(__powerpc__) || defined(__sparc__)
    unsigned short	d16;
#endif
#if defined(__sparc__)
    unsigned char	d8;
    unsigned long	d64;
#endif
#if defined(__alpha)
    unsigned long	d64;
    unsigned short	d16;
#endif
#if  defined(__amd64__) || defined(__x86_64__)
    unsigned long	d64;
    int			d32s;
#endif
#if defined(__ia64__)
    unsigned long	d64;
    unsigned long	d128[2];
#endif
} relocData;

typedef struct _elf_reloc {
    Elf_Rel_t *rel;
    ELFModulePtr file;
    Elf_Word secn;
    int relocated;
    Elf_Addr symval;
    int assigned;
    relocData olddata;
    struct _elf_reloc *next;
} ELFRelocRec;

/*
 * symbols with a st_shndx of COMMON need to have space allocated for them.
 *
 * Gather all of these symbols together, and allocate one chunk when we
 * are done.
 */
typedef struct _elf_COMMON {
    Elf_Sym *sym;
    struct _elf_COMMON *next;
} ELFCommonRec;

static ELFCommonPtr listCOMMON = NULL;

/* Prototypes for static functions */
static int ELFhashCleanOut(void *, itemPtr);
static char *ElfGetStringIndex(ELFModulePtr, int, int);
static char *ElfGetString(ELFModulePtr, int);
static char *ElfGetSectionName(ELFModulePtr, int);
static ELFRelocPtr ElfDelayRelocation(ELFModulePtr, Elf_Word, Elf_Rel_t *);
static ELFCommonPtr ElfAddCOMMON(Elf_Sym *);
static int ElfCOMMONSize(void);
static int ElfCreateCOMMON(ELFModulePtr, LOOKUP *);
static const char *ElfGetSymbolNameIndex(ELFModulePtr, int, int);
static const char *ElfGetSymbolName(ELFModulePtr, int);
static Elf_Addr ElfGetSymbolValue(ELFModulePtr, int, int *);
static int Elf_RelocateEntry(ELFRelocPtr);
static ELFRelocPtr ELFCollectRelocations(ELFModulePtr, int);
static LOOKUP *ELF_GetSymbols(ELFModulePtr, unsigned short **);
static void ELFCollectSections(ELFModulePtr, int, int *, int *);

#if defined(__alpha__) || defined(__ia64__)
static void ElfAddGOT(ELFModulePtr, Elf_Rel_t *);
static int ELFCreateGOT(ELFModulePtr, int);
#endif
#if defined(__ia64__)
static void ElfAddOPD(ELFModulePtr, int, LOOKUP *);
static void ELFCreateOPD(ELFModulePtr);
static void ElfAddPLT(ELFModulePtr, Elf_Rel_t *);
static void ELFCreatePLT(ELFModulePtr);
enum ia64_operand {
    IA64_OPND_IMM22,
    IA64_OPND_TGT25C,
    IA64_OPND_LDXMOV
};
static void IA64InstallReloc(unsigned long *, int, enum ia64_operand, long);
#endif /*__ia64__*/
#if defined(__powerpc__)
static void ELFCreatePLT(ELFModulePtr);
#endif

#ifdef MergeSectionAlloc
static void *
ELFLoaderSectToMem(ELFModulePtr elffile, int align, unsigned long offset,
		   int size, char *label)
{
    void *ret;

    elffile->baseptr = (elffile->baseptr + align - 1) & ~(align - 1);
    ret = (void *)elffile->baseptr;
    _LoaderFileRead(elffile->fd, offset, ret, size);
    elffile->baseptr += size;
    return ret;
}

static void *
ELFLoaderSectCalloc(ELFModulePtr elffile, int align, int size)
{
    void *ret;
    unsigned long newbaseptr;

    newbaseptr = (elffile->baseptr + align - 1) & ~(align - 1);
    ret = (void *)newbaseptr;
    newbaseptr += size;
    if (newbaseptr > (unsigned long)elffile->base + elffile->basesize) {
	ErrorF("ELFLoaderSectCalloc: need to grow base (0x%lx, 0x%lx).\n",
		newbaseptr, (unsigned long)elffile->base + elffile->basesize);
	/*
	 * Cannot grow the area without the possibility of it moving.
	 * This shouldn't happen anyway, since all space should be reserved
	 * before ever calling this function.
	 */
	return NULL;
    }
    elffile->baseptr = newbaseptr;
#ifndef DoMMAPedMerge
    memset(ret, 0, size);	/* mmap() does this for us */
#endif
    return ret;
}
#else /* MergeSectionAlloc */
# define ELFLoaderSectToMem(elffile,align,offset,size,label)	\
_LoaderFileToMem((elffile)->fd,offset,size,label)
# define ELFLoaderSectCalloc(elffile,align,size) xf86loadercalloc(1,size)
#endif

/*
 * Utility Functions
 */

static int
ELFhashCleanOut(void *voidptr, itemPtr item)
{
    ELFModulePtr module = (ELFModulePtr) voidptr;

    return (module->handle == item->handle);
}

/*
 * Manage listResolv
 */
static ELFRelocPtr
ElfDelayRelocation(ELFModulePtr elffile, Elf_Word secn, Elf_Rel_t *rel)
{
    ELFRelocPtr reloc;

    if ((reloc = xf86loadermalloc(sizeof(ELFRelocRec))) == NULL) {
	ErrorF("ElfDelayRelocation() Unable to allocate memory!!!!\n");
	return 0;
    }
    reloc->file = elffile;
    reloc->secn = secn;
    reloc->rel = rel;
    reloc->relocated = 0;
    reloc->symval = 0;
    reloc->assigned = 0;
    memset(&reloc->olddata, 0, sizeof(reloc->olddata));
    reloc->next = NULL;

#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
	     "ElfDelayRelocation %p: file %p, sec %d,"
	     " r_offset 0x%lx, r_info 0x%x",
	     (void *)reloc, (void *)elffile, secn,
	     (unsigned long)rel->r_offset, (int)rel->r_info);
# if defined(__powerpc__) || \
    defined(__mc68000__) || \
    defined(__alpha__) || \
    defined(__sparc__) || \
    defined(__ia64__) || \
    defined(__amd64__) || \
    defined(__x86_64__)
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, ", r_addend 0x%lx", rel->r_addend);
# endif
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "\n");
#endif
    return reloc;
}

/*
 * Manage listCOMMON
 */
static ELFCommonPtr
ElfAddCOMMON(Elf_Sym *sym)
{
    ELFCommonPtr common;

    if ((common = xf86loadermalloc(sizeof(ELFCommonRec))) == NULL) {
	ErrorF("ElfAddCOMMON() Unable to allocate memory!!!!\n");
	return 0;
    }
    common->sym = sym;
    common->next = 0;
    return common;
}

static int
ElfCOMMONSize(void)
{
    int size = 0;
    ELFCommonPtr common;

    for (common = listCOMMON; common; common = common->next) {
	size += common->sym->st_size;
#if defined(__alpha__) || \
    defined(__ia64__) || \
    defined(__amd64__) || \
    defined(__x86_64__) || \
    (defined(__sparc__) && \
     (defined(__arch64__) || defined(__sparcv9) || defined(__sparcv9__)))
	size = (size + 7) & ~0x7;
#endif
    }
    return size;
}

static int
ElfCreateCOMMON(ELFModulePtr elffile, LOOKUP *pLookup)
{
    int numsyms = 0, size = 0, l = 0;
    int offset = 0, firstcommon = 0;
    ELFCommonPtr common;

    if (listCOMMON == NULL)
	return TRUE;

    for (common = listCOMMON; common; common = common->next) {
	size += common->sym->st_size;
#if defined(__alpha__) || \
    defined(__ia64__) || \
    defined(__amd64__) || \
    defined(__x86_64__) || \
    (defined(__sparc__) && \
     (defined(__arch64__) || defined(__sparcv9) || defined(__sparcv9__)))
	size = (size + 7) & ~0x7;
#endif
	numsyms++;
    }

#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
	     "ElfCreateCOMMON() %d entries (%d bytes) of COMMON data\n",
	     numsyms, size);
#endif

    elffile->comsize = size;
    if ((elffile->common = ELFLoaderSectCalloc(elffile, 8, size)) == NULL) {
	ErrorF("ElfCreateCOMMON() Unable to allocate memory!!!!\n");
	return FALSE;
    }

    if (DebuggerPresent) {
	ldrCommons = xf86loadermalloc(numsyms * sizeof(LDRCommon));
	nCommons = numsyms;
    }

    for (l = 0; pLookup[l].symName; l++) ;
    firstcommon = l;

    /* Traverse the common list and create a lookup table with all the
     * common symbols.  Destroy the common list in the process.
     * See also ResolveSymbols.
     */
    while (listCOMMON) {
	common = listCOMMON;
	/* this is xstrdup because is should be more efficient. it is freed
	 * with xf86loaderfree
	 */
	pLookup[l].symName =
		xf86loaderstrdup(ElfGetString(elffile, common->sym->st_name));
	pLookup[l].offset = (funcptr) (elffile->common + offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		 "Adding common %p %s\n",
		 (void *)pLookup[l].offset, pLookup[l].symName);
#endif

	/* Record the symbol address for gdb */
	if (DebuggerPresent && ldrCommons) {
	    ldrCommons[l - firstcommon].addr = (void *)pLookup[l].offset;
	    ldrCommons[l - firstcommon].name = pLookup[l].symName;
	    ldrCommons[l - firstcommon].namelen = strlen(pLookup[l].symName);
	}
	listCOMMON = common->next;
	offset += common->sym->st_size;
#if defined(__alpha__) || \
    defined(__ia64__) || \
    defined(__amd64__) || \
    defined(__x86_64__) || \
    (defined(__sparc__) && \
     (defined(__arch64__) || defined(__sparcv9) || defined(__sparcv9__)))
	offset = (offset + 7) & ~0x7;
#endif
	xf86loaderfree(common);
	l++;
    }
    /* listCOMMON == 0 */
    pLookup[l].symName = NULL;	/* Terminate the list. */
    return TRUE;
}

/*
 * String Table
 */
static char *
ElfGetStringIndex(ELFModulePtr file, int offset, int index)
{
    if (!offset || !index)
	return "";

    return (char *)(file->saddr[index] + offset);
}

static char *
ElfGetString(ELFModulePtr file, int offset)
{
    return ElfGetStringIndex(file, offset, file->strndx);
}

static char *
ElfGetSectionName(ELFModulePtr file, int offset)
{
    return (char *)(file->shstraddr + offset);
}

/*
 * Symbol Table
 */

/*
 * Get symbol name
 */
static const char *
ElfGetSymbolNameIndex(ELFModulePtr elffile, int index, int secndx)
{
    Elf_Sym *syms;

#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
	     "ElfGetSymbolNameIndex(%x,%x) ", index, secndx);
#endif

    syms = (Elf_Sym *) elffile->saddr[secndx];

#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
	     "%s ", ElfGetString(elffile, syms[index].st_name));
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
	     "%x %x ", ELF_ST_BIND(syms[index].st_info),
	     ELF_ST_TYPE(syms[index].st_info));
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
	     "%lx\n", (unsigned long)syms[index].st_value);
#endif

    return ElfGetString(elffile, syms[index].st_name);
}

static const char *
ElfGetSymbolName(ELFModulePtr elffile, int index)
{
    return ElfGetSymbolNameIndex(elffile, index, elffile->symndx);
}

static Elf_Addr
ElfGetSymbolValue(ELFModulePtr elffile, int index, int *pInvariant)
{
    Elf_Sym *syms;
    Elf_Addr symval = 0;	/* value of the indicated symbol */
    char *symname = NULL;	/* name of symbol in relocation */
    itemPtr symbol = NULL;	/* name/value of symbol */

    syms = (Elf_Sym *) elffile->saddr[elffile->symndx];

    switch (ELF_ST_TYPE(syms[index].st_info)) {
    case STT_NOTYPE:
    case STT_OBJECT:
    case STT_FUNC:
	switch (ELF_ST_BIND(syms[index].st_info)) {
	case STB_LOCAL:
	    symval = (Elf_Addr) (elffile->saddr[syms[index].st_shndx] +
				 syms[index].st_value);
#ifdef __ia64__
	    if (ELF_ST_TYPE(syms[index].st_info) == STT_FUNC) {
		ELFOpdPtr opdent;

		for (opdent = elffile->opd_entries; opdent;
		     opdent = opdent->next)
		    if (opdent->index == index)
			break;
		if (opdent) {
		    ((unsigned long *)(elffile->got + opdent->offset))[0] =
			    symval;
		    ((unsigned long *)(elffile->got + opdent->offset))[1] =
			    (long)elffile->got;
		    symval = (Elf_Addr) (elffile->got + opdent->offset);
		}
	    }
#endif
	    if (pInvariant)
		*pInvariant = 1;
	    break;
	case STB_GLOBAL:
	case STB_WEAK:		/* STB_WEAK seems like a hack to cover for
				 * some other problem */
	    symname = ElfGetString(elffile, syms[index].st_name);
	    symbol = LoaderHashFind(symname);
	    if (symbol == NULL ||
		!SCOPE_OK(symbol, elffile->handle, LOOKUP_SCOPE_GLOBAL)) {
		return 0;
	    }
	    symval = (Elf_Addr) symbol->address;
	    if (pInvariant) {
		*pInvariant = ((symbol->handle == elffile->handle) ||
			       (symbol->scope & LOOKUP_SCOPE_BUILTIN));
	    }

	    break;
	default:
	    symval = 0;
	    ErrorF("ElfGetSymbolValue(), unhandled symbol scope %x\n",
		   ELF_ST_BIND(syms[index].st_info));
	    break;
	}
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "%p\t", (void *)symbol);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "%lx\t", (unsigned long)symval);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
	     "%s\n", symname ? symname : "NULL");
#endif
	break;
    case STT_SECTION:
	symval = (Elf_Addr) elffile->saddr[syms[index].st_shndx];
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
	     "ST_SECTION %lx\n", (unsigned long)symval);
#endif
	if (pInvariant)
	    *pInvariant = 1;
	break;
    case STT_FILE:
    case STT_LOPROC:
    case STT_HIPROC:
    default:
	symval = 0;
	ErrorF("ElfGetSymbolValue(), unhandled symbol type %x\n",
	       ELF_ST_TYPE(syms[index].st_info));
	break;
    }
    return symval;
}

#if defined(__powerpc__)
/*
 * This function returns the address of a pseudo PLT routine which can
 * be used to compute a function offset. This is needed because loaded
 * modules have an offset from the .text section of greater than 24 bits.
 * The code generated makes the assumption that all function entry points
 * will be within a 24 bit offset (non-PIC code).
 */
static Elf_Addr
ElfGetPLTAddr(ELFModulePtr elffile, int index)
{
    Elf_Sym *syms;
    Elf_Addr symval = 0;	/* value of the indicated symbol */
    char *symname = NULL;	/* name of symbol in relocation */
    itemPtr symbol;		/* name/value of symbol */
    pltentry *plt;
    int i;

    syms = (Elf_Sym *) elffile->saddr[elffile->symndx];

    switch (ELF_ST_TYPE(syms[index].st_info)) {
    case STT_NOTYPE:
    case STT_OBJECT:
    case STT_FUNC:
	switch (ELF_ST_BIND(syms[index].st_info)) {
	case STB_GLOBAL:
	    symname = ElfGetString(elffile, syms[index].st_name);
	    symbol = LoaderHashFind(symname);
	    if (symbol == NULL) {
		symname = "LoaderDefaultFunc";
		symbol = LoaderHashFind(symname);
	    }
	    if (symbol == NULL ||
		!SCOPE_OK(symbol, elffile->handle, LOOKUP_SCOPE_GLOBAL))
		return 0;

	    for (i = 0; i < elffile->numplt; i++) {
		plt = elffile->plt + i;
		if (plt->address && plt->relinfo == index) {
#if LOADERDEBUG
		    LoaderDebugMsg(LOADER_DEBUG_PLT,
				   "already created entry for index %d (%d)\n",
				   index, i);
#endif
		    /* PLT entry created. */
		    if (symbol->address == plt->address) {
			/* PLT entry points to the same place. */
			symval = (Elf_Addr)&plt->code[0];
#if LOADERDEBUG
			LoaderDebugMsg(LOADER_DEBUG_PLT,
				       "PLT entry for index %d has address "
				       "unchanged (%p)\n", index, plt->address);
#endif
			break;
		    }
		} else if (plt->address) {
		    continue;
		}
#if LOADERDEBUG
		if (plt->address)
		    LoaderDebugMsg(LOADER_DEBUG_PLT,
				   "PLT entry for index %d has changed "
				   "address (%p -> %p)\n",
				   index, plt->address, symbol->address);
		else
		    LoaderDebugMsg(LOADER_DEBUG_PLT,
				   "New PLT entry for index %d (%d) "
				   "address %p\n",
				   index, i, symbol->address);
#endif
		plt->relinfo = index;
		plt->address = symbol->address;
/*
 * Here we are building up a pseudo Plt function that can make a call to
 * a function that has an offset greater than 24 bits. The following code
 * is being used to implement this.

     1  00000000                                .extern realfunc
     2  00000000                                .global pltfunc
     3  00000000                        pltfunc:
     4  00000000  3d 80 00 00                   lis     r12,hi16(realfunc)
     5  00000004  61 8c 00 00                   ori     r12,r12,lo16(realfunc)
     6  00000008  7d 89 03 a6                   mtctr   r12
     7  0000000c  4e 80 04 20                   bctr

 */

		plt->code[0] = 0x3d80;	/* lis     r12 */
		plt->code[1] =
			(((Elf_Addr) symbol->address) & 0xffff0000) >> 16;
		plt->code[2] = 0x618c;	/* ori     r12,r12 */
		plt->code[3] = (Elf_Addr) symbol->address & 0xffff;
		plt->code[4] = 0x7d89;	/* mtcr    r12 */
		plt->code[5] = 0x03a6;
		plt->code[6] = 0x4e80;	/* bctr */
		plt->code[7] = 0x0420;
		ppc_flush_icache(&plt->code[0]);
		ppc_flush_icache(&plt->code[6]);
		symval = (Elf_Addr)&plt->code[0];
		break;
	    }
	    if (i == elffile->numplt) {
		/* This should not happen. */
		ErrorF("ElfGetPLTAddr(): "
		       "ran out of PLT entries for index %d\n", index);
	    }
	    break;
	default:
	    ErrorF("ElfGetPLTAddr(), unhandled symbol scope %x\n",
		   ELF_ST_BIND(syms[index].st_info));
	    return 0;
	    break;
	}
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_PLT,
		       "ElfGetPLTAddr: symbol=%lx\t", symbol);
	LoaderDebugMsg(LOADER_DEBUG_PLT, "newval=%lx\t", symval);
	LoaderDebugMsg(LOADER_DEBUG_PLT,
		       "name=\"%s\"\n", symname ? symname : "NULL");
#endif
	break;
    case STT_SECTION:
    case STT_FILE:
    case STT_LOPROC:
    case STT_HIPROC:
    default:
	symval = 0;
	ErrorF("ElfGetPLTAddr(), Unexpected symbol type %x",
	       ELF_ST_TYPE(syms[index].st_info));
	ErrorF("for a Plt request\n");
	break;
    }
    return symval;
}
#endif /* __powerpc__ */

#if defined(__alpha__) || defined(__ia64__)
/*
 * Manage GOT Entries
 */
static void
ElfAddGOT(ELFModulePtr elffile, Elf_Rel_t *rel)
{
    ELFGotEntryPtr gotent;

#if LOADERDEBUG
    {
	Elf_Sym *sym;

	sym = (Elf_Sym *) & (elffile->symtab[ELF_R_SYM(rel->r_info)]);
	if (sym->st_name) {
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		     "ElfAddGOT: Adding GOT entry for %s\n",
		     ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	} else
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		     "ElfAddGOT: Adding GOT entry for %s\n",
		     ElfGetSectionName(elffile,
				       elffile->sections[sym->st_shndx].
				       sh_name));
    }
#endif

    for (gotent = elffile->got_entries; gotent; gotent = gotent->next) {
	if (ELF_R_SYM(gotent->rel->r_info) == ELF_R_SYM(rel->r_info) &&
	    gotent->rel->r_addend == rel->r_addend)
	    break;
    }

    if (gotent) {
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "Entry already present in GOT\n");
#endif
	return;
    }

    if ((gotent = xf86loadermalloc(sizeof(ELFGotEntryRec))) == NULL) {
	ErrorF("ElfAddGOT() Unable to allocate memory!!!!\n");
	return;
    }
#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
	     "Entry added with offset %x\n", elffile->gotsize);
#endif
    gotent->rel = rel;
    gotent->offset = elffile->gotsize;
    gotent->next = elffile->got_entries;
    elffile->got_entries = gotent;
    elffile->gotsize += 8;
    return;
}

static int
ELFCreateGOT(ELFModulePtr elffile, int maxalign)
{
# ifdef MergeSectionAlloc
    ELFGotPtr gots;
# endif
    int gotsize;

    /*
     * XXX:  Is it REALLY needed to ensure GOT's are non-null?
     */
#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
	     "ELFCreateGOT: %x entries in the GOT\n", elffile->gotsize / 8);

    /*
     * Hmmm. Someone is getting here without any got entries, but they
     * may still have R_ALPHA_GPDISP relocations against the got.
     */
    if (elffile->gotsize == 0)
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		 "Module %s doesn't have any GOT entries!\n",
		 _LoaderModuleToName(elffile->module));
#endif
    if (elffile->gotsize == 0)
	elffile->gotsize = 8;
    elffile->sections[elffile->gotndx].sh_size = elffile->gotsize;
    gotsize = elffile->gotsize;

# ifdef MergeSectionAlloc
#  ifdef __alpha__
#   define GOTDistance 0x100000
#  endif
#  ifdef __ia64__
#   define GOTDistance 0x200000
#  endif
    for (gots = ELFSharedGOTs; gots; gots = gots->next) {
	if (gots->freeptr + elffile->gotsize > gots->section + gots->size)
	    continue;
	if (gots->section > elffile->base) {
	    if (gots->section + gots->size - elffile->base >= GOTDistance)
		continue;
	} else {
	    if (elffile->base + elffile->basesize - gots->section >=
		GOTDistance)
		continue;
	}
	elffile->got = gots->freeptr;
	elffile->shared_got = gots;
	gots->freeptr = gots->freeptr + elffile->gotsize;
	gots->nuses++;
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		 "ELFCreateGOT: GOT address %p in shared GOT, nuses %d\n",
		 elffile->got, gots->nuses);
#endif
	return TRUE;
    }

    gotsize += 16383 + sizeof(ELFGotRec);
# endif	/*MergeSectionAlloc */

    if ((elffile->got = xf86loadermalloc(gotsize)) == NULL) {
	ErrorF("ELFCreateGOT() Unable to allocate memory!!!!\n");
	return FALSE;
    }
# ifdef MergeSectionAlloc
    if (elffile->got > elffile->base) {
	if (elffile->got + elffile->gotsize - elffile->base >= GOTDistance)
	    gotsize = 0;
    } else {
	if (elffile->base + elffile->basesize - elffile->got >= GOTDistance)
	    gotsize = 0;
    }

    if (!gotsize) {
	xf86loaderfree(elffile->got);
#  if !defined(DoMMAPedMerge)
	elffile->basesize += 8 + elffile->gotsize;
	elffile->base = xf86loaderrealloc(elffile->base, elffile->basesize);
	if (elffile->base == NULL) {
	    ErrorF("ELFCreateGOT() Unable to reallocate memory!!!!\n");
	    return FALSE;
	}
#   if (defined(linux) && defined(__ia64__)) || \
       defined(__OpenBSD__) || defined(__NetBSD__)
	{
	    unsigned long page_size = getpagesize();
	    unsigned long round;

	    round = (unsigned long)elffile->base & (page_size - 1);
	    mprotect(elffile->base - round,
		     (elffile->basesize + round + page_size -
		      1) & ~(page_size - 1),
		     PROT_READ | PROT_WRITE | PROT_EXEC);
	}
#   endif
#  else
	{
	    int oldbasesize = elffile->basesize;

	    elffile->basesize += 8 + elffile->gotsize;
	    MMAP_ALIGN(elffile->basesize);
	    elffile->base = mremap(elffile->base, oldbasesize,
				   elffile->basesize, MREMAP_MAYMOVE);
	    if (elffile->base == NULL) {
		ErrorF("ELFCreateGOT() Unable to remap memory!!!!\n");
		return FALSE;
	    }
	}
#  endif

	elffile->baseptr =
		((long)elffile->base + (maxalign - 1)) & ~(maxalign - 1);
	elffile->got =
		(unsigned char *)((long)(elffile->base + elffile->basesize -
					 elffile->gotsize) & ~7);
    } else {
	gots = (ELFGotPtr) elffile->got;
	elffile->got = gots->section;
	gots->size = gotsize - sizeof(ELFGotRec) + 1;
	gots->nuses = 1;
	gots->freeptr = gots->section + elffile->gotsize;
	gots->next = ELFSharedGOTs;
	ELFSharedGOTs = gots;
	elffile->shared_got = gots;
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		 "ELFCreateGOT: Created a shareable GOT with size %d\n",
		 gots->size);
#endif
    }
# endif	/*MergeSectionAlloc */

#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
	     "ELFCreateGOT: GOT address %p\n", elffile->got);
#endif

    return TRUE;
}
#endif /* defined(__alpha__) || defined(__ia64__) */

#if defined(__ia64__)
/*
 * Manage OPD Entries
 */
static void
ElfAddOPD(ELFModulePtr elffile, int index, LOOKUP *l)
{
    ELFOpdPtr opdent;

    if (index != -1) {
	for (opdent = elffile->opd_entries; opdent; opdent = opdent->next)
	    if (opdent->index == index)
		return;
    }

    if ((opdent = xf86loadermalloc(sizeof(ELFOpdRec))) == NULL) {
	ErrorF("ElfAddOPD() Unable to allocate memory!!!!\n");
	return;
    }
#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
	     "OPD Entry %d added with offset %x\n", index, elffile->gotsize);
#endif
    opdent->l = l;
    opdent->index = index;
    opdent->offset = elffile->gotsize;
    opdent->next = elffile->opd_entries;
    elffile->opd_entries = opdent;
    elffile->gotsize += 16;
    return;
}

static void
ELFCreateOPD(ELFModulePtr elffile)
{
    ELFOpdPtr opdent;

    if (elffile->got == NULL)
	ErrorF("ELFCreateOPD() Unallocated GOT!!!!\n");

    for (opdent = elffile->opd_entries; opdent; opdent = opdent->next) {
	if (opdent->index != -1)
	    continue;
	((unsigned long *)(elffile->got + opdent->offset))[0] =
		(long)opdent->l->offset;
	((unsigned long *)(elffile->got + opdent->offset))[1] =
		(long)elffile->got;
	opdent->l->offset = (funcptr) (elffile->got + opdent->offset);
    }
}

/*
 * Manage PLT Entries
 */
static void
ElfAddPLT(ELFModulePtr elffile, Elf_Rel_t *rel)
{
    ELFPltEntryPtr pltent;

#if LOADERDEBUG
    {
	Elf_Sym *sym;

	sym = (Elf_Sym *) & (elffile->symtab[ELF_R_SYM(rel->r_info)]);
	if (sym->st_name) {
	    LoaderDebugMsg(LOADER_DEBUG_PLT,
		     "ElfAddPLT: Adding PLT entry for %s\n",
		     ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	} else
	    LoaderDebugMsg(LOADER_DEBUG_PLT,
			   "ElfAddPLT: Add PLT entry for section??\n");
    }
#endif

    if (rel->r_addend)
	ErrorF("ElfAddPLT: Add PLT entry with non-zero addend??\n");

    for (pltent = elffile->plt_entries; pltent; pltent = pltent->next) {
	if (ELF_R_SYM(pltent->rel->r_info) == ELF_R_SYM(rel->r_info))
	    break;
    }

    if (pltent) {
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_PLT, "Entry already present in PLT\n");
#endif
	return;
    }

    if ((pltent = xf86loadermalloc(sizeof(ELFPltEntryRec))) == NULL) {
	ErrorF("ElfAddPLT() Unable to allocate memory!!!!\n");
	return;
    }
#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_PLT,
	     "Entry added with offset %x\n", elffile->pltsize);
#endif
    pltent->rel = rel;
    pltent->offset = elffile->pltsize;
    pltent->gotoffset = elffile->gotsize;
    pltent->next = elffile->plt_entries;
    elffile->plt_entries = pltent;
    elffile->pltsize += 32;
    elffile->gotsize += 16;
    return;
}

static void
ELFCreatePLT(ELFModulePtr elffile)
{
#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_PLT,
	     "ELFCreatePLT: %x entries in the PLT\n", elffile->pltsize / 8);
#endif

    if (elffile->pltsize == 0)
	return;

    if ((elffile->plt =
	 ELFLoaderSectCalloc(elffile, 32, elffile->pltsize)) == NULL) {
	ErrorF("ELFCreatePLT() Unable to allocate memory!!!!\n");
	return;
    }
    elffile->sections[elffile->pltndx].sh_size = elffile->pltsize;
#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_PLT,
	     "ELFCreatePLT: PLT address %lx\n", elffile->plt);
#endif

    return;
}

static void
IA64InstallReloc(unsigned long *data128, int slot, enum ia64_operand opnd,
		 long value)
{
    unsigned long data = 0;

#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
	     "\nIA64InstallReloc %p %d %d %016lx\n", data128, slot, opnd,
	     value);
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		"Before [%016lx%016lx]\n", data128[1], data128[0]);
#endif
    switch (slot) {
    case 0:
	data = *data128;
	break;
    case 1:
	memcpy(&data, (char *)data128 + 5, 8);
	break;
    case 2:
	memcpy(&data, (char *)data128 + 10, 6);
	break;
    default:
	FatalError("Unexpected slot in IA64InstallReloc()\n");
    }
    switch (opnd) {
    case IA64_OPND_IMM22:
	data &= ~(0x3fff9fc0000UL << slot);
	data |= (value & 0x7f) << (18 + slot);	/* [13:19] + 5 + slot */
	data |= (value & 0xff80) << (25 + slot);	/* [27:35] + 5 + slot */
	data |= (value & 0x1f0000) << (11 + slot);	/* [22:26] + 5 + slot */
	data |= (value & 0x200000) << (20 + slot);	/* [36:36] + 5 + slot */
	if (value << 42 >> 42 != value)
	    ErrorF("Relocation %016lx truncated to fit into IMM22\n", value);
	break;
    case IA64_OPND_TGT25C:
	data &= ~(0x23ffffc0000UL << slot);
	data |= (value & 0xfffff0) << (14 + slot);	/* [13:32] + 5 + slot */
	data |= (value & 0x1000000) << (17 + slot);	/* [36:36] + 5 + slot */
	if (value << 39 >> 39 != value || (value & 0xf))
	    ErrorF("Relocation %016lx truncated to fit into TGT25C\n", value);
	break;
#ifdef IA64_LDX_OPTIMIZATION
    case IA64_OPND_LDXMOV:
	/*
	 * Convert "ld8 t2=[t1]" to "mov t2=t1" which is really "add t2=0,t1"
	 * Mask all but the r3,r1,qp fields,
	 * then OR in the ALU opcode = 8 into the opcode field [40:37]
	 *
	 * Mask for the r3,r1,qp bit fields [26:20][12:6][5:0] = 0x7f01fff,
	 * This mask negated only within the 41 bit wide instruction and
	 * shifted left by 5 for the bundle template is 0x3FFF01FC0000
	 *
	 * opcode field [40:37] with a value of 8 is 0x10000000000
	 * shifted left by 5 for the bundle template is 0x200000000000
	 *
	 */
	data &= ~(0x3FFF01FC0000 << slot);
	data |= (0x200000000000 << slot);
	break;
#endif
    default:
	FatalError("Unhandled operand in IA64InstallReloc()\n");
    }
    switch (slot) {
    case 0:
	*data128 = data;
	break;
    case 1:
	memcpy((char *)data128 + 5, &data, 8);
	break;
    case 2:
	memcpy((char *)data128 + 10, &data, 6);
	break;
    default:
	FatalError("Unexpected slot in IA64InstallReloc()\n");
    }
    ia64_flush_cache(data128);
#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
	     "After  [%016lx%016lx]\n", data128[1], data128[0]);
#endif
}

#endif /* __ia64__ */

#if defined(__powerpc__)
static int
ELFGetNumPLTEntries(ELFModulePtr elffile)
{
    ELFRelocPtr p;
    int total = 0, n;
    int minIndex = MAXINT, maxIndex = 0;

    /*
     * Make an (over) estimate of how many PLT entries will be needed.
     * Return the minimum of the total number of relocations and the
     * number of relocations spanning the min,max reloc indices.
     * The over-estimate typically requires less than one page of storage,
     * so calculating a precise count of the unique PLT entries needed isn't
     * of any significant benefit.
     */
    for (p = *_LoaderGetRelocations(elffile->desc); p; p = p->next) {
	if (p->file != elffile)
	    continue;
	switch (ELF_R_TYPE(p->rel->r_info)) {
#if defined(PowerMAX_OS)
	case R_PPC_DISP24:
#endif
	case R_PPC_REL24:
	    if (ELF_R_SYM(p->rel->r_info) < minIndex)
		minIndex = ELF_R_SYM(p->rel->r_info);
	    if (ELF_R_SYM(p->rel->r_info) > maxIndex)
		maxIndex = ELF_R_SYM(p->rel->r_info);
	    total++;
	    break;
	default:
	    break;
	}
    }
#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_PLT,
		   "Found a total of %d PLT relocations ([min,max] %d) "
		   "for module %s, pltsize is %d\n",
		   total, maxIndex - minIndex + 1,
		   _LoaderHandleToCanonicalName(elffile->handle),
		   total * sizeof(pltentry));
#endif
    if (total < maxIndex - minIndex + 1)
	return total;
    else
	return maxIndex - minIndex + 1;
}

static void
ELFCreatePLT(ELFModulePtr elffile)
{
#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_PLT,
		   "ELFCreatePLT: %x entries in the PLT\n", elffile->numplt);
#endif

    if (elffile->numplt == 0)
	return;

    if (!(elffile->plt =
	  ELFLoaderSectCalloc(elffile, 8,
			      elffile->numplt * sizeof(pltentry)))) {
	ErrorF("ELFCreatePLT() Unable to allocate memory!!!!\n");
	return;
    }
#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_PLT,
		   "ELFCreatePLT: PLT address %lx\n", elffile->plt);
#endif

    return;
}
#endif /* __powerpc__ */

#if defined(i386) || defined(__x86_64__) || defined(__amd64__) || \
    defined(__alpha__) || defined(__powerpc__) || defined(__arm__)
static void
resetDest32(ELFRelocPtr p, unsigned int *dest32)
{
#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32=%8.8x\t", *dest32);
#endif
    if (p->assigned) {
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "was assigned\t");
#endif
	*dest32 = p->olddata.d32;
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32=%8.8x\t", *dest32);
#endif
    } else {
	p->olddata.d32 = *dest32;
	p->assigned = 1;
    }
}
#endif

#if defined(__alpha__) || defined(__amd64__) || defined(__x86_64__) || \
    defined(__sparc__)
static void
resetDest64(ELFRelocPtr p, unsigned long *dest64)
{
#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest64=%16.16lx\t", *dest64);
#endif
    if (p->assigned) {
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "was assigned\t");
#endif
	*dest64 = p->olddata.d64;
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest64=%16.16lx\t", *dest64);
#endif
    } else {
	p->olddata.d64 = *dest64;
	p->assigned = 1;
    }
}
#endif

#if defined(__ia64__)
static void
resetDest128(ELFRelocPtr p, unsigned long *dest128)
{
#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		   "*dest128=[%016lx%016lx]\t", dest128[1], dest128[0]);
#endif
    if (p->assigned) {
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "was assigned\t");
#endif
	dest128[0] = p->olddata.d128[0];
	dest128[1] = p->olddata.d128[1];
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
			"*dest128=[%016lx%016lx]\t", dest128[1], dest128[0]);
#endif
    } else {
	p->olddata.d128[0] = dest128[0];
	p->olddata.d128[1] = dest128[1];
	p->assigned = 1;
    }
}
#endif

#if  defined(__amd64__) || defined(__x86_64__)
static void
resetDest32s(ELFRelocPtr p, int *dest32s)
{
#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32s=%8.8x\t", *dest32s);
#endif
    if (p->assigned) {
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "was assigned\t");
#endif
	*dest32s = p->olddata.d32s;
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32s=%8.8x\t", *dest32s);
#endif
    } else {
	p->olddata.d32s = *dest32s;
	p->assigned = 1;
    }
}
#endif


/*
 * Fix all of the relocations for the given section.
 * Relocations that cannot be found are made by substituting
 * LoaderDefaultFunc.
 */

static int
Elf_RelocateEntry(ELFRelocPtr p)
{
    ELFModulePtr elffile = p->file;
    Elf_Word secn = p->secn;
    Elf_Rel_t *rel = p->rel;
    unsigned char *secp = elffile->saddr[secn];
    int invariant = 0;

#if !defined(__ia64__)
    unsigned int *dest32;	/* address of the 32 bit place being modified */
#endif
#if defined(__powerpc__) || defined(__sparc__)
    unsigned short *dest16;	/* address of the 16 bit place being modified */
#endif
#if defined(__sparc__)
    unsigned char *dest8;	/* address of the 8 bit place being modified */
    unsigned long *dest64;
#endif
#if defined(__alpha__)
    unsigned int *dest32h;	/* address of the high 32 bit place being modified */
    unsigned long *dest64;
    unsigned short *dest16;
#endif
#if  defined(__amd64__) || defined(__x86_64__)
    unsigned long *dest64;
    int *dest32s;
#endif
#if defined(__ia64__)
    unsigned long *dest64;
    unsigned long *dest128;
#endif
    Elf_Addr symval = 0;	/* value of the indicated symbol */

#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
	     "%lx %d %d\n", (unsigned long)rel->r_offset,
	     (int)ELF_R_SYM(rel->r_info), (int)ELF_R_TYPE(rel->r_info));
# if defined(__powerpc__) || \
    defined(__mc68000__) || \
    defined(__alpha__) || \
    defined(__sparc__) || \
    defined(__ia64__) || \
    defined(__amd64__) || \
    defined(__x86_64__)
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "%lx", rel->r_addend);
# endif
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "\n");
#endif /* LOADERDEBUG */
#if defined(__alpha__)
	    if (ELF_R_SYM(rel->r_info)
		&& ELF_R_TYPE(rel->r_info) != R_ALPHA_GPDISP)
#else
	    if (ELF_R_SYM(rel->r_info))
#endif
    {
	symval = ElfGetSymbolValue(elffile, ELF_R_SYM(rel->r_info), &invariant);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		 "Elf_RelocateEntry: \"%s\": ",
		 _LoaderHandleToCanonicalName(elffile->handle));
#endif
	if (symval == 0) {
	    symval = (Elf_Addr) &LoaderDefaultFunc;
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		   "Setting symbol %s to defaultfunc (%p) "
		   "(%s previously resolved)\n",
		   ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)),
		   (void *)&LoaderDefaultFunc, p->relocated ? "was" : "not");
#endif
	    p->relocated = 0;
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		     "***Unable to resolve symbol %s\n",
		     ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
#endif
	} else {
#if LOADERDEBUG
	    if (invariant) {
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "Symbol is invariant\n");
	    }
#endif
	    if (p->relocated) {
#if LOADERDEBUG
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
			 "Symbol %s already resolved\n",
			 ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
#endif
		return invariant;
	    } else {
#if LOADERDEBUG
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
			 "Resolving symbol %s (%p)\n",
			 ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)),
					(void *)symval);
#endif
		p->relocated = 1;
	    }
	}
	if (symval == p->symval) {
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		     "Symbol %s is unchanged (%p)\n",
		     ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)),
		     (void *)symval);
#endif
	    return invariant;
	} else
	    p->symval = symval;
    }

    switch (ELF_R_TYPE(rel->r_info)) {
#if defined(i386)
    case R_386_32:
	dest32 = (unsigned int *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_386_32\t");
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32=%p\t", (void *)dest32);
#endif
	resetDest32(p, dest32);
	*dest32 = symval + *dest32;
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		 "*dest32=%8.8x\n", (unsigned int)*dest32);
#endif
	break;

    case R_386_PC32:
	dest32 = (unsigned int *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		 "R_386_PC32 %s\t",
		 ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%p\t", secp);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		 "symval=%lx\t", (unsigned long)symval);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32=%p\t", (void *)dest32);
#endif

	resetDest32(p, dest32);
	*dest32 = symval + *dest32 - (Elf_Addr) dest32;

#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		 "*dest32=%8.8x\n", (unsigned int)*dest32);
#endif

	break;
#endif /* i386 */

#if defined(__amd64__) || defined(__x86_64__)
    case R_X86_64_32:
	dest32 = (unsigned int *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_X86_32\t");
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32=%p\t", dest32);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "r_addend=%lx\t", rel->r_addend);
#endif
	resetDest32(p, dest32);
	*dest32 = symval + rel->r_addend + *dest32;
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32=%8.8x\n", *dest32);
#endif
	break;

    case R_X86_64_32S:
	dest32s = (int *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_X86_64_32\t");
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32s=%p\t", dest32s);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "r_addend=%lx\t", rel->r_addend);
#endif
	resetDest32s(p, dest32s);
	*dest32s = symval + rel->r_addend + (*dest32s);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32s=%8.8x\n", *dest32s);
#endif
	break;

    case R_X86_64_PC32:
	dest32 = (unsigned int *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_X86_64_PC32 %s\t",
		 ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%p\t", secp);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32=%p\t", dest32);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "r_addend=%lx\t", rel->r_addend);
#endif
	resetDest32(p, dest32);
	*dest32 = symval + rel->r_addend + *dest32 - (Elf_Addr) dest32;

#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32=%8.8x\n", *dest32);
#endif
	break;

    case R_X86_64_64:
	dest64 = (unsigned long *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_AMD64_64\t");
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest64=%p\t", dest64);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "r_addend=%lx\t", rel->r_addend);
#endif
	resetDest64(p, dest64);
	*dest64 = symval + rel->r_addend + *dest64;
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest64=%16.16lx\n", *dest64);
#endif
	break;
#endif /* __amd64__ || __x86_64__ */

#if defined(__alpha__)
    case R_ALPHA_NONE:
    case R_ALPHA_LITUSE:
	break;

    case R_ALPHA_REFQUAD:
	dest64 = (unsigned long *)(secp + rel->r_offset);
	symval = ElfGetSymbolValue(elffile, ELF_R_SYM(rel->r_info), &invariant);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_ALPHA_REFQUAD\t");
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest64=%p\t", dest64);
#endif
	resetDest64(p, dest64);
	*dest64 = symval + rel->r_addend + *dest64;
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest64=%16.16lx\n", *dest64);
#endif
	break;

    case R_ALPHA_GPREL32:
	{
	    dest32 = (unsigned int *)(secp + rel->r_offset);

#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_ALPHA_GPREL32 %s\t",
		     ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%p\t", secp);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32=%p\t", dest32);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32=%8.8x\t", *dest32);
#endif
	    symval += rel->r_addend;
	    symval = (unsigned char *)symval - (unsigned char *)elffile->got;
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
#endif
	    if (((symval & 0xffffffff00000000) != 0x0000000000000000) &&
		((symval & 0xffffffff00000000) != 0xffffffff00000000)) {
		FatalError("R_ALPHA_GPREL32 symval-got is too large for %s\n",
			   ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	    }

	    *dest32 = symval;
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32=%x\n", *dest32);
#endif
	    break;
	}

    case R_ALPHA_GPRELLOW:
	{
	    dest16 = (unsigned short *)(secp + rel->r_offset);

#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_ALPHA_GPRELLOW %s\t",
		     ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%p\t", secp);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest16=%p\t", dest16);
#endif
	    symval += rel->r_addend;
	    symval = (unsigned char *)symval - (unsigned char *)elffile->got;

	    *dest16 = symval;

#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest16=%8.8x\n", *dest16);
#endif
	    break;
	}

    case R_ALPHA_GPRELHIGH:
	{
	    int nonfatal = 0;
	    dest16 = (unsigned short *)(secp + rel->r_offset);

#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_ALPHA_GPRELHIGH %s\t",
		     ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%p\t", secp);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest16=%p\t", dest16);
#endif

	    symval += rel->r_addend;

#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
#endif
	    /*
	     * R_ALPHA_GPRELHIGH relocations to LoaderDefaultFunc should be
	     * temporary, so ignore out of range problems with them.
	     */
	    nonfatal = (symval == (Elf_Addr) &LoaderDefaultFunc);
	    symval = (unsigned char *)symval - (unsigned char *)elffile->got;
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval-got=%lx\t", symval);
#endif
	    symval = ((long)symval >> 16) + ((symval >> 15) & 1);
	    if (((long)symval > 0x7fff) || ((long)symval < -(long)0x8000)) {
		if (nonfatal) {
#if LOADERDEBUG
		    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
				   "R_ALPHA_GPRELHIGH symval-got is too large "
				   "for %s:%lx (%ld)\n",
				   ElfGetSymbolName(elffile,
						    ELF_R_SYM(rel->r_info)),
				   symval, symval);
#endif
		} else {
		    FatalError("R_ALPHA_GPRELHIGH symval-got is too large "
			       "for %s:%lx (%ld)\n",
			       ElfGetSymbolName(elffile,
						ELF_R_SYM(rel->r_info)),
			       symval, symval);
		}
	    }

#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest16=%8.8x\n", *dest16);
#endif
	    *dest16 = symval;
	    break;
	}

    case R_ALPHA_LITERAL:
	{
	    ELFGotEntryPtr gotent;

	    dest32 = (unsigned int *)(secp + rel->r_offset);
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_ALPHA_LITERAL %s\t",
		     ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%p\t", secp);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32=%p\t", dest32);
#endif
	    resetDest32(p, dest32);

	    for (gotent = elffile->got_entries; gotent; gotent = gotent->next) {
		if (ELF_R_SYM(gotent->rel->r_info) == ELF_R_SYM(rel->r_info)
		    && gotent->rel->r_addend == rel->r_addend)
		    break;
	    }

	    /* Set the address in the GOT */
	    if (gotent) {
		*(unsigned long *)(elffile->got + gotent->offset) =
			symval + rel->r_addend;
#if LOADERDEBUG
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "Setting gotent[%x]=%lx\t",
			 gotent->offset, symval + rel->r_addend);
#endif
		if ((gotent->offset & 0xffff0000) != 0)
		    FatalError("\nR_ALPHA_LITERAL offset %x too large\n",
			       gotent->offset);
		*dest32 |= gotent->offset;
	    } else {
		unsigned long val;

		val = symval + rel->r_addend - (Elf_Addr) dest32;
#if LOADERDEBUG
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "S+A-P=%lx\t", val);
#endif
		if (((val & 0xffff0000) != 0xffff0000) &&
		    ((val & 0xffff0000) != 0x00000000)) {
		    ErrorF("\nR_ALPHA_LITERAL offset %lx too large\n", val);
		    break;
		}
		val &= 0x0000ffff;
		*dest32 |= val;
	    }
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32=%8.8x\n", *dest32);
#endif

	    break;
	}

    case R_ALPHA_GPDISP:
	{
	    long offset;
	    unsigned long val64;

	    dest32h = (unsigned int *)(secp + rel->r_offset);
	    dest32 = (unsigned int *)((secp + rel->r_offset) + rel->r_addend);
	    val64 = *dest32 | ((unsigned long)(*dest32h) << 32);

#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_ALPHA_GPDISP %s\t",
		     ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%p\t", secp);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "got=%p\t", elffile->got);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32=%p\t", dest32);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32h=%p\t", dest32h);
#endif
	    resetDest64(p, &val64);
	    *dest32 = (val64 & 0xffffffff);
	    *dest32h = (val64 >> 32);

	    if (((*dest32h >> 26) != 9) || ((*dest32 >> 26) != 8)) {
		ErrorF("\n***Bad instructions in relocating %s\n",
		       ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	    }

	    symval = (*dest32h & 0xffff) << 16 | (*dest32 & 0xffff);
	    symval = (symval ^ 0x80008000) - 0x80008000;

	    offset = (unsigned char *)elffile->got - (unsigned char *)dest32h;
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "got-dest32=%lx\t", offset);
#endif

	    if ((offset >= 0x7fff8000L) || (offset < -0x80000000L)) {
		FatalError("\nOffset overflow for R_ALPHA_GPDISP\n");
	    }

	    symval += (unsigned long)offset;
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
#endif
	    *dest32 = (*dest32 & 0xffff0000) | (symval & 0xffff);
	    *dest32h = (*dest32h & 0xffff0000) |
		    (((symval >> 16) + ((symval >> 15) & 1)) & 0xffff);
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32=%8.8x\t", *dest32);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32h=%8.8x\n", *dest32h);
#endif
	    invariant = 1;
	    p->relocated = 1;
	    break;
	}

    case R_ALPHA_HINT:
	dest32 = (unsigned int *)(secp + rel->r_offset + rel->r_addend);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_ALPHA_HINT %s\t",
		 ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%p\t", secp);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32=%p\t", dest32);
#endif
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
#endif
	symval -= (Elf_Addr) ((unsigned char *)dest32 + 4);
	if (symval % 4) {
	    ErrorF("\n%s: R_ALPHA_HINT bad alignment of offset %lx.\n",
		   ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)), symval);
	}
	symval = symval >> 2;

#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);

	if (symval & 0xffff8000) {
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
			"R_ALPHA_HINT symval too large\n");
	}
#endif

	*dest32 = (*dest32 & ~0x3fff) | (symval & 0x3fff);

#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32=%8.8x\n", *dest32);
#endif
	break;

    case R_ALPHA_GPREL16:
	{
	    dest16 = (unsigned short *)(secp + rel->r_offset);

#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_ALPHA_GPREL16 %s\t",
		     ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%p\t", secp);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest16=%p\t", dest16);
#endif
	    symval += rel->r_addend;
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
#endif
	    symval = (unsigned char *)symval - (unsigned char *)elffile->got;
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval-got=%lx\t", symval);
#endif
	    if (((long)symval > 0x7fff) || ((long)symval < -(long)0x8000)) {
		FatalError
			("\nR_ALPHA_GPREL16 symval-got is too large for %s:%lx\n",
			 ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)),
			 symval);
	    }

	    *dest16 = symval;

#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest16=%8.8x\n", *dest16);
#endif
	    break;
	}

    case R_ALPHA_SREL32:
	{
	    dest32 = (unsigned int *)(secp + rel->r_offset);
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_ALPHA_SREL32 %s\t",
		     ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%p\t", secp);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32=%p\t", dest32);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
#endif
	    symval += rel->r_addend;
	    symval -= (unsigned long) dest32;
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
#endif
	    if (((long)symval >= 0x80000000) ||
		((long)symval < -(long)0x80000000))
		FatalError("\nR_ALPHA_SREL32 overflow for %s: %lx\n",
			   ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)),
			   symval);
	    *dest32 = symval;

#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32=%8.8x\n", *dest32);
#endif
	    break;
	}

    case R_ALPHA_BRSGP:
	{
	    Elf_Sym *syms;
	    int      Delta;
	    dest32 = (unsigned int *)(secp + rel->r_offset + rel->r_addend);

#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_ALPHA_BRSGP %s\t",
			   ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%p\t", secp);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32=%p\t", dest32);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32=%8.8x\t", *dest32);
#endif

	    syms = (Elf_Sym *) elffile->saddr[elffile->symndx];

	    if (syms[ELF_R_SYM(rel->r_info)].st_other & 0x8)
		Delta = -4;
	    else
		Delta = 4;
	    symval -= (Elf_Addr) ((unsigned char *)dest32 + Delta);
	    if (symval % 4) {
		ErrorF("\n%s: R_ALPHA_BRSGP bad aligment of offset %lx.\n",
		       ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)),
		       symval);
	    }
	    symval = symval >> 2;

#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
#endif

#if LOADERDEBUG
	    if (symval & 0xffe00000) {
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
			       "R_ALPHA_BRSGP symval too large\n");
	    }
#endif
	    *dest32 = (*dest32 & ~0x1fffff) | (symval & 0x1fffff);

#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32=%8.8x\n", *dest32);
#endif
	    break;
	}
#endif /* alpha */

#if defined(__mc68000__)
    case R_68K_32:
	dest32 = (unsigned int *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_68K_32\t");
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%x\t", secp);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%x\t", symval);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "r_addend=%x\t", rel->r_addend);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32=%8.8x\t", dest32);
#endif
	{
	    unsigned long val;

	    val = symval + rel->r_addend;
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "S+A=%x\n", val);
#endif
	    *dest32 = val;
	}
	break;

    case R_68K_PC32:
	dest32 = (unsigned int *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_68K_PC32\t");
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%x\t", secp);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%x\t", symval);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "r_addend=%x\t", rel->r_addend);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32=%8.8x\t", dest32);
#endif
	{
	    unsigned long val;

	    val = symval + rel->r_addend;
	    *dest32 = val - (Elf_Addr) dest32;
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "S+A=%x\t", val);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "S+A-P=%x\n", *dest32);
#endif
	}
	break;
#endif /* __mc68000__ */

#if defined(__powerpc__)
# if defined(PowerMAX_OS)
    case R_PPC_DISP24:		/* 11 */
	dest32 = (unsigned long *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_PPC_DISP24 %s\t",
		 ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%x\t", secp);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%x\t", symval);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32=%x\t", dest32);
#endif
	resetDest32(p, dest32);
	{
	    unsigned long val;

	    val = symval + rel->r_addend - (Elf_Addr) dest32;
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "S+A-P=%x\t", val);
#endif
	    val = val >> 2;
	    if (((val & 0x3f000000) != 0x3f000000) &&
		((val & 0x3f000000) != 0x00000000)) {
#if LOADERDEBUG
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
			"R_PPC_DISP24 offset %x too large\n", val << 2);
#endif
		symval = ElfGetPLTAddr(elffile, ELF_R_SYM(rel->r_info));
		val = symval + rel->r_addend - (Elf_Addr) dest32;
#if LOADERDEBUG
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "PLT offset is %x\n", val);
#endif
		val = val >> 2;
		if (((val & 0x3f000000) != 0x3f000000) &&
		    ((val & 0x3f000000) != 0x00000000))
		    FatalError("R_PPC_DISP24 PLT offset %x too large\n",
			       val << 2);
	    }
	    val &= 0x00ffffff;
	    *dest32 |= val << 2;
	    ppc_flush_icache(dest32);
	}
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32=%8.8x\n", *dest32);
#endif
	break;

    case R_PPC_16HU:		/* 31 */
	dest16 = (unsigned short *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_PPC_16HU\t");
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%x\t", secp);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%x\t", symval);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "r_addend=%x\t", rel->r_addend);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest16=%x\t", dest16);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest16=%8.8x\t", *dest16);
#endif
	{
	    unsigned short val;

	    val = ((symval + rel->r_addend) & 0xffff0000) >> 16;
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "uhi16(S+A)=%x\n", val);
#endif
	    *dest16 = val;
	    ppc_flush_icache(dest16);
	}
	break;

    case R_PPC_32:		/* 32 */
	dest32 = (unsigned long *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_PPC_32\t");
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%x\t", secp);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%x\t", symval);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "r_addend=%x\t", rel->r_addend);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32=%8.8x\t", dest32);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32=%8.8x\n", *dest32);
#endif
	{
	    unsigned long val;

	    val = symval + rel->r_addend;
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "S+A=%x\n", val);
#endif
	    *dest32 = val;
	    ppc_flush_icache(dest32);
	}
	break;

    case R_PPC_32UA:		/* 33 */
	dest32 = (unsigned long *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_PPC_32UA\t");
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%x\t", secp);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%x\t", symval);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "r_addend=%x\t", rel->r_addend);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32=%8.8x\t", dest32);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32=%8.8x\n", *dest32);
#endif
	{
	    unsigned long val;
	    unsigned char *dest8 = (unsigned char *)dest32;

	    val = symval + rel->r_addend;
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "S+A=%x\t", val);
#endif
	    *dest8++ = (val & 0xff000000) >> 24;
	    *dest8++ = (val & 0x00ff0000) >> 16;
	    *dest8++ = (val & 0x0000ff00) >> 8;
	    *dest8++ = (val & 0x000000ff);
	    ppc_flush_icache(dest32);
	}
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32=%8.8x\n", *dest32);
#endif
	break;

    case R_PPC_16H:		/* 34 */
	dest16 = (unsigned short *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_PPC_16H\t");
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%x\t", secp);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symbol=%s\t",
		 ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%x\t", symval);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "r_addend=%x\t", rel->r_addend);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest16=%x\t", dest16);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest16=%8.8x\t", *dest16);
#endif
	{
	    unsigned short val;
	    unsigned short loval;

	    val = ((symval + rel->r_addend) & 0xffff0000) >> 16;
	    loval = (symval + rel->r_addend) & 0xffff;
	    if (loval & 0x8000) {
		/*
		 * This is hi16(), instead of uhi16(). Because of this,
		 * if the lo16() will produce a negative offset, then
		 * we have to increment this part of the address to get
		 * the correct final result.
		 */
		val++;
	    }
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "hi16(S+A)=%x\n", val);
#endif
	    *dest16 = val;
	    ppc_flush_icache(dest16);
	}
	break;

    case R_PPC_16L:		/* 35 */
	dest16 = (unsigned short *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_PPC_16L\t");
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%x\t", secp);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%x\t", symval);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "r_addend=%x\t", rel->r_addend);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest16=%x\t", dest16);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest16=%8.8x\t", *dest16);
#endif
	{
	    unsigned short val;

	    val = (symval + rel->r_addend) & 0xffff;
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "lo16(S+A)=%x\n", val);
#endif
	    *dest16 = val;
	    ppc_flush_icache(dest16);
	}
	break;
# else /* PowerMAX_OS */
	/* Linux PPC */
    case R_PPC_ADDR32:		/* 1 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval = ElfGetSymbolValue(elffile, ELF_R_SYM(rel->r_info), &invariant);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_PPC_ADDR32\t");
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%x\t", secp);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%x\t", symval);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "r_addend=%x\t", rel->r_addend);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32=%8.8x\t", dest32);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32=%8.8x\n", *dest32);
#endif
	{
	    unsigned long val;

	    val = symval + rel->r_addend;
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "S+A=%x\n", val);
#endif
	    *dest32 = val;
	    ppc_flush_icache(dest32);
	}
	break;

    case R_PPC_ADDR16_LO:	/* 4 */
	dest16 = (unsigned short *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_PPC_ADDR16_LO\t");
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%x\t", secp);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%x\t", symval);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "r_addend=%x\t", rel->r_addend);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest16=%x\t", dest16);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest16=%8.8x\t", *dest16);
#endif
	{
	    unsigned short val;

	    val = (symval + rel->r_addend) & 0xffff;
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "lo16(S+A)=%x\n", val);
#endif
	    *dest16 = val;
	    ppc_flush_icache(dest16);
	}
	break;

    case R_PPC_ADDR16_HA:	/* 6 */
	dest16 = (unsigned short *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_PPC_ADDR16_HA\t");
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%x\t", secp);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%x\t", symval);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "r_addend=%x\t", rel->r_addend);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest16=%x\t", dest16);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest16=%8.8x\t", *dest16);
#endif
	{
	    unsigned short val;
	    unsigned short loval;

	    val = ((symval + rel->r_addend) & 0xffff0000) >> 16;
	    loval = (symval + rel->r_addend) & 0xffff;
	    if (loval & 0x8000) {
		/*
		 * This is hi16(), instead of uhi16(). Because of this,
		 * if the lo16() will produce a negative offset, then
		 * we have to increment this part of the address to get
		 * the correct final result.
		 */
		val++;
	    }
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "hi16(S+A)=%x\n", val);
#endif
	    *dest16 = val;
	    ppc_flush_icache(dest16);
	}
	break;

    case R_PPC_REL24:		/* 10 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_PPC_REL24 %s\t",
		 ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%x\t", secp);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%x\t", symval);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32=%x\t", dest32);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "rel->r_addend=%x\t", rel->r_addend);
#endif
	resetDest32(p, dest32);
	{
	    unsigned long val;

	    val = symval + rel->r_addend - (Elf_Addr) dest32;
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "S+A-P=%x\t", val);
#endif
	    val = val >> 2;
	    if (((val & 0x3f000000) != 0x3f000000) &&
		((val & 0x3f000000) != 0x00000000)) {
#if LOADERDEBUG
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
			"R_PPC_REL24 offset %x too large\n", val << 2);
#endif
		symval = ElfGetPLTAddr(elffile, ELF_R_SYM(rel->r_info));
		val = symval + rel->r_addend - (Elf_Addr) dest32;
#if LOADERDEBUG
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "PLT offset is %x\n", val);
#endif
		val = val >> 2;
		if (((val & 0x3f000000) != 0x3f000000) &&
		    ((val & 0x3f000000) != 0x00000000))
		    FatalError("R_PPC_REL24 PLT offset %x too large\n",
			       val << 2);
	    }
	    val &= 0x00ffffff;
	    *dest32 |= val << 2;
	    ppc_flush_icache(dest32);
	}
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32=%8.8x\n", *dest32);
#endif
	break;

    case R_PPC_REL32:		/* 26 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_PPC_REL32\t");
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%x\t", secp);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%x\t", symval);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "r_addend=%x\t", rel->r_addend);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32=%8.8x\t", dest32);
#endif
	{
	    unsigned long val;

	    val = symval + rel->r_addend;
	    *dest32 = val - (Elf_Addr) dest32;
	    ppc_flush_icache(dest32);
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "S+A=%x\t", val);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "S+A-P=%x\n", *dest32);
#endif
	}
	break;
# endif	/* PowerMAX_OS */
#endif /* __powerpc__ */

#ifdef __sparc__
    case R_SPARC_NONE:		/*  0 */
	break;

    case R_SPARC_8:		/*  1 */
	dest8 = (unsigned char *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest8 = symval;
	break;

    case R_SPARC_16:		/*  2 */
	dest16 = (unsigned short *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest16 = symval;
	break;

    case R_SPARC_32:		/*  3 */
    case R_SPARC_UA32:		/* 23 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	((unsigned char *)dest32)[0] = (unsigned char)(symval >> 24);
	((unsigned char *)dest32)[1] = (unsigned char)(symval >> 16);
	((unsigned char *)dest32)[2] = (unsigned char)(symval >> 8);
	((unsigned char *)dest32)[3] = (unsigned char)(symval);
	break;

    case R_SPARC_GLOB_DAT:	/* 20 */
    case R_SPARC_64:		/* 32 */
	dest64 = (unsigned long *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest64 = symval;
	break;

    case R_SPARC_DISP8:		/*  4 */
	dest8 = (unsigned char *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest8 = symval - (Elf_Addr) dest8;
	break;

    case R_SPARC_DISP16:	/*  5 */
	dest16 = (unsigned short *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest16 = symval - (Elf_Addr) dest16;
	break;

    case R_SPARC_DISP32:	/*  6 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = symval - (Elf_Addr) dest32;
	break;

    case R_SPARC_WDISP30:	/*  7 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = (*dest32 & ~0x3fffffff) |
		  (((symval - (Elf_Addr) dest32) >> 2) & 0x3fffffff);
	break;

    case R_SPARC_WDISP22:	/*  8 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = (*dest32 & ~0x003fffff) |
		   (((symval - (Elf_Addr) dest32) >> 2) & 0x003fffff);
	break;

    case R_SPARC_HI22:		/*  9 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = (*dest32 & ~0x003fffff) | ((symval >> 10) & 0x003fffff);
	break;

    case R_SPARC_22:		/* 10 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = (*dest32 & ~0x003fffff) | (symval & 0x003fffff);
	break;

    case R_SPARC_13:		/* 11 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = (*dest32 & ~0x001fff) | (symval & 0x001fff);
	break;

    case R_SPARC_LO10:		/* 12 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = (*dest32 & ~0x001fff) | (symval & 0x03ff);
	break;

    case R_SPARC_PC10:		/* 16 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = (*dest32 & ~0x001fff) |
		   ((symval - (Elf_Addr) dest32) & 0x03ff);
	break;

    case R_SPARC_PC22:		/* 17 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = (*dest32 & ~0x03fffff) |
		   (((symval - (Elf_Addr) dest32) >> 10) & 0x03fffff);
	break;

    case R_SPARC_COPY:		/* 19 */
	/* Fix your code...  I'd rather dish out an error here
	 * so people will not link together PIC and non-PIC
	 * code into a final driver object file.
	 */
	ErrorF("Elf_RelocateEntry():"
	       "  Copy relocs not supported on SPARC.\n");
	break;

    case R_SPARC_JMP_SLOT:	/* 21 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	/* Before we change it the PLT entry looks like:
	 *
	 * pltent:      sethi   %hi(rela_plt_offset), %g1
	 *              b,a     PLT0
	 *              nop
	 *
	 * We change it into:
	 *
	 * pltent:      sethi   %hi(rela_plt_offset), %g1
	 *              sethi   %hi(symval), %g1
	 *              jmp     %g1 + %lo(symval), %g0
	 */
	symval += rel->r_addend;
	dest32[2] = 0x81c06000 | (symval & 0x3ff);
	__asm __volatile("flush %0 + 0x8"::"r"(dest32));

	dest32[1] = 0x03000000 | (symval >> 10);
	__asm __volatile("flush %0 + 0x4"::"r"(dest32));

	break;

    case R_SPARC_RELATIVE:	/* 22 */
	dest64 = (unsigned long *)(secp + rel->r_offset);
	resetDest64(p, dest64);
	*dest64 += (unsigned long)secp + rel->r_addend;
	break;

    case R_SPARC_10:		/* 30 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = (*dest32 & ~0x03ff) | (symval & 0x03ff);
	break;

    case R_SPARC_11:		/* 31 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = (*dest32 & ~0x07ff) | (symval & 0x07ff);
	break;

    case R_SPARC_OLO10:		/* 33 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval = ((symval + rel->r_addend) & 0x03ff) +
		  (ELF64_R_TYPE(rel->r_info) >> 8);
	*dest32 = (*dest32 & ~0x1fff) | (symval & 0x1fff);
	break;

    case R_SPARC_HH22:		/* 34 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = (*dest32 & ~0x003fffff) |
		  /* Avoid warnings on SPARC32 */
		  (((symval >> 21) >> 21) & 0x003fffff);
	break;

    case R_SPARC_HM10:		/* 35 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = (*dest32 & ~0x001fff) |
		  /* Avoid warnings on SPARC32 */
		  (((symval >> 16) >> 16) & 0x03ff);
	break;

    case R_SPARC_LM22:		/* 36 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = (*dest32 & ~0x003fffff) | ((symval >> 10) & 0x003fffff);
	break;

    case R_SPARC_7:		/* 43 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = (*dest32 & ~0x007f) | (symval & 0x007f);
	break;

    case R_SPARC_5:		/* 44 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = (*dest32 & ~0x001f) | (symval & 0x001f);
	break;

    case R_SPARC_6:		/* 45 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = (*dest32 & ~0x003f) | (symval & 0x003f);
	break;

    case R_SPARC_HIX22:		/* 48 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = (*dest32 & ~0x003fffff) |
		  (((symval ^ ~0L) >> 10) & 0x003fffff);
	break;

    case R_SPARC_LOX10:		/* 49 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = (*dest32 & ~0x03ff) | 0x001c00 | (symval & 0x03ff);
	break;

    case R_SPARC_H44:		/* 50 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = (*dest32 & ~0x003fffff) | ((symval >> 22) & 0x003fffff);
	break;

    case R_SPARC_M44:		/* 51 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = (*dest32 & ~0x03ff) | ((symval >> 12) & 0x03ff);
	break;

    case R_SPARC_L44:		/* 52 */
	dest32 = (unsigned int *)(secp + rel->r_offset);
	symval += rel->r_addend;
	*dest32 = (*dest32 & ~0x1fff) | (symval & 0x0fff);
	break;

    case R_SPARC_UA64:		/* 54 */
	dest64 = (unsigned long *)(secp + rel->r_offset);
	symval += rel->r_addend;
	/* Avoid warnings on SPARC32 */
	((unsigned char *)dest64)[0] = (unsigned char)((symval >> 28) >> 28);
	((unsigned char *)dest64)[1] = (unsigned char)((symval >> 24) >> 24);
	((unsigned char *)dest64)[2] = (unsigned char)((symval >> 20) >> 20);
	((unsigned char *)dest64)[3] = (unsigned char)((symval >> 16) >> 16);
	((unsigned char *)dest64)[4] = (unsigned char)(symval >> 24);
	((unsigned char *)dest64)[5] = (unsigned char)(symval >> 16);
	((unsigned char *)dest64)[6] = (unsigned char)(symval >> 8);
	((unsigned char *)dest64)[7] = (unsigned char)(symval);
	break;

    case R_SPARC_UA16:		/* 55 */
	dest16 = (unsigned short *)(secp + rel->r_offset);
	symval += rel->r_addend;
	((unsigned char *)dest16)[0] = (unsigned char)(symval >> 8);
	((unsigned char *)dest16)[1] = (unsigned char)(symval);
	break;
#endif /*__sparc__*/

#ifdef __ia64__
    case R_IA64_NONE:
	break;

    case R_IA64_LTOFF_FPTR22:
	if (rel->r_addend)
	    FatalError("\nAddend for R_IA64_LTOFF_FPTR22 not supported\n");
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "opd=%016lx.%016lx\n",
		 ((long *)symval)[0], ((long *)symval)[1]);
#endif
	/* FALLTHROUGH */
    case R_IA64_LTOFF22:
#ifndef IA64_LDX_OPTIMIZATION
    case R_IA64_LTOFF22X:	/* If not implementing LDXMOV optimization treat LTOFF22X as LTOFF22 */
#endif
	{
	    ELFGotEntryPtr gotent;

	    dest128 = (unsigned long *)(secp + (rel->r_offset & ~3));
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		     "%s %s\t", ELF_R_TYPE(rel->r_info) == R_IA64_LTOFF_FPTR22 ?
		     "R_IA64_LTOFF_FPTR22" : "R_IA64_LTOFF22[X]",
		     ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%lx\t", secp);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest128=%lx\t", dest128);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "slot=%d\n", rel->r_offset & 3);
#endif
	    resetDest128(p, dest128);
	    for (gotent = elffile->got_entries; gotent; gotent = gotent->next) {
		if (ELF_R_SYM(gotent->rel->r_info) == ELF_R_SYM(rel->r_info)
		    && gotent->rel->r_addend == rel->r_addend)
		    break;
	    }

	    /* Set the address in the GOT */
	    if (gotent) {
		*(unsigned long *)(elffile->got + gotent->offset) =
			symval + rel->r_addend;
#if LOADERDEBUG
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "Setting gotent[%x]=%lx\n",
			 gotent->offset, symval + rel->r_addend);
#endif
		if ((gotent->offset & 0xffe00000) != 0)
		    FatalError("\nR_IA64_LTOFF22 offset %x too large\n",
			       gotent->offset);
		IA64InstallReloc(dest128, rel->r_offset & 3, IA64_OPND_IMM22,
				 gotent->offset);
	    } else
		FatalError("\nCould not find GOT entry\n");
	}
	break;

    case R_IA64_PCREL21B:
	{
	    ELFPltEntryPtr pltent;

	    dest128 = (unsigned long *)(secp + (rel->r_offset & ~3));
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_IA64_PCREL21B %s\t",
		     ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%lx\t", secp);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		     "opd=%lx.%lx\t", ((long *)symval)[0], ((long *)symval)[1]);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest128=%lx\t", dest128);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		     "slot=%d\n", rel->r_offset & 3);
#endif
	    resetDest128(p, dest128);
	    if (rel->r_addend)
		FatalError("\nAddend for PCREL21B not supported\n");
	    if ((((long *)symval)[1] == (long)elffile->got) &&
		(((unsigned long)dest128 - ((unsigned long *)symval)[0] +
		    0x2000000) < 0x4000000)) {
		/* We can save the travel through PLT */
		IA64InstallReloc(dest128, rel->r_offset & 3, IA64_OPND_TGT25C,
				 ((unsigned long *)symval)[0] -
				 (unsigned long)dest128);
		break;
	    }
	    for (pltent = elffile->plt_entries; pltent; pltent = pltent->next) {
		if (ELF_R_SYM(pltent->rel->r_info) == ELF_R_SYM(rel->r_info)
		    && pltent->rel->r_addend == rel->r_addend)
		    break;
	    }

	    /* Set the address in the PLT */
	    if (pltent == NULL)
		FatalError("\nCould not find PLT entry\n");
	    else {
		unsigned long *p =
			(unsigned long *)(elffile->plt + pltent->offset);
		unsigned long r =
			(unsigned long)symval - (unsigned long)elffile->got;

		if (r + 0x200000 >= 0x400000) {
		    /* Too far from gp to use the official function descriptor,
		     * so we have to make a local one.
		     */
		    r = pltent->gotoffset;
		    memcpy(elffile->got + r, (char *)symval, 16);
		}

		/* [MMI] addl r15=NNN,r1;; ld8 r16=[r15],8; mov r14=r1;; */
		p[0] = 0x410024000200780bUL;
		p[1] = 0x84000801c028303cUL;
		/* [MIB] ld8 r1=[r15]; mov b6=r16; br.few b6;; */
		p[2] = 0x806010181e000811UL;
		p[3] = 0x0080006000038004UL;
		IA64InstallReloc(p, 0, IA64_OPND_IMM22, r);
		IA64InstallReloc(dest128, rel->r_offset & 3, IA64_OPND_TGT25C,
				 (unsigned long)p - (unsigned long)dest128);
	    }
	}
	break;

    case R_IA64_FPTR64LSB:
	dest64 = (unsigned long *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_IA64_FPTR64LSB %s\t",
		 ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%lx\t", secp);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest64=%lx\t", dest64);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "opd=%016lx.%016lx\n",
		 ((long *)symval)[0], ((long *)symval)[1]);
#endif

	if (rel->r_addend)
	    FatalError("\nAddend not supported for R_IA64_FPTR64LSB\n");
	*dest64 = symval;
	ia64_flush_cache(dest64);
	break;

    case R_IA64_DIR64LSB:
	dest64 = (unsigned long *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_IA64_DIR64LSB %s\t",
		 ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%lx\t", secp);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest64=%lx\n", dest64);
#endif
	*dest64 = symval + rel->r_addend;
	ia64_flush_cache(dest64);
	break;

    case R_IA64_PCREL64LSB:
	dest64 = (unsigned long *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_IA64_PCREL64LSB %s\t",
		 ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%lx\t", secp);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest64=%lx\n", dest64);
#endif
	*dest64 = symval + rel->r_addend - (unsigned long)dest64;
	break;

    case R_IA64_GPREL22:
	dest128 = (unsigned long *)(secp + (rel->r_offset & ~3));
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_IA64_GPREL22 %s\t",
		 ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%lx\t", secp);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest128=%lx\t", dest128);
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "slot=%d\n", rel->r_offset & 3);
#endif
	resetDest128(p, dest128);
	IA64InstallReloc(dest128, rel->r_offset & 3, IA64_OPND_IMM22,
			 symval + rel->r_addend - (long)elffile->got);
	break;

#ifdef IA64_LDX_OPTIMIZATION
    case R_IA64_LTOFF22X:
	{
	    ELFGotEntryPtr gotent;
	    long gp_offset = symval + rel->r_addend - (long)elffile->got;

	    dest128 = (unsigned long *)(secp + (rel->r_offset & ~3));
	    resetDest128(p, dest128);
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_IA64_LTOFF22X %s\t",
		     ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%lx\t", secp);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest128=%lx\t", dest128);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		     "slot=%d\n", rel->r_offset & 3);
#endif

	    if (gp_offset << 42 >> 42 != gp_offset) {
		/* Offset is too large for LTOFF22X,
		 * fallback to using GOT lookup, e.g. LTOFF22.
		 * Note: LDXMOV will fail the same test and will be ignored. */

#if LOADERDEBUG
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
			"gp_offset=%ld too large, using GOT instead "
			"(LTOFF22)\n", gp_offset);
#endif

		for (gotent = elffile->got_entries; gotent;
		     gotent = gotent->next) {
		    if (ELF_R_SYM(gotent->rel->r_info) ==
			ELF_R_SYM(rel->r_info)
			&& gotent->rel->r_addend == rel->r_addend)
			break;
		}

		/* Set the address in the GOT */
		if (gotent) {
		    *(unsigned long *)(elffile->got + gotent->offset) =
			    symval + rel->r_addend;
#if LOADERDEBUG
		    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
			     "Setting gotent[%x]=%lx\n", gotent->offset,
			     symval + rel->r_addend);
#endif
		    if ((gotent->offset & 0xffe00000) != 0)
			FatalError("\nR_IA64_LTOFF22 offset %x too large\n",
				   gotent->offset);
		} else {
		    FatalError("\nCould not find GOT entry\n");
		}
		gp_offset = gotent->offset;	/* Use GOT lookup */
	    } else {
#if LOADERDEBUG
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
			"using gp_offset=%ld (LTOFF22X)", gp_offset);
#endif
	    }
	    IA64InstallReloc(dest128, rel->r_offset & 3, IA64_OPND_IMM22,
			     gp_offset);
	}
	break;
#endif

    case R_IA64_LDXMOV:
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_IA64_LDXMOV %s\t",
		 ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)));
#endif

#ifdef IA64_LDX_OPTIMIZATION
	{
	    long gp_offset = symval + rel->r_addend - (long)elffile->got;

	    dest128 = (unsigned long *)(secp + (rel->r_offset & ~3));
	    resetDest128(p, dest128);
	    if (gp_offset << 42 >> 42 != gp_offset) {
		/* Offset is too large for LTOFF22X, ignore this relocation */
#if LOADERDEBUG
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
			"offset = %ld too large, ignoring\n", gp_offset);
#endif
	    } else {

#if LOADERDEBUG
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%lx\t", secp);
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest128=%lx\t", dest128);
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
			"slot=%d\t", rel->r_offset & 3);
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
			"offset=%ld\t", gp_offset);
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
			"*dest128=[%016lx%016lx]\n", dest128[1], dest128[0]);
#endif

		IA64InstallReloc(dest128, rel->r_offset & 3, IA64_OPND_LDXMOV,
				 0);
	    }
	}
#endif
	break;

#endif /*__ia64__*/

#if defined(__arm__)
    case R_ARM_ABS32:
	dest32 = (unsigned int *)(secp + rel->r_offset);
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_ARM_ABS32\t");
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32=%x\t", dest32);
#endif
	resetDest32(p, dest32);
	*dest32 = symval + *dest32;
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32=%8.8lx\n", *dest32);
#endif
#if defined(__NetBSD__)
	arm_sync_icache(dest32, 4);
#endif
	break;

    case R_ARM_REL32:
	dest32 = (unsigned int *)(secp + rel->r_offset);
#if LOADERDEBUG
	{
	    char *namestr = ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info));

	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "R_ARM_REL32 %s\t", namestr);
	    xf86loaderfree(namestr);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "secp=%x\t", secp);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "symval=%lx\t", symval);
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "dest32=%x\t", dest32);
	}
#endif
	resetDest32(p, dest32);
	*dest32 = symval + *dest32 - (Elf_Addr) dest32;

#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "*dest32=%8.8lx\n", *dest32);
#endif
#if defined(__NetBSD__)
	arm_sync_icache(dest32, 4);
#endif
	break;

    case R_ARM_PC24:
	{
	    unsigned long val;

	    dest32 = (unsigned int *)(secp + rel->r_offset);
	    resetDest32(p, dest32);
	    val = (*dest32 & 0x00ffffff) << 2;
	    val = symval - (unsigned long)dest32 + val;
	    val >>= 2;
	    *dest32 = (*dest32 & 0xff000000) | (val & 0x00ffffff);
#ifdef NOTYET
	    arm_flush_cache(dest32);
#else
#if defined(__NetBSD__)
	    arm_sync_icache(dest32, 4);
#endif
#endif
	}
	break;

#endif /* (__arm__) */

    default:
	ErrorF("Elf_RelocateEntry() \"%s\" Unsupported relocation "
	       "type %d (0x%x)\n",
	       ElfGetSymbolName(elffile, ELF_R_SYM(rel->r_info)),
	       (int)ELF_R_TYPE(rel->r_info), (int)ELF_R_TYPE(rel->r_info));
	break;
    }
    return invariant;
}

static ELFRelocPtr
ELFCollectRelocations(ELFModulePtr elffile, int index)
{
    int i, numrel;
    Elf_Shdr *sect = &(elffile->sections[index]);
    Elf_Rel_t *rel = (Elf_Rel_t *) elffile->saddr[index];
    ELFRelocPtr reloc_head = NULL;
    ELFRelocPtr tmp;

    numrel = sect->sh_size / sect->sh_entsize;

#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		"Collecting relocations for file %p (%s)\n", elffile,
		_LoaderHandleToCanonicalName(elffile->handle));
#endif
    for (i = 0; i < numrel; i++) {
#if defined(__alpha__)
	if (ELF_R_TYPE(rel[i].r_info) == R_ALPHA_LITERAL) {
	    ElfAddGOT(elffile, &rel[i]);
	}
#endif
#if defined(__ia64__)
	if (ELF_R_TYPE(rel[i].r_info) == R_IA64_LTOFF22
	    || ELF_R_TYPE(rel[i].r_info) == R_IA64_LTOFF22X
	    || ELF_R_TYPE(rel[i].r_info) == R_IA64_LTOFF_FPTR22) {
	    ElfAddGOT(elffile, &rel[i]);
	}
	if (ELF_R_TYPE(rel[i].r_info) == R_IA64_PCREL21B) {
	    ElfAddPLT(elffile, &rel[i]);
	}
	if (ELF_R_TYPE(rel[i].r_info) == R_IA64_LTOFF_FPTR22
	    || ELF_R_TYPE(rel[i].r_info) == R_IA64_FPTR64LSB) {
	    Elf_Sym *syms = (Elf_Sym *) elffile->saddr[elffile->symndx];

	    if (ELF_ST_BIND(syms[ELF_R_SYM(rel[i].r_info)].st_info) ==
		STB_LOCAL) {
		ElfAddOPD(elffile, ELF_R_SYM(rel[i].r_info), NULL);
	    }
	}
#endif
	tmp = ElfDelayRelocation(elffile, sect->sh_info, &(rel[i]));
	tmp->next = reloc_head;
	reloc_head = tmp;
    }

    return reloc_head;
}

/*
 * ELF_GetSymbols()
 *
 * add the symbols to the symbol table maintained by the loader.
 */

static LOOKUP *
ELF_GetSymbols(ELFModulePtr elffile, unsigned short **psecttable)
{
    Elf_Sym *syms;
    Elf_Shdr *sect;
    int i, l, numsyms;
    LOOKUP *lookup, *p;
    ELFCommonPtr tmp;
    unsigned short *secttable;

    syms = elffile->symtab;
    sect = &(elffile->sections[elffile->symndx]);
    numsyms = sect->sh_size / sect->sh_entsize;

    if ((lookup = xf86loadermalloc((numsyms + 1) * sizeof(LOOKUP))) == NULL)
	return 0;

    if ((secttable =
	 xf86loadercalloc(sizeof(unsigned short), (numsyms + 1))) == NULL) {
	xf86loaderfree(lookup);
	return 0;
    }
    *psecttable = secttable;

    for (i = 0, l = 0; i < numsyms; i++) {
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		 "value=%lx\tsize=%lx\tBIND=%x\tTYPE=%x\tndx=%x\t%s\n",
		 (unsigned long)syms[i].st_value,
		 (unsigned long)syms[i].st_size,
		 ELF_ST_BIND(syms[i].st_info), ELF_ST_TYPE(syms[i].st_info),
		 syms[i].st_shndx, ElfGetString(elffile, syms[i].st_name));
#endif

	if (ELF_ST_BIND(syms[i].st_info) == STB_LOCAL)
	    /* Don't add static symbols to the symbol table */
	    continue;

	switch (ELF_ST_TYPE(syms[i].st_info)) {
	case STT_OBJECT:
	case STT_FUNC:
	case STT_SECTION:
	case STT_NOTYPE:
	    switch (syms[i].st_shndx) {
	    case SHN_ABS:
		ErrorF("ELF_GetSymbols() Don't know how to handle SHN_ABS\n");
		break;
	    case SHN_COMMON:
#if LOADERDEBUG
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
			 "Adding COMMON space for %s\n",
			 ElfGetString(elffile, syms[i].st_name));
#endif
		if (!LoaderHashFind(ElfGetString(elffile, syms[i].st_name))) {
		    tmp = ElfAddCOMMON(&(syms[i]));
		    if (tmp) {
			tmp->next = listCOMMON;
			listCOMMON = tmp;
		    }
		}
		break;
	    case SHN_UNDEF:
		/*
		 * UNDEF will get resolved later, so the value
		 * doesn't really matter here.
		 */
		/* since we don't know the value don't advertise the symbol */
		break;
	    default:
		lookup[l].symName =
			xf86loaderstrdup(ElfGetString
					 (elffile, syms[i].st_name));
		lookup[l].offset = (funcptr) syms[i].st_value;
		secttable[l] = syms[i].st_shndx;
#if LOADERDEBUG
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
			 "Adding symbol %lx(%d) %s\n",
			 (unsigned long)lookup[l].offset, secttable[l],
			 lookup[l].symName);
#endif
#ifdef __ia64__
		if (ELF_ST_TYPE(syms[i].st_info) == STT_FUNC) {
		    ElfAddOPD(elffile, -1, &lookup[l]);
		}
#endif
		l++;
		break;
	    }
	    break;
	case STT_FILE:
	case STT_LOPROC:
	case STT_HIPROC:
	    /* Skip this type */
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "Skipping TYPE %d %s\n",
		     ELF_ST_TYPE(syms[i].st_info),
		     ElfGetString(elffile, syms[i].st_name));
#endif
	    break;
	default:
	    ErrorF("ELF_GetSymbols(): Unepected symbol type %d\n",
		   ELF_ST_TYPE(syms[i].st_info));
	    break;
	}
    }

    lookup[l].symName = NULL;	/* Terminate the list */

    /*
     * Remove the ELF symbols that will show up in every object module.
     */
    for (i = 0, p = lookup; p->symName; i++, p++) {
	while (!strcmp(lookup[i].symName, ".text")
	       || !strcmp(lookup[i].symName, ".data")
	       || !strcmp(lookup[i].symName, ".bss")
	       || !strcmp(lookup[i].symName, ".comment")
	       || !strcmp(lookup[i].symName, ".note")
		) {
	    memmove(&(lookup[i]), &(lookup[i + 1]), (l - i) * sizeof(LOOKUP));
	    memmove(&(secttable[i]), &(secttable[i + 1]),
		    (l-- - i) * sizeof(unsigned short));
	}
    }
    return lookup;
}

#define SecOffset(index) elffile->sections[index].sh_offset
#define SecSize(index) elffile->sections[index].sh_size
#define SecAlign(index) elffile->sections[index].sh_addralign
#define SecType(index) elffile->sections[index].sh_type
#define SecFlags(index) elffile->sections[index].sh_flags
#define SecInfo(index) elffile->sections[index].sh_info

#define AdjustSize(i)				\
    if (!pass) {				\
	if (SecAlign(i) > *maxalign)		\
	    *maxalign = SecAlign(i);		\
	*totalsize += (SecAlign(i) - 1);	\
	*totalsize &= ~(SecAlign(i) - 1);	\
	*totalsize += SecSize(i);		\
	continue;				\
    } do { } while (0)

/*
 * ELFCollectSections
 *
 * Do the work required to load each section into memory.
 */
static void
ELFCollectSections(ELFModulePtr elffile, int pass, int *totalsize,
		   int *maxalign)
{
    int i;
    int j;

    /*
     * Find and identify all of the Sections
     */
    j = elffile->lsectidx;
    for (i = 1; i < elffile->numsh; i++) {
	int flags = 0;
	char *name = ElfGetSectionName(elffile, elffile->sections[i].sh_name);

#if defined(__alpha__) || defined(__ia64__)
	if (!strcmp(name, ".got")	/*Isn't there a more generic way to do this? */
# if defined(__ia64__)
	    || !strcmp(name, ".plt") || !strcmp(name, ".IA_64.unwind_info")
# endif
		)
	    continue;
#endif
	switch (SecType(i)) {
	case SHT_STRTAB:
	    if (!strcmp(name, ".shstrtab"))	/* already loaded */
		continue;
	    if (!strcmp(name, ".stabstr"))	/* ignore debug info */
		continue;
	    if (!strcmp(name, ".stab.indexstr")) /* ignore debug info */
		continue;
	case SHT_SYMTAB:
	    if (pass)
		continue;
	    flags = LOADED_SECTION;
	    flags |= RELOC_SECTION;
	    break;
	case SHT_REL:
	case SHT_RELA:
	    if (pass)
		continue;
	    if (!(SecFlags(SecInfo(i)) & SHF_ALLOC))
		continue;
#ifdef __ia64__
	    if (SecType(SecInfo(i)) == SHT_IA_64_UNWIND)
		continue;
#endif
	    flags = LOADED_SECTION;
	    flags |= RELOC_SECTION;
	    break;
	case SHT_PROGBITS:
	    flags |= LOADED_SECTION;
	case SHT_NOBITS:
	    if (!(elffile->sections[i].sh_flags & SHF_ALLOC))
		continue;
	    AdjustSize(i);
	    break;
	default:
#if LOADERDEBUG
	    if (pass)
		LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "ELF: Not loading %s\n", name);
#endif
	    continue;
	}

	elffile->lsection = xf86loaderrealloc(elffile->lsection,
					      (j + 1) * sizeof(LoadSection));
	if (!(flags & RELOC_SECTION)) {
	    if (flags & LOADED_SECTION) {
		elffile->lsection[j].saddr	/* sect. contains data */
			= ELFLoaderSectToMem(elffile, SecAlign(i),
					     SecOffset(i), SecSize(i), name);
	    } else {
		if (SecSize(i))
		    elffile->lsection[j].saddr
			    = ELFLoaderSectCalloc(elffile, SecAlign(i),
						  SecSize(i));
		else
		    elffile->lsection[j].saddr = NULL;
	    }
	} else {
	    elffile->lsection[j].saddr =
		    (Elf_Sym *) _LoaderFileToMem(elffile->fd, SecOffset(i),
						 SecSize(i), name);
	}
	elffile->saddr[i] = elffile->lsection[j].saddr;
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "%s starts at %p size: %lx\n",
		 name, elffile->saddr[i], (unsigned long)SecSize(i));
#endif
	elffile->lsection[j].name = name;
	elffile->lsection[j].ndx = i;
	elffile->lsection[j].size = SecSize(i);
	elffile->lsection[j].flags = flags;
	switch (SecType(i)) {
#if defined(__OpenBSD__) || defined(__NetBSD__)
	case SHT_PROGBITS:
	    mprotect(elffile->lsection[j].saddr, SecSize(i),
		     PROT_READ | PROT_WRITE | PROT_EXEC);
	    break;
#endif
	case SHT_SYMTAB:
	    elffile->symtab = (Elf_Sym *) elffile->saddr[i];
	    elffile->symndx = i;
	    break;
	case SHT_STRTAB:
	    elffile->straddr = elffile->saddr[i];
	    elffile->strsize = elffile->lsection[j].size;
	    elffile->strndx = i;
	    break;
	default:
	    break;
	}
	elffile->lsectidx = ++j;
    }
}

/*
 * Public API for the ELF implementation of the loader.
 */
void *
ELFLoadModule(loaderPtr modrec, int elffd, LOOKUP **ppLookup)
{
    ELFModulePtr elffile;
    Elf_Ehdr *header;
    ELFRelocPtr elf_reloc, tail;
    LDRModulePtr elfmod;
    int totalsize, maxalign, i;
    unsigned short *secttable = NULL;
    LOOKUP *pLookup;

    ldrCommons = 0;
    nCommons = 0;

#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_FILES,
		   "Loading %s %s\n", modrec->name, modrec->cname);
#endif

    if ((elffile = xf86loadercalloc(1, sizeof(ELFModuleRec))) == NULL) {
	ErrorF("Unable to allocate ELFModuleRec\n");
	return NULL;
    }

    elffile->handle = modrec->handle;
    elffile->module = modrec->module;
    elffile->fd = elffd;
    elffile->desc = modrec->desc;

    /*
     *  Get the ELF header
     */
    elffile->header =
	    (Elf_Ehdr *) _LoaderFileToMem(elffd, 0, sizeof(Elf_Ehdr),
					  "header");
    header = (Elf_Ehdr *) elffile->header;

    /*
     * Get the section table
     */
    elffile->numsh = header->e_shnum;
    elffile->secsize = (header->e_shentsize * header->e_shnum);
    elffile->sections =
	    (Elf_Shdr *) _LoaderFileToMem(elffd, header->e_shoff,
					  elffile->secsize, "sections");
#if defined(__alpha__) || defined(__ia64__)
    /*
     * Need to allocate space for the .got section which will be
     * fabricated later
     */
    elffile->gotndx = header->e_shnum;
    header->e_shnum++;
# if defined(__ia64__)
    elffile->pltndx = header->e_shnum;
    header->e_shnum++;
# endif
    elffile->numsh = header->e_shnum;
    elffile->secsize = (header->e_shentsize * header->e_shnum);
    elffile->sections =
	    xf86loaderrealloc(elffile->sections, elffile->secsize);
#endif /*defined(__alpha__) || defined(__ia64__) */
    elffile->saddr =
	    xf86loadercalloc(elffile->numsh, sizeof(unsigned char *));

#if defined(__alpha__) || defined(__ia64__)
    /*
     * Manually fill in the entry for the .got section so ELFCollectSections()
     * will be able to find it.
     */
    elffile->sections[elffile->gotndx].sh_name =
	    SecSize(header->e_shstrndx) + 1;
    elffile->sections[elffile->gotndx].sh_type = SHT_PROGBITS;
    elffile->sections[elffile->gotndx].sh_flags = SHF_WRITE | SHF_ALLOC;
    elffile->sections[elffile->gotndx].sh_size = 0;
    elffile->sections[elffile->gotndx].sh_addralign = 8;
    /* Add room to copy ".got", and maintain alignment */
    SecSize(header->e_shstrndx) += 8;
#endif
#if defined(__ia64__)
    /*
     * Manually fill in the entry for the .plt section so ELFCollectSections()
     * will be able to find it.
     */
    elffile->sections[elffile->pltndx].sh_name =
	    SecSize(header->e_shstrndx) + 1;
    elffile->sections[elffile->pltndx].sh_type = SHT_PROGBITS;
    elffile->sections[elffile->pltndx].sh_flags = SHF_EXECINSTR | SHF_ALLOC;
    elffile->sections[elffile->pltndx].sh_size = 0;
    elffile->sections[elffile->pltndx].sh_addralign = 32;
    /* Add room to copy ".plt", and maintain alignment */
    SecSize(header->e_shstrndx) += 32;
#endif

    /*
     * Get the section header string table
     */
    elffile->shstrsize = SecSize(header->e_shstrndx);
    elffile->shstraddr =
	    _LoaderFileToMem(elffd, SecOffset(header->e_shstrndx),
			     SecSize(header->e_shstrndx), ".shstrtab");
    elffile->shstrndx = header->e_shstrndx;
#if defined(__alpha__) || defined(__ia64__)
    /*
     * Add the string for the .got section
     */
    strcpy((char *)(elffile->shstraddr +
		    elffile->sections[elffile->gotndx].sh_name), ".got");
#endif
#if defined(__ia64__)
    /*
     * Add the string for the .plt section
     */
    strcpy((char *)(elffile->shstraddr +
		    elffile->sections[elffile->pltndx].sh_name), ".plt");
#endif

    /*
     * Load some desired sections, compute size of the remaining ones
     */
    totalsize = 0;
    maxalign = 0;
    ELFCollectSections(elffile, 0, &totalsize, &maxalign);
    if (elffile->straddr == NULL || elffile->strsize == 0) {
#if 0
	ErrorF("No symbols found in this module\n");
#endif
	ELFUnloadModule(elffile);
	return (void *)-1L;
    }
    /*
     * add symbols
     */
    *ppLookup = pLookup = ELF_GetSymbols(elffile, &secttable);

    /*
     * Do relocations
     */
    for (i = 0; i < elffile->lsectidx; i++) {
	switch (SecType(elffile->lsection[i].ndx)) {
	case SHT_REL:
	case SHT_RELA:
	    break;
	default:
	    continue;
	}
	elf_reloc = ELFCollectRelocations(elffile, elffile->lsection[i].ndx);
	if (elf_reloc) {
	    for (tail = elf_reloc; tail->next; tail = tail->next) ;
	    tail->next = *_LoaderGetRelocations(elffile->desc);
	    *_LoaderGetRelocations(elffile->desc) = elf_reloc;
	}
    }

#if defined(__ia64__)
    totalsize += (elffile->sections[elffile->pltndx].sh_addralign - 1);
    totalsize &= ~(elffile->sections[elffile->pltndx].sh_addralign - 1);
    totalsize += elffile->pltsize;
    if (maxalign < elffile->sections[elffile->pltndx].sh_addralign)
	maxalign = elffile->sections[elffile->pltndx].sh_addralign;
#endif

    /* Space for COMMON */
    totalsize = (totalsize + 7) & ~7;
    totalsize += ElfCOMMONSize();

#if defined(__powerpc__)
    totalsize = (totalsize + 7) & ~7;
    elffile->numplt = ELFGetNumPLTEntries(elffile);
    totalsize += elffile->numplt * sizeof(pltentry);
#endif

#ifdef MergeSectionAlloc
    elffile->basesize = totalsize + maxalign;

# if !defined(DoMMAPedMerge)
    elffile->base = xf86loadermalloc(elffile->basesize);
    if (elffile->base == NULL) {
	ErrorF("Unable to allocate ELF sections\n");
	return NULL;
    }
#  if (defined(linux) && defined(__ia64__)) || \
      defined(__OpenBSD__) || defined(__NetBSD__)
    {
	unsigned long page_size = getpagesize();
	unsigned long round;

	round = (unsigned long)elffile->base & (page_size - 1);
	mprotect(elffile->base - round,
		 (elffile->basesize + round + page_size - 1) & ~(page_size -
								 1),
		 PROT_READ | PROT_WRITE | PROT_EXEC);
    }
#  endif
# else
    MMAP_ALIGN(elffile->basesize);
    elffile->base = mmap(0, elffile->basesize, MMAP_PROT, MMAP_FLAGS, -1,
			 (off_t) 0);
    if (elffile->base == NULL) {
	ErrorF("Unable to mmap ELF sections\n");
	return NULL;
    }
# endif
    elffile->baseptr =
	    ((long)elffile->base + (maxalign - 1)) & ~(maxalign - 1);
#endif

#if defined(__alpha__) || defined(__ia64__)
    if (!ELFCreateGOT(elffile, maxalign))
	return NULL;
#endif
#if defined(__ia64__) || defined(__powerpc__)
    ELFCreatePLT(elffile);
#endif

    ELFCollectSections(elffile, 1, NULL, NULL);

    for (i = 0; pLookup[i].symName; i++)
	if (secttable[i]) {
	    pLookup[i].offset =
		    (funcptr) ((long)pLookup[i].offset +
			       (long)elffile->saddr[secttable[i]]);
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "Finalizing symbol %p %s\n",
		     (void *)pLookup[i].offset, pLookup[i].symName);
#endif
	}
    xf86loaderfree(secttable);

#if defined(__ia64__)
    ELFCreateOPD(elffile);
#endif

    if (!ElfCreateCOMMON(elffile, *ppLookup))
	return NULL;

    /* Record info for gdb - if we can't allocate the loader record fail
     * silently (the user will find out soon enough that there's no VM left */
    if ((elfmod = xf86loadercalloc(1, sizeof(LDRModuleRec))) != NULL) {
	elfmod->name = strdup(modrec->name);
	elfmod->namelen = strlen(modrec->name);
	elfmod->version = 1;
	for (i = 0; i < elffile->lsectidx; i++) {
	    char *name = elffile->lsection[i].name;

	    if (!strcmp(name, ".text"))
		elfmod->text = elffile->lsection[i].saddr;
	    else if (!strcmp(name, ".data"))
		elfmod->data = elffile->lsection[i].saddr;
	    else if (!strcmp(name, ".rodata"))
		elfmod->rodata = elffile->lsection[i].saddr;
	    else if (!strcmp(name, ".bss"))
		elfmod->bss = elffile->lsection[i].saddr;
	}
	elfmod->next = ModList;
	elfmod->commons = ldrCommons;
	elfmod->commonslen = nCommons;

	ModList = elfmod;

	/* Tell GDB something interesting happened */
	_loader_debug_state();
    }
    return (void *)elffile;
}

void
ELFResolveSymbols(LoaderDescPtr desc, int handle)
{
    ELFRelocPtr p, *pp, tmp;

    /* Try to relocate everything. */
    pp = (ELFRelocPtr *)_LoaderGetRelocations(desc);
    for (p = *_LoaderGetRelocations(desc); p;) {

	/* If handle is valid, only relocate symbols for that module. */
	if (handle >= 0 && p->file->handle != handle) {
	    pp = &(p->next);
	    p = p->next;
	    continue;
	}

#if LOADERDEBUG
	{
	    char *modname = _LoaderHandleToCanonicalName(p->file->handle);
	    if (modname) {
		LoaderDebugMsg(LOADER_DEBUG_REPORT_RELOC,
			       "ResolvedSymbols: module %s, file %p sec %d, "
			       "r_offset 0x%x, r_info %p (%s)\n",
			       modname, (void *)p->file, p->secn,
			       (int)p->rel->r_offset, (void *)p->rel->r_info,
			       ElfGetSymbolName(p->file,
						ELF_R_SYM(p->rel->r_info)));
	    }
	}
#endif

	if (Elf_RelocateEntry(p)) {
	    /*
	     * Remove invariant relocations, since they can't change when
	     * other modules are loaded or unloaded.
	     */
	    *pp = p->next;
	    tmp = p;
	    p = p->next;
	    xf86loaderfree(tmp);
	} else {
#if LOADERDEBUG
	    {
		char *modname = _LoaderHandleToCanonicalName(p->file->handle);
		if (modname) {
		    LoaderDebugMsg(LOADER_DEBUG_REPORT_RELOC,
				   "Symbol %s %s resolved\n",
				   ElfGetSymbolName(p->file,
						    ELF_R_SYM(p->rel->r_info)),
				   p->relocated ? "is" : "is not");
		}
	    }
#endif
	    pp = &(p->next);
	    p = p->next;
	}
    }
}

const char *
ELFFindRelocName(LoaderDescPtr desc, int handle, unsigned long addr)
{
    ELFRelocPtr p;
    long diff;

    for (p = *_LoaderGetRelocations(desc); p;) {
	/* If handle is valid, only look for symbols for that module. */
	if (handle >= 0 && p->file->handle != handle) {
	    p = p->next;
	    continue;
	}
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_REPORT_RELOC, "FindRelocName: "
		 "file %p, sec %d, secaddr 0x%lx, r_offset 0x%x, r_info %p\n",
		 (void *)p->file, p->secn,
		 (unsigned long)p->file->saddr[p->secn], (int)p->rel->r_offset,
		 (void *)p->rel->r_info);
#endif

	diff = addr - ((unsigned long)p->file->saddr[p->secn] +
		       (unsigned long)p->rel->r_offset);
	if (diff < 8 && diff > -8) {
	    return ElfGetSymbolName(p->file, ELF_R_SYM(p->rel->r_info));
	}
	p = p->next;
    }
    return NULL;
}

int
ELFCheckForUnresolved(LoaderDescPtr desc)
{
    ELFRelocPtr erel;
    const char *name;
    int fatalsym = 0;

    for (erel = *_LoaderGetRelocations(desc); erel; erel = erel->next) {
	if (erel->relocated) {
#if LOADERDEBUG
	    LoaderDebugMsg(LOADER_DEBUG_REPORT_RELOC,
		   "Module \"%s\" has relocation for %s\n",
		   _LoaderHandleToCanonicalName(erel->file->handle),
		   ElfGetSymbolName(erel->file, ELF_R_SYM(erel->rel->r_info)));
#endif
	    continue;
	}
	name = ElfGetSymbolName(erel->file, ELF_R_SYM(erel->rel->r_info));
	if ( _LoaderHandleUnresolved(name, erel->file->handle))
	    fatalsym = 1;
    }
    return fatalsym;
}

void
ELFUnloadModule(void *modptr)
{
    ELFModulePtr elffile = (ELFModulePtr) modptr;
    ELFRelocPtr relptr, reltptr, *brelptr;
    int i;

    /*
     * Delete our relocations.
     */

#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		"Deleting relocations for module %s\n",
		_LoaderHandleToCanonicalName(elffile->handle));
#endif

    relptr = *_LoaderGetRelocations(elffile->desc);
    brelptr = (ELFRelocPtr *)_LoaderGetRelocations(elffile->desc);

    while (relptr) {
#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		"relptr %p, relptr->file %p, elffile %p, relptrname %s, "
		"elffile name %s\n",
		(void *)relptr, (void *)relptr->file, (void *)elffile,
		_LoaderHandleToCanonicalName(relptr->file->handle),
		_LoaderHandleToCanonicalName(elffile->handle));
#endif
	if (relptr->file == elffile) {
	    *brelptr = relptr->next;	/* take it out of the list */
	    reltptr = relptr;		/* save pointer to this node */
	    relptr = relptr->next;	/* advance the pointer */
	    xf86loaderfree(reltptr);	/* free the node */
	} else {
	    brelptr = &(relptr->next);
	    relptr = relptr->next;	/* advance the pointer */
	}
    }

    /*
     * Delete any symbols in the symbols table.
     */

#if LOADERDEBUG
    LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL, "Deleting symbols for module %s\n",
		_LoaderHandleToCanonicalName(elffile->handle));
#endif
    LoaderHashTraverse((void *)elffile, ELFhashCleanOut);

    /*
     * Free the sections that were allocated.
     */
#if !defined(DoMMAPedMerge)
# define CheckandFree(ptr,size)  if(ptr) xf86loaderfree(ptr)
#else
# define CheckandFree(ptr,size) if (ptr) munmap(ptr,size)
#endif
#define CheckandFreeFile(ptr,size)  if(ptr) _LoaderFreeFileMem((ptr),(size))

#ifdef MergeSectionAlloc
    CheckandFree(elffile->base, elffile->basesize);
# if defined(__alpha__) || defined(__ia64__)
    if (elffile->shared_got) {
	elffile->shared_got->nuses--;
	if (!elffile->shared_got->nuses) {
	    ELFGotPtr *pgot = &ELFSharedGOTs;

	    while (*pgot && *pgot != elffile->shared_got)
		pgot = &(*pgot)->next;
	    if (*pgot)
		*pgot = elffile->shared_got->next;
	    xf86loaderfree(elffile->shared_got);
	}
    }
# endif
#else /*MergeSectionAlloc */
    CheckandFree(elffile->common, elffile->comsize);
# if defined(__alpha__) || defined(__ia64__)
    CheckandFree(elffile->got, elffile->gotsize);
# endif
# if defined(__ia64__)
    CheckandFree(elffile->plt, elffile->pltsize);
# endif
#endif
#if defined(__alpha__) || defined(__ia64__)
    {
	ELFGotEntryPtr gotent;

	while ((gotent = elffile->got_entries)) {
	    elffile->got_entries = gotent->next;
	    xf86loaderfree(gotent);
	}
    }
#endif
#if defined(__ia64__)
    {
	ELFPltEntryPtr pltent;

	while ((pltent = elffile->plt_entries)) {
	    elffile->plt_entries = pltent->next;
	    xf86loaderfree(pltent);
	}
    }
    {
	ELFOpdPtr opdent;

	while ((opdent = elffile->opd_entries)) {
	    elffile->opd_entries = opdent->next;
	    xf86loaderfree(opdent);
	}
    }
#endif

    for (i = 0; i < elffile->lsectidx; i++) {
#ifdef MergeSectionAlloc
	if (!(elffile->lsection[i].flags & RELOC_SECTION))
	    continue;
#endif
	if (elffile->lsection[i].flags & LOADED_SECTION) {
	    CheckandFreeFile(elffile->lsection[i].saddr,
			     elffile->lsection[i].size);
	} else {
	    CheckandFree(elffile->lsection[i].saddr,
			 elffile->lsection[i].size);
	}
    }
    xf86loaderfree(elffile->lsection);

    /*
     * Free the section table, section pointer array, and section names
     */
    _LoaderFreeFileMem(elffile->sections, elffile->secsize);
    xf86loaderfree(elffile->saddr);
    _LoaderFreeFileMem(elffile->header, sizeof(Elf_Ehdr));
    _LoaderFreeFileMem(elffile->shstraddr, elffile->shstrsize);

    /*
     * Free the ELFModuleRec
     */
    xf86loaderfree(elffile);

    return;
}

static int
ELF_AddressToSectNum(ELFModulePtr elffile, unsigned long address)
{
    int i;

    for (i = 1; i < elffile->numsh; i++) {
	if (address >= (unsigned long)elffile->saddr[i] &&
	    address <= (unsigned long)elffile->saddr[i] + SecSize(i)) {
	    return i;
	}
    }
    return 0;
}

char *
ELFAddressToSection(void *modptr, unsigned long address)
{
    ELFModulePtr elffile;
    int i;

    if (!modptr)
	return NULL;
    else
	elffile = (ELFModulePtr)modptr;

    for (i = 1; i < elffile->numsh; i++) {
	if (address >= (unsigned long)elffile->saddr[i] &&
	    address <= (unsigned long)elffile->saddr[i] + SecSize(i)) {
	    return ElfGetSectionName(elffile, elffile->sections[i].sh_name);
	}
    }
    return NULL;
}

/*
 * ELFAddressToSymbol
 */

const char *
ELFAddressToSymbol(void *modptr, unsigned long address, unsigned long *symaddr,
		   const char **filename, int exe)
{
    ELFModulePtr elffile;
    Elf_Sym *syms;
    Elf_Shdr *sect;
    int i, numsyms;
    long bestDiff = MAXINT, diff;
    const char *best = NULL;
    unsigned long saddr, bestAddr = 0;
    int sectnum;

    if (!modptr)
	return NULL;
    else
	elffile = (ELFModulePtr)modptr;

    syms = elffile->symtab;
    sect = &(elffile->sections[elffile->symndx]);
    numsyms = sect->sh_size / sect->sh_entsize;

    sectnum = ELF_AddressToSectNum(elffile, address);

    if (!sectnum && !exe)
	return NULL;

    for (i = 0; i < numsyms; i++) {
#if LOADERDEBUG
	LoaderDebugMsg(LOADER_DEBUG_LOWLEVEL,
		 "value=%lx\tsize=%lx\tBIND=%x\tTYPE=%x\tndx=%x\t%s\n",
		 (unsigned long)syms[i].st_value,
		 (unsigned long)syms[i].st_size,
		 ELF_ST_BIND(syms[i].st_info), ELF_ST_TYPE(syms[i].st_info),
		 syms[i].st_shndx, ElfGetString(elffile, syms[i].st_name));
#endif

	if (syms[i].st_shndx != sectnum && !exe)
	    continue;

	if (syms[i].st_name == 0)
	    continue;

	switch (ELF_ST_TYPE(syms[i].st_info)) {
	case STT_OBJECT:
	case STT_FUNC:
	case STT_SECTION:
	    saddr = syms[i].st_value + (unsigned long)elffile->saddr[sectnum];
	    diff = address - saddr;
	    if (diff >= 0) {
		if ((best && diff < bestDiff) || !best) {
		    best = ElfGetString(elffile, syms[i].st_name);
		    bestDiff = diff;
		    bestAddr = saddr;
		}
	    }
	    break;
	}
    }
    *filename = NULL;
    if (best && bestDiff < 0x10000) {
	*symaddr = bestAddr;
	return best;
    } else {
	return NULL;
    }
}


#if EXESYMDEBUG
static void
ShowExeSyms(ELFModulePtr elffile)
{
    Elf_Sym *syms;
    Elf_Shdr *sect;
    int i, numsyms;

    syms = elffile->symtab;
    sect = &(elffile->sections[elffile->symndx]);
    if (!sect->sh_entsize)
	return;
    numsyms = sect->sh_size / sect->sh_entsize;

    LoaderDebugMsg(LOADER_DEBUG_SHOW_EXE_SYMS,
		   "ShowExeSyms: numsyms: %d\n", numsyms);

    for (i = 0; i < numsyms; i++) {
	LoaderDebugMsg(LOADER_DEBUG_SHOW_EXE_SYMS,
		 "ShowExeSyms: "
		 "value=%lx\tsize=%lx\tBIND=%x\tTYPE=%x\tndx=%x\t%s\n",
		 (unsigned long)syms[i].st_value,
		 (unsigned long)syms[i].st_size,
		 ELF_ST_BIND(syms[i].st_info), ELF_ST_TYPE(syms[i].st_info),
		 syms[i].st_shndx, ElfGetString(elffile, syms[i].st_name));

	switch (ELF_ST_TYPE(syms[i].st_info)) {
	case STT_OBJECT:
	case STT_FUNC:
	case STT_SECTION:
	case STT_NOTYPE:
	    switch (syms[i].st_shndx) {
	    case SHN_ABS:
	    case SHN_COMMON:
	    case SHN_UNDEF:
	    default:
		break;
	    }
	    break;
	}
    }
}
#endif

/*
 * ELFReadExecutableSyms
 *
 * Open the main executable, and read in the symbol table and section
 * offsets.
 */
void *
ELFReadExecutableSyms(int elffd)
{
    ELFModulePtr elffile;
    Elf_Ehdr *header;
    int totalsize, maxalign;

    ldrCommons = 0;
    nCommons = 0;

    elffile = xf86loadercalloc(1, sizeof(*elffile));
    if (!elffile) {
	ErrorF("Cannot allocate space for the main executable symbol table\n");
	return NULL;
    }

    elffile->fd = elffd;

    /*
     *  Get the ELF header
     */
    elffile->header =
	    (Elf_Ehdr *) _LoaderFileToMem(elffd, 0, sizeof(Elf_Ehdr),
					  "header");
    header = elffile->header;

    /*
     * Get the section table
     */
    elffile->numsh = header->e_shnum;
    elffile->secsize = (header->e_shentsize * header->e_shnum);
    elffile->sections =
	    (Elf_Shdr *) _LoaderFileToMem(elffd, header->e_shoff,
					  elffile->secsize, "sections");
    elffile->saddr =
	    xf86loadercalloc(elffile->numsh, sizeof(unsigned char *));

    /*
     * Get the section header string table
     */
    elffile->shstrsize = SecSize(header->e_shstrndx);
    elffile->shstraddr =
	    _LoaderFileToMem(elffd, SecOffset(header->e_shstrndx),
			     SecSize(header->e_shstrndx), ".shstrtab");
    elffile->shstrndx = header->e_shstrndx;

    /*
     * Load some desired sections, compute size of the remaining ones
     */
    totalsize = 0;
    maxalign = 0;
    ELFCollectSections(elffile, 0, &totalsize, &maxalign);
    if (elffile->straddr == NULL || elffile->strsize == 0 ||
	elffile->sections[elffile->symndx].sh_entsize == 0) {
	ErrorF("No symbols found in the main executable.\n");
	_LoaderFreeFileMem(elffile->sections, elffile->secsize);
	_LoaderFreeFileMem(elffile->header, sizeof(Elf_Ehdr));
	_LoaderFreeFileMem(elffile->shstraddr, elffile->shstrsize);
	xf86loaderfree(elffile->saddr);
	xf86loaderfree(elffile);
	return NULL;
    }
#if EXESYMDEBUG
    ShowExeSyms(elffile);
#endif
    return elffile;
}

