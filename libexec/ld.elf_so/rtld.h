/*	$NetBSD: rtld.h,v 1.79.4.4 2012/03/17 18:28:31 bouyer Exp $	 */

/*
 * Copyright 1996 John D. Polstra.
 * Copyright 1996 Matt Thomas <matt@3am-software.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by John Polstra.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef RTLD_H
#define RTLD_H

#include <dlfcn.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/exec_elf.h>
#include "rtldenv.h"
#include "link.h"

#if defined(_RTLD_SOURCE)

#ifndef	RTLD_DEFAULT_LIBRARY_PATH
#define	RTLD_DEFAULT_LIBRARY_PATH	"/usr/lib"
#endif
#define _PATH_LD_HINTS			"/etc/ld.so.conf"

extern int _rtld_pagesz;

#define round_down(x)	((x) & ~(_rtld_pagesz - 1))
#define round_up(x)	round_down((x) + _rtld_pagesz - 1)

#define NEW(type)	((type *) xmalloc(sizeof(type)))
#define CNEW(type)	((type *) xcalloc(sizeof(type)))

/*
 * Fill in a DoneList with an allocation large enough to hold all of
 * the currently-loaded objects. Keep this in a macro since it calls
 * alloca and we want that to occur within the scope of the caller.
 */
#define _rtld_donelist_init(dlp)					\
    ((dlp)->num_alloc = _rtld_objcount,					\
    (dlp)->objs = alloca((dlp)->num_alloc * sizeof((dlp)->objs[0])),	\
    assert((dlp)->objs != NULL),					\
    (dlp)->num_used = 0)

#endif /* _RTLD_SOURCE */

/*
 * C++ has mandated the use of the following keywords for its new boolean
 * type.  We might as well follow their lead.
 */
struct Struct_Obj_Entry;

typedef struct Struct_Objlist_Entry {
	SIMPLEQ_ENTRY(Struct_Objlist_Entry) link;
	struct Struct_Obj_Entry *obj;
} Objlist_Entry;

typedef SIMPLEQ_HEAD(Struct_Objlist, Struct_Objlist_Entry) Objlist;

typedef struct Struct_Name_Entry {
	STAILQ_ENTRY(Struct_Name_Entry)	link;
	char	name[1];
} Name_Entry;

typedef struct Struct_Needed_Entry {
	struct Struct_Needed_Entry *next;
	struct Struct_Obj_Entry *obj;
	unsigned long   name;	/* Offset of name in string table */
} Needed_Entry;

typedef struct _rtld_search_path_t {
	struct _rtld_search_path_t *sp_next;
	const char     *sp_path;
	size_t          sp_pathlen;
} Search_Path;


#define RTLD_MAX_ENTRY 10
#define RTLD_MAX_LIBRARY 4
#define RTLD_MAX_CTL 2
typedef struct _rtld_library_xform_t {
	struct _rtld_library_xform_t *next;
	char *name;
	const char *ctlname;
	struct {
		char *value;
		char *library[RTLD_MAX_LIBRARY];
	} entry[RTLD_MAX_ENTRY];
} Library_Xform;

/*
 * Shared object descriptor.
 *
 * Items marked with "(%)" are dynamically allocated, and must be freed
 * when the structure is destroyed.
 */

#define RTLD_MAGIC	0xd550b87a
#define RTLD_VERSION	1
#define	RTLD_MAIN	0x800

