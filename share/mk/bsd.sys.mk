#	$NetBSD: bsd.sys.mk,v 1.292.2.2 2024/02/29 13:00:14 martin Exp $
#
# Build definitions used for NetBSD source tree builds.

.if !defined(_BSD_SYS_MK_)
_BSD_SYS_MK_=1

.if !empty(.INCLUDEDFROMFILE:MMakefile*)
error1:
	@(echo "bsd.sys.mk should not be included from Makefiles" >& 2; exit 1)
.endif
.if !defined(_BSD_OWN_MK_)
error2:
	@(echo "bsd.own.mk must be included before bsd.sys.mk" >& 2; exit 1)
.endif

# NetBSD sources use C17 style, with some GCC extensions.
# Coverity does not like -std=gnu17
.if !defined(COVERITY_TOP_CONFIG) && empty(CFLAGS:M*-std=*)
CFLAGS+=	${${ACTIVE_CC} == "clang":? -std=gnu17 :}
CFLAGS+=	${${ACTIVE_CC} == "gcc":? -std=gnu17 :}
CFLAGS+=	${${ACTIVE_CC} == "pcc":? -std=gnu17 :}
.endif

.if defined(WARNS)
CFLAGS+=	${${ACTIVE_CC} == "clang":? -Wno-sign-compare -Wno-pointer-sign :}
.if ${WARNS} > 0
CFLAGS+=	-Wall -Wstrict-prototypes -Wmissing-prototypes -Wpointer-arith
#CFLAGS+=	-Wmissing-declarations -Wredundant-decls -Wnested-externs
# Add -Wno-sign-compare.  -Wsign-compare is included in -Wall as of GCC 3.3,
# but our sources aren't up for it yet. Also, add -Wno-traditional because
# gcc includes #elif in the warnings, which is 'this code will not compile
# in a traditional environment' warning, as opposed to 'this code behaves
# differently in traditional and ansi environments' which is the warning
# we wanted, and now we don't get anymore.
CFLAGS+=	-Wno-sign-compare
# Don't suppress warnings coming from constructs in system headers.
# Our system headers should be clean and we want to warn about things like:
# isdigit((char)1)
CFLAGS+=	${${ACTIVE_CC} == "gcc" :? -Wsystem-headers :}
CFLAGS+=	${${ACTIVE_CC} == "gcc" :? -Wno-traditional :}
.if !defined(NOGCCERROR)
# Set assembler warnings to be fatal
CFLAGS+=	${${ACTIVE_CC} == "gcc" :? -Wa,--fatal-warnings :}
.endif

# Set linker warnings to be fatal
# XXX no proper way to avoid "FOO is a patented algorithm" warnings
# XXX on linking static libs
.if (!defined(MKPIC) || ${MKPIC} != "no") && \
    (!defined(LDSTATIC) || ${LDSTATIC} != "-static")
# XXX there are some strange problems not yet resolved
. if !defined(HAVE_GCC) || defined(HAVE_LLVM)
LDFLAGS+=	-Wl,--fatal-warnings
. endif
.endif
.endif

LDFLAGS+=	-Wl,--warn-shared-textrel

.if ${WARNS} > 1
CFLAGS+=	-Wreturn-type -Wswitch -Wshadow
.endif
.if ${WARNS} > 2
CFLAGS+=	-Wcast-qual -Wwrite-strings
# Readd -Wno-sign-compare to override -Wextra with clang
CFLAGS+=	-Wno-sign-compare
.if "${ACTIVE_CC}" == "gcc" && ${HAVE_GCC} < 8
#  XXX: Won't warn about anything.  -Wabi warns about differences from
#  the most up-to-date ABI, which in g++ 8 is used by default.
CXXFLAGS+=	-Wabi
.endif
CXXFLAGS+=	-Wold-style-cast
CXXFLAGS+=	-Wctor-dtor-privacy -Wnon-virtual-dtor -Wreorder \
		    -Wno-deprecated -Woverloaded-virtual -Wsign-promo -Wsynth
CXXFLAGS+=	${${ACTIVE_CXX} == "gcc":? -Wno-non-template-friend -Wno-pmf-conversions :}
.endif
.if ${WARNS} > 3 && (defined(HAVE_GCC) || defined(HAVE_LLVM))
.if ${WARNS} > 4
CFLAGS+=	-Wold-style-definition
.endif
.if ${WARNS} > 5
CFLAGS+=	-Wconversion
.endif
CFLAGS+=	-Wsign-compare -Wformat=2
CFLAGS+=	${${ACTIVE_CC} == "gcc":? -Wno-format-zero-length :}
.endif
.if ${WARNS} > 3 && defined(HAVE_LLVM)
CFLAGS+=	${${ACTIVE_CC} == "clang":? -Wpointer-sign -Wmissing-noreturn :}
.endif
.if (defined(HAVE_GCC) \
     && (${MACHINE_ARCH} == "coldfire" || \
	 ${MACHINE_CPU} == "sh3" || \
	 ${MACHINE_CPU} == "m68k"))
