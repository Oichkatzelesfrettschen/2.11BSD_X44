# $NetBSD: Makefile,v 1.9 2020/01/22 15:10:32 mgorny Exp $

.include <bsd.own.mk>

LIBEXECINFO_MACHINE_ARCH?= ${MACHINE_ARCH}
USE_UNWIND=yes
WARNS?=4
INCSDIR=/usr/include
INCS=execinfo.h

#CPPFLAGS+='-D__RCSID(a)=' -D_GNU_SOURCE '-D__printflike(a,b)='
#CPPFLAGS+=-I/usr/include/libelf
#COPTS+=-std=gnu99

LIBDPLIBS+= elf	${NETBSDSRCDIR}/contrib/elftoolchain/lib/libelf

LIB=execinfo
SRCS=symtab.c backtrace.c
MAN= backtrace.3

.if ${USE_UNWIND} == "yes"
SRCS+=unwind.c
.if !empty(LIBEXECINFO_MACHINE_ARCH:Mearm*)
SRCS+=unwind_arm_ehabi_stub.c
.endif
.else
SRCS+=builtin.c
.endif

.if exists(${.CURDIR}/symbol_${LIBEXECINFO_MACHINE_ARCH}.c)
SRCS+=symbol_${LIBEXECINFO_MACHINE_ARCH}.c
.endif

MLINKS+= backtrace.3 backtrace_symbols.3
MLINKS+= backtrace.3 backtrace_symbols_fmt.3
MLINKS+= backtrace.3 backtrace_symbols_fd.3
MLINKS+= backtrace.3 backtrace_symbols_fd_fmt.3

.include <bsd.lib.mk>