typedef struct Struct_Obj_Entry {
	Elf32_Word      magic;		/* Magic number (sanity check) */
	Elf32_Word      version;	/* Version number of struct format */

	struct Struct_Obj_Entry *next;
	char           *path;		/* Pathname of underlying file (%) */
	int             refcount;
	int             dl_refcount;	/* Number of times loaded by dlopen */

	/* These items are computed by map_object() or by digest_phdr(). */
	caddr_t         mapbase;	/* Base address of mapped region */
	size_t          mapsize;	/* Size of mapped region in bytes */
	size_t          textsize;	/* Size of text segment in bytes */
	Elf_Addr        vaddrbase;	/* Base address in shared object file */
	caddr_t         relocbase;	/* Reloc const = mapbase - *vaddrbase */
	Elf_Dyn        *dynamic;	/* Dynamic section */
	caddr_t         entry;		/* Entry point */
	const Elf_Phdr *phdr;		/* Program header (may be xmalloc'ed) */
	size_t		phsize;		/* Size of program header in bytes */

	/* Items from the dynamic section. */
	Elf_Addr       *pltgot;		/* PLTGOT table */
	const Elf_Rel  *rel;		/* Relocation entries */
	const Elf_Rel  *rellim;		/* Limit of Relocation entries */
	const Elf_Rela *rela;		/* Relocation entries */
	const Elf_Rela *relalim;	/* Limit of Relocation entries */
	const Elf_Rel  *pltrel;		/* PLT relocation entries */
	const Elf_Rel  *pltrellim;	/* Limit of PLT relocation entries */
	const Elf_Rela *pltrela;	/* PLT relocation entries */
	const Elf_Rela *pltrelalim;	/* Limit of PLT relocation entries */
	const Elf_Sym  *symtab;		/* Symbol table */
	const char     *strtab;		/* String table */
	unsigned long   strsize;	/* Size in bytes of string table */
#ifdef __mips__
	Elf_Word        local_gotno;	/* Number of local GOT entries */
	Elf_Word        symtabno;	/* Number of dynamic symbols */
	Elf_Word        gotsym;		/* First dynamic symbol in GOT */
#endif

	const Elf_Word *buckets;	/* Hash table buckets array */
	unsigned long   nbuckets;	/* Number of buckets */
	const Elf_Word *chains;		/* Hash table chain array */
	unsigned long   nchains;	/* Number of chains */

	Search_Path    *rpaths;		/* Search path specified in object */
	Needed_Entry   *needed;		/* Shared objects needed by this (%) */

	void            (*init)(void); 	/* Initialization function to call */
	void            (*fini)(void);	/* Termination function to call */

	/* Entry points for dlopen() and friends. */
	void           *(*dlopen)(const char *, int);
	void           *(*dlsym)(void *, const char *);
	char           *(*dlerror)(void);
	int             (*dlclose)(void *);
	int             (*dladdr)(const void *, Dl_info *);

	u_int32_t	mainprog:1,	/* True if this is the main program */
	        	rtld:1,		/* True if this is the dynamic linker */
			textrel:1,	/* True if there are relocations to
					 * text seg */
			symbolic:1,	/* True if generated with
					 * "-Bsymbolic" */
			printed:1,	/* True if ldd has printed it */
			isdynamic:1,	/* True if this is a pure PIC object */
			mainref:1,	/* True if on _rtld_list_main */
			globalref:1,	/* True if on _rtld_list_global */
			init_done:1,	/* True if .init has been added */
			init_called:1,	/* True if .init function has been 
					 * called */
			fini_called:1,	/* True if .fini function has been 
					 * called */
			initfirst:1,	/* True if object's .init/.fini take
					 * priority over others */
			phdr_loaded:1;	/* Phdr is loaded and doesn't need to
					 * be freed. */

	struct link_map linkmap;	/* for GDB */

	/* These items are computed by map_object() or by digest_phdr(). */
	const char     *interp;	/* Pathname of the interpreter, if any */
	Objlist         dldags;	/* Object belongs to these dlopened DAGs (%) */
	Objlist         dagmembers;	/* DAG has these members (%) */
	dev_t           dev;		/* Object's filesystem's device */
	ino_t           ino;		/* Object's inode number */

	void		*ehdr;
	size_t		pathlen;	/* Pathname length */
	STAILQ_HEAD(, Struct_Name_Entry) names;	/* List of names for this object we
						   know about. */
} Obj_Entry;

typedef struct Struct_DoneList {
	const Obj_Entry **objs;		/* Array of object pointers */
	unsigned int num_alloc;		/* Allocated size of the array */
	unsigned int num_used;		/* Number of array slots used */
} DoneList;


#if defined(_RTLD_SOURCE)

extern struct r_debug _rtld_debug;
extern Search_Path *_rtld_default_paths;
extern Obj_Entry *_rtld_objlist;
extern Obj_Entry **_rtld_objtail;
extern u_int _rtld_objcount;
extern u_int _rtld_objloads;
extern Obj_Entry *_rtld_objmain;
extern Obj_Entry _rtld_objself;
extern Search_Path *_rtld_paths;
extern Library_Xform *_rtld_xforms;
extern bool _rtld_trust;
extern Objlist _rtld_list_global;
extern Objlist _rtld_list_main;
extern Elf_Sym _rtld_sym_zero;

