# $NetBSD: Makefile,v 1.8 2002/08/19 14:55:15 lukem Exp $

USE_FORT= yes
USE_SHLIBDIR=	yes

.include <bsd.own.mk>

LIB=	ipsec
CPPFLAGS+=-I${.CURDIR} -I. -DIPSEC_DEBUG -DYY_NO_UNPUT

.if defined(IPSEC)
CPPFLAGS+=-DIPSEC
.endif

.if defined(FAST_IPSEC)
CPPFLAGS+=-DFAST_IPSEC
.endif

.if (${USE_INET6} != "no")
CPPFLAGS+=-DINET6
.endif

SRCS+=	ipsec_strerror.c policy_parse.y policy_token.l
SRCS+=	ipsec_get_policylen.c ipsec_dump_policy.c
LPREFIX+=__libyy
YPREFIX+=__libyy
YHEADER=1

#.PATH:	${NETBSDSRCDIR}/sys/netkey
#SRCS+=	pfkey.c pfkey_dump.c
#SRCS+=	key_debug.c

#LFLAGS+=	-olex.yy.c

MAN=	ipsec_set_policy.3 ipsec_strerror.3
MLINKS+=ipsec_set_policy.3 ipsec_get_policylen.3 \
	    ipsec_set_policy.3 ipsec_dump_policy.3

CLEANFILES+=	y.tab.h

.include <bsd.lib.mk>