# XXX GCC 4.5 for sh3 and m68k (which we compile with -Os) is extra noisy for
# cases it should be better with
CFLAGS+=	-Wno-uninitialized
CFLAGS+=	-Wno-maybe-uninitialized
.endif
.endif

.if ${MKRELRO:Uno} != "no"
LDFLAGS+=	-Wl,-z,relro
.endif
.if ${MKRELRO:Uno} == "full"
LDFLAGS+=	-Wl,-z,now
.endif

.if ${MKSANITIZER:Uno} == "yes"
SANITIZERFLAGS:=	-fsanitize=${USE_SANITIZER} ${SANITIZERFLAGS}
.else
SANITIZERFLAGS=		# empty
.endif

.if ${MKLIBCSANITIZER:Uno} == "yes"
LIBCSANITIZERFLAGS:=	-fsanitize=${USE_LIBCSANITIZER} ${LIBCSANITIZERFLAGS}
LIBCSANITIZERFLAGS+=	-fno-sanitize=vptr	# Unsupported in micro-UBSan
.else
LIBCSANITIZERFLAGS=	# empty
.endif

CWARNFLAGS+=	${CWARNFLAGS.${ACTIVE_CC}}

CPPFLAGS+=	${AUDIT:D-D__AUDIT__}
_NOWERROR=	${defined(NOGCCERROR) || (${ACTIVE_CC} == "clang" && defined(NOCLANGERROR)):?yes:no}
CFLAGS+=	${${_NOWERROR} == "no" :?-Werror:} ${CWARNFLAGS}
LINTFLAGS+=	${DESTDIR:D-d ${DESTDIR}/usr/include}


.if !defined(NOSSP) && (${USE_SSP:Uno} != "no") && (${BINDIR:Ux} != "/usr/mdec")
.   if !defined(KERNSRCDIR) && !defined(KERN) # not for kernels / kern modules
CPPFLAGS+=	-D_FORTIFY_SOURCE=2
.   endif
.   if !defined(COVERITY_TOP_CONFIG)
COPTS+=	-fstack-protector -Wstack-protector 

# GCC 4.8 on m68k erroneously does not protect functions with
# variables needing special alignement, see
#	http://gcc.gnu.org/bugzilla/show_bug.cgi?id=59674
# (the underlying issue for sh and vax may be different, needs more
# investigation, symptoms are similar but for different sources)
# also true for GCC 5, assume GCC 6 too.
.	if "${ACTIVE_CC}" == "gcc" && \
     ( ${HAVE_GCC} == "5" || \
       ${HAVE_GCC} == "6" ) && \
     ( ${MACHINE_CPU} == "sh3" || \
       ${MACHINE_ARCH} == "vax" || \
       ${MACHINE_CPU} == "m68k" || \
       ${MACHINE_CPU} == "or1k" )
COPTS+=	-Wno-error=stack-protector 
.	endif

COPTS+=	${${ACTIVE_CC} == "clang":? --param ssp-buffer-size=1 :}
COPTS+=	${${ACTIVE_CC} == "gcc":? --param ssp-buffer-size=1 :}
.   endif
.endif

CFLAGS+=	${CPUFLAGS}
AFLAGS+=	${CPUFLAGS}

.if !defined(NOPIE) && (!defined(LDSTATIC) || ${LDSTATIC} != "-static")
# Position Independent Executable flags
PIE_CFLAGS?=        -fPIE
PIE_LDFLAGS?=       -pie ${${ACTIVE_CC} == "gcc":? -shared-libgcc :}
PIE_AFLAGS?=	    -fPIE
.endif

ELF2AOUT?=	elf2aout
ELF2ECOFF?=	elf2ecoff
MKDEP?=		mkdep
MKDEPCXX?=	mkdep
OBJCOPY?=	objcopy
OBJDUMP?=	objdump
STRIP?=		strip

.SUFFIXES:	.m .o .ln .lo .c .cc .cpp .cxx .C ${YHEADER:D.h}

# C
.c.o:
	${_MKTARGET_COMPILE}
	${COMPILE.c} ${COPTS.${.IMPSRC:T}} ${CPUFLAGS.${.IMPSRC:T}} ${CPPFLAGS.${.IMPSRC:T}} ${.IMPSRC}
.if defined(CTFCONVERT)
	${CTFCONVERT} ${CTFFLAGS} ${.TARGET}
.endif