/* rtld.c */

/* We export these symbols using _rtld_symbol_lookup and is_exported. */
char *dlerror(void);
void *dlopen(const char *, int);
void *dlsym(void *, const char *);
int dlclose(void *);
int dladdr(const void *, Dl_info *);
int dlinfo(void *, int, void *);
int dl_iterate_phdr(int (*)(struct dl_phdr_info *, size_t, void *),
    void *);
void *_dlauxinfo(void) __pure;

/* These aren't exported */
void _rtld_error(const char *, ...)
     __attribute__((__format__(__printf__,1,2)));
void _rtld_die(void) __attribute__((__noreturn__));
void *_rtld_objmain_sym(const char *);
void _rtld_debug_state(void);
void _rtld_linkmap_add(Obj_Entry *);
void _rtld_linkmap_delete(Obj_Entry *);
void _rtld_objlist_push_head(Objlist *, Obj_Entry *);
void _rtld_objlist_push_tail(Objlist *, Obj_Entry *);
Objlist_Entry *_rtld_objlist_find(Objlist *, const Obj_Entry *);

/* expand.c */
size_t _rtld_expand_path(char *, size_t, const char *, const char *,\
    const char *);

/* headers.c */
void _rtld_digest_dynamic(const char *, Obj_Entry *);
Obj_Entry *_rtld_digest_phdr(const Elf_Phdr *, int, caddr_t);

/* load.c */
Obj_Entry *_rtld_load_object(const char *, int);
int _rtld_load_needed_objects(Obj_Entry *, int);
int _rtld_preload(const char *);

/* path.c */
void _rtld_add_paths(const char *, Search_Path **, const char *);
void _rtld_process_hints(const char *, Search_Path **, Library_Xform **,
    const char *);
int _rtld_sysctl(const char *, void *, size_t *);

/* reloc.c */
int _rtld_do_copy_relocations(const Obj_Entry *);
int _rtld_relocate_objects(Obj_Entry *, bool);
int _rtld_relocate_nonplt_objects(const Obj_Entry *);
int _rtld_relocate_plt_lazy(const Obj_Entry *);
int _rtld_relocate_plt_objects(const Obj_Entry *);
void _rtld_setup_pltgot(const Obj_Entry *);

/* search.c */
Obj_Entry *_rtld_load_library(const char *, const Obj_Entry *, int);

/* symbol.c */
unsigned long _rtld_elf_hash(const char *);
const Elf_Sym *_rtld_symlook_obj(const char *, unsigned long,
    const Obj_Entry *, bool);
const Elf_Sym *_rtld_find_symdef(unsigned long, const Obj_Entry *,
    const Obj_Entry **, bool);
const Elf_Sym *_rtld_find_plt_symdef(unsigned long, const Obj_Entry *, 
    const Obj_Entry **, bool);

const Elf_Sym *_rtld_symlook_list(const char *, unsigned long,
    const Objlist *, const Obj_Entry **, bool, DoneList *);
const Elf_Sym *_rtld_symlook_default(const char *, unsigned long,
    const Obj_Entry *, const Obj_Entry **, bool);
const Elf_Sym *_rtld_symlook_needed(const char *, unsigned long,
    const Needed_Entry *, const Obj_Entry **, bool,
    DoneList *, DoneList *);
#ifdef COMBRELOC
void _rtld_combreloc_reset(const Obj_Entry *);
#endif

/* map_object.c */
Obj_Entry *_rtld_map_object(const char *, int, const struct stat *);
void _rtld_obj_free(Obj_Entry *);
Obj_Entry *_rtld_obj_new(void);

/* function descriptors */
#ifdef __HAVE_FUNCTION_DESCRIPTORS
Elf_Addr _rtld_function_descriptor_alloc(const Obj_Entry *, 
    const Elf_Sym *, Elf_Addr);
const void *_rtld_function_descriptor_function(const void *);
#endif /* __HAVE_FUNCTION_DESCRIPTORS */

#endif /* _RTLD_SOURCE */

#endif /* RTLD_H */
