#	$NetBSD: Makefile.inc,v 1.33 2021/04/12 00:05:56 mrg Exp $

.ifndef _EXTERNAL_GPL3_GCC_USR_BIN_MAKEFILE_INC_
_EXTERNAL_GPL3_GCC_USR_BIN_MAKEFILE_INC_=1

NOLINT=yes
NOCTF=	# defined
.include <bsd.init.mk>

.sinclude "../../Makefile.gcc_path"

GCC_MACHINE_ARCH=${MACHINE_ARCH:S/earmv5/earm/}
.include "../../../gnu/gmp/Makefile.arch"

TOP=		${NETBSDSRCDIR}
DIST=		${GCCDIST}

GCCARCH=	${.CURDIR}/../gcc/arch/${GCC_MACHINE_ARCH}

CPPFLAGS+=	-DLOCALEDIR=\"/usr/share/locale\" -DNETBSD_NATIVE
HOST_CPPFLAGS+=	-I${.CURDIR}/..

# Link in the GMP/MPFR/MPC headers since we don't install them
GMPDISTDIR=	${NETBSDSRCDIR}/contrib/gnu/gmp
MPFRDISTDIR=	${NETBSDSRCDIR}/contrib/gnu/mpfr
MPCDISTDIR=	${NETBSDSRCDIR}/contrib/gnu/mpc

BUILDSYMLINKS+=	${GMPDISTDIR}/lib/libgmp/arch/${GMP_MACHINE_ARCH}/gmp.h gmp.h
BUILDSYMLINKS+=	${MPFRDISTDIR}/dist/src/mpfr.h mpfr.h
BUILDSYMLINKS+=	${MPFRDISTDIR}/dist/src/mpf2mpfr.h mpf2mpfr.h
BUILDSYMLINKS+=	${MPCDISTDIR}/dist/src/mpc.h mpc.h
BUILDSYMLINKS+=	${MPCDISTDIR}/dist/src/mpc-log.h mpc-log.h
CPPFLAGS+=	-I.
# XXX
DPSRCS+= gmp.h mpfr.h mpf2mpfr.h mpc.h mpc-log.h

BACKENDOBJ!=		cd ${.CURDIR}/../backend && ${PRINTOBJDIR}
FRONTENDOBJ!=		cd ${.CURDIR}/../frontend && ${PRINTOBJDIR}

.if ${USETOOLS} == "yes"
NBCOMPATLIB=   -L${TOOLDIR}/lib -lnbcompat
.endif

#CXXFLAGS+=	-std=gnu++98

GNUHOSTDIST=	${DIST}
BUILD_PREFIX=	${C_BUILD_PREFIX}

HOSTLIBIBERTYOBJ!=	cd ${.CURDIR}/../host-libiberty && ${PRINTOBJDIR}
HOSTLIBIBERTY=		${HOSTLIBIBERTYOBJ}/libiberty/libiberty.a

HOSTLIBCPPOBJ!=		cd ${.CURDIR}/../host-libcpp && ${PRINTOBJDIR}
HOSTLIBCPP=		${HOSTLIBCPPOBJ}/libcpp/libcpp.a

BASEVER!=		cat ${GNUHOSTDIST}/gcc/BASE-VER
.if exists(${GNUHOSTDIST}/gcc/DEV-PHASE)
DEVPHASE!=		cat ${GNUHOSTDIST}/gcc/DEV-PHASE
.else
DEVPHASE=
.endif
.if ${DEVPHASE} == "release" || ${DEVPHASE} == ""
DATESTAMP=
.else
DATESTAMP!=		cat ${GNUHOSTDIST}/gcc/DATESTAMP
.endif

BASEVERSTR=		"\"$(BASEVER)\""
.if !empty(DEVPHASE)
DEVPHASESTR=		"\" $(DEVPHASE)\""
.else
DEVPHASESTR=		"\"\""
.endif
.if !empty(DATESTAMP)
DATESTAMPSTR=		"\" $(DATESTAMP)\""
.else
DATESTAMPSTR=		"\"\""
.endif

.include "${TOP}/tools/gcc/gcc-version.mk"

# XXX pull this out of our configs
G_BUGURL=<http://www.NetBSD.org/support/send-pr.html>
G_BUGURL_s="\"${G_BUGURL}\""
G_PKGVERSION=(${NETBSD_GCC_VERSION}) 
G_PKGVERSION_s="\"${G_PKGVERSION} \""

VER_CPPFLAGS=	-DBASEVER="\"${BASEVER}"\" \
		-DDATESTAMP=$(DATESTAMP_s) \
		-DREVISION=$(REVISION_s) \
		-DDEVPHASE=$(DEVPHASE_s) \
		-DPKGVERSION=$(G_PKGVERSION_s) \
		-DBUGURL=$(G_BUGURL_s)

VERFILES=	cppbuiltin.c c-cppbuiltin.c gcc.c version.c

.for _f in ${VERFILES}
CPPFLAGS.${_f} +=	${VER_CPPFLAGS}
${_f}: ${GCCARCH}/defs.mk ${TOP}/tools/gcc/gcc-version.mk
.endfor

.include "${GCCARCH}/defs.mk"

.if ${MKPIC} != "no"
.if ${G_ENABLE_SHARED} == "yes" && ${G_SHLIB_LINK} != ""
CPPFLAGS+=	-DENABLE_SHARED_LIBGCC
.endif

.if empty(G_SHLIB_MULTILIB)
CPPFLAGS+=	-DNO_SHARED_LIBGCC_MULTILIB
.endif
.endif

NOCLANGERROR=	# defined

WARNS=1
COPTS+=-Wno-narrowing -Wno-unused

# Lots trigger this.
CFLAGS+=	-Wno-format-diag

.endif