.c.ln:
	${_MKTARGET_COMPILE}
	${LINT} ${LINTFLAGS} ${LINTFLAGS.${.IMPSRC:T}} \
	    ${CPPFLAGS:C/-([IDU])[  ]*/-\1/Wg:M-[IDU]*} \
	    ${CPPFLAGS.${.IMPSRC:T}:C/-([IDU])[  ]*/-\1/Wg:M-[IDU]*} \
	    -i ${.IMPSRC}

# C++
.cc.o .cpp.o .cxx.o .C.o:
	${_MKTARGET_COMPILE}
	${COMPILE.cc} ${COPTS.${.IMPSRC:T}} ${CPUFLAGS.${.IMPSRC:T}} ${CPPFLAGS.${.IMPSRC:T}} ${.IMPSRC}

# Objective C
# (Defined here rather than in <sys.mk> because `.m' is not just
#  used for Objective C source)
.m.o:
	${_MKTARGET_COMPILE}
	${COMPILE.m} ${OBJCOPTS} ${OBJCOPTS.${.IMPSRC:T}} ${.IMPSRC}
.if defined(CTFCONVERT)
	${CTFCONVERT} ${CTFFLAGS} ${.TARGET}
.endif

# Host-compiled C objects
# The intermediate step is necessary for Sun CC, which objects to calling
# object files anything but *.o
.c.lo:
	${_MKTARGET_COMPILE}
	${HOST_COMPILE.c} -o ${.TARGET}.o ${COPTS.${.IMPSRC:T}} ${CPUFLAGS.${.IMPSRC:T}} ${CPPFLAGS.${.IMPSRC:T}} ${.IMPSRC}
	${MV} ${.TARGET}.o ${.TARGET}

# C++
.cc.lo .cpp.lo .cxx.lo .C.lo:
	${_MKTARGET_COMPILE}
	${HOST_COMPILE.cc} -o ${.TARGET}.o ${COPTS.${.IMPSRC:T}} ${CPUFLAGS.${.IMPSRC:T}} ${CPPFLAGS.${.IMPSRC:T}} ${.IMPSRC}
	${MV} ${.TARGET}.o ${.TARGET}

# Assembly
.s.o:
	${_MKTARGET_COMPILE}
	${COMPILE.s} ${COPTS.${.IMPSRC:T}} ${CPUFLAGS.${.IMPSRC:T}} ${CPPFLAGS.${.IMPSRC:T}} ${.IMPSRC}
.if defined(CTFCONVERT)
	${CTFCONVERT} ${CTFFLAGS} ${.TARGET}
.endif

.S.o:
	${_MKTARGET_COMPILE}
	${COMPILE.S} ${COPTS.${.IMPSRC:T}} ${CPUFLAGS.${.IMPSRC:T}} ${CPPFLAGS.${.IMPSRC:T}} ${.IMPSRC}
.if defined(CTFCONVERT)
	${CTFCONVERT} ${CTFFLAGS} ${.TARGET}
.endif

# Lex
LFLAGS+=	${LPREFIX.${.IMPSRC:T}:D-P${LPREFIX.${.IMPSRC:T}}}
LFLAGS+=	${LPREFIX:D-P${LPREFIX}}

.l.c:
	${_MKTARGET_LEX}
	${LEX.l} -o${.TARGET} ${.IMPSRC}

# Yacc
YFLAGS+=	${YPREFIX.${.IMPSRC:T}:D-p${YPREFIX.${.IMPSRC:T}}} ${YHEADER.${.IMPSRC:T}:D-d}
YFLAGS+=	${YPREFIX:D-p${YPREFIX}} ${YHEADER:D-d}

.y.c:
	${_MKTARGET_YACC}
	${YACC.y} -o ${.TARGET} ${.IMPSRC}

.ifdef YHEADER
.if empty(.MAKEFLAGS:M-n)
.y.h: ${.TARGET:.h=.c}
.endif
.endif

# Objcopy
.if ${MACHINE_ARCH} == aarch64eb
# AARCH64 big endian needs to preserve $x/$d symbols for the linker.
OBJCOPYLIBFLAGS_EXTRA=-w -K '[$$][dx]' -K '[$$][dx]\.*'
.elif ${MACHINE_CPU} == "arm"
# ARM big endian needs to preserve $a/$d/$t symbols for the linker.
OBJCOPYLIBFLAGS_EXTRA=-w -K '[$$][adt]' -K '[$$][adt]\.*'
.endif

.if ${MKSTRIPSYM:Uyes} == "yes"
OBJCOPYLIBFLAGS?=${"${.TARGET:M*.po}" != "":?-X:-x} ${OBJCOPYLIBFLAGS_EXTRA}
.else
OBJCOPYLIBFLAGS?=-X ${OBJCOPYLIBFLAGS_EXTRA}
.endif

.endif	# !defined(_BSD_SYS_MK_)
