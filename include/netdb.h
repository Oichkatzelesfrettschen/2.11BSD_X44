/*
 * Copyright (c) 1980,1983,1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 *
 *	@(#)netdb.h	5.9.1 (2.11BSD GTE) 12/31/93
 */

#ifndef _NETDB_H_
#define	_NETDB_H_

#include <sys/cdefs.h>
#include <sys/ansi.h>
#include <machine/endian.h>
#include <inttypes.h>

/*
 * Data types
 */
#ifndef socklen_t
typedef __socklen_t	socklen_t;
#define	socklen_t	__socklen_t
#endif

#ifdef  _BSD_SIZE_T_
typedef _BSD_SIZE_T_	size_t;
#undef  _BSD_SIZE_T_
#endif

#define	_PATH_HEQUIV		"/etc/hosts.equiv"
#define	_PATH_HOSTS			"/etc/hosts"
#define	_PATH_NETWORKS		"/etc/networks"
#define	_PATH_PROTOCOLS		"/etc/protocols"
#define	_PATH_SERVICES		"/etc/services"
#define	_PATH_SERVICES_CDB 	"/var/db/services.cdb"
#define	_PATH_SERVICES_DB 	"/var/db/services.db"

__BEGIN_DECLS
extern int h_errno;
extern int * __h_errno(void);
__END_DECLS

/*
 * Structures returned by network
 * data base library.  All addresses
 * are supplied in host order, and
 * returned in network order (suitable
 * for use in system calls).
 */
struct hostent {
	char		*h_name;			/* official name of host */
	char		**h_aliases;		/* alias list */
	int			h_addrtype;			/* host address type */
	int			h_length;			/* length of address */
	char		**h_addr_list;		/* list of addresses from name server */
#define	h_addr	h_addr_list[0]		/* address, for backward compatiblity */
};

/*
 * Assumption here is that a network number
 * fits in 32 bits -- probably a poor one.
 */
struct netent {
	char			*n_name;		/* official name of net */
	char			**n_aliases;	/* alias list */
	int				n_addrtype;		/* net address type */
	unsigned long	n_net;			/* network # */
};

struct servent {
	char		*s_name;		/* official service name */
	char		**s_aliases;	/* alias list */
	int			s_port;			/* port # */
	char		*s_proto;		/* protocol to use */
};

struct protoent {
	char		*p_name;		/* official protocol name */
	char		**p_aliases;	/* alias list */
	int			p_proto;		/* protocol # */
};

/*
 * Note: ai_addrlen used to be a size_t, per RFC 2553.
 * In XNS5.2, and subsequently in POSIX-2001 and
 * draft-ietf-ipngwg-rfc2553bis-02.txt it was changed to a socklen_t.
 * To accommodate for this while preserving binary compatibility with the
 * old interface, we prepend or append 32 bits of padding, depending on
 * the (LP64) architecture's endianness.
 *
 * This should be deleted the next time the libc major number is
 * incremented.
 */
#if (_POSIX_C_SOURCE - 0) >= 200112L || (_XOPEN_SOURCE - 0) >= 520 || __BSD_VISIBLE
struct addrinfo {
	int				ai_flags;		/*%< AI_PASSIVE, AI_CANONNAME */
	int				ai_family;		/*%< PF_xxx */
	int				ai_socktype;	/*%< SOCK_xxx */
	int				ai_protocol;	/*%< 0 or IPPROTO_xxx for IPv4 and IPv6 */
	socklen_t	 	ai_addrlen;		/*%< length of ai_addr */
	char			*ai_canonname;	/*%< canonical name for hostname */
	struct sockaddr	*ai_addr; 		/*%< binary address */
	struct addrinfo	*ai_next; 		/*%< next structure in linked list */
};
#endif

/*
 * Error return codes from gethostbyname() and gethostbyaddr()
 * (left in extern int h_errno).
 */

#define	NETDB_INTERNAL	-1	/* see errno */
#define	NETDB_SUCCESS	0	/* no problem */
#define	HOST_NOT_FOUND	1 /* Authoritative Answer Host not found */
#define	TRY_AGAIN		2 /* Non-Authoritive Host not found, or SERVERFAIL */
#define	NO_RECOVERY		3 /* Non recoverable errors, FORMERR, REFUSED, NOTIMP */
#define	NO_DATA			4 /* Valid name, no data record of requested type */
#define	NO_ADDRESS		NO_DATA		/* no address, look for MX record */

/*
 * Error return codes from getaddrinfo()
 */
#if (_POSIX_C_SOURCE - 0) >= 200112L || (_XOPEN_SOURCE - 0) >= 520 || __BSD_VISIBLE
#define	EAI_ADDRFAMILY	1	/*%< address family for hostname not supported */
#define	EAI_AGAIN	 	2	/*%< temporary failure in name resolution */
#define	EAI_BADFLAGS	3	/*%< invalid value for ai_flags */
#define	EAI_FAIL	 	4	/*%< non-recoverable failure in name resolution */
#define	EAI_FAMILY	 	5	/*%< ai_family not supported */
#define	EAI_MEMORY	 	6	/*%< memory allocation failure */
#define	EAI_NODATA	 	7	/*%< no address associated with hostname */
#define	EAI_NONAME	 	8	/*%< hostname nor servname provided, or not known */
#define	EAI_SERVICE	 	9	/*%< servname not supported for ai_socktype */
#define	EAI_SOCKTYPE	10	/*%< ai_socktype not supported */
#define	EAI_SYSTEM		11	/*%< system error returned in errno */
#define	EAI_BADHINTS	12	/* invalid value for hints */
#define	EAI_PROTOCOL	13	/* resolved protocol is unknown */
#define	EAI_OVERFLOW	14	/* argument buffer overflow */
#define	EAI_MAX			15
#endif /* _POSIX_C_SOURCE >= 200112 || _XOPEN_SOURCE >= 520 || __BSD_VISIBLE */

/*%
 * Flag values for getaddrinfo()
 */
#if (_POSIX_C_SOURCE - 0) >= 200112L || (_XOPEN_SOURCE - 0) >= 520 || __BSD_VISIBLE
#define	AI_PASSIVE		0x00000001 /* get address to use bind() */
#define	AI_CANONNAME	0x00000002 /* fill ai_canonname */
#define	AI_NUMERICHOST	0x00000004 /* prevent host name resolution */
#define	AI_NUMERICSERV	0x00000008 /* prevent service name resolution */
#define	AI_ADDRCONFIG	0x00000400 /* only if any address is assigned */
/* valid flags for addrinfo (not a standard def, apps should not use it) */
#ifdef __BSD_VISIBLE
#define	AI_SRV			0x00000800 /* do _srv lookups */
#define	AI_MASK	    	(AI_PASSIVE | AI_CANONNAME | AI_NUMERICHOST | AI_NUMERICSERV | AI_ADDRCONFIG | AI_SRV)
#else
#define	AI_MASK	   		(AI_PASSIVE | AI_CANONNAME | AI_NUMERICHOST | AI_NUMERICSERV | AI_ADDRCONFIG)
#endif
#endif

#if (_POSIX_C_SOURCE - 0) >= 200112L || (_XOPEN_SOURCE - 0) >= 520 || __BSD_VISIBLE
/*%
 * Constants for getnameinfo()
 */
#if __BSD_VISIBLE
#define	NI_MAXHOST		1025
#define	NI_MAXSERV		32
#endif

/*%
 * Flag values for getnameinfo()
 */
#define	NI_NOFQDN		0x00000001
#define	NI_NUMERICHOST	0x00000002
#define	NI_NAMEREQD		0x00000004
#define	NI_NUMERICSERV	0x00000008
#define	NI_DGRAM		0x00000010
#define	NI_WITHSCOPEID	0x00000020
#define	NI_NUMERICSCOPE	0x00000040

/*%
 * Scope delimit character
 */
#if __BSD_VISIBLE
#define	SCOPE_DELIMITER	'%'
#endif
#endif /* (_POSIX_C_SOURCE - 0) >= 200112L || ... */

//unsigned long	gethostid(void);

__BEGIN_DECLS
void			endhostent(void);
void			endnetent(void);
void			endprotoent(void);
void			endservent(void);
struct hostent	*gethostbyaddr(const char *, int, int);
struct hostent	*gethostbyname(const char *);
struct hostent	*gethostent(void);
struct netent	*getnetbyaddr(long, int); /* u_long? */
struct netent	*getnetbyname(const char *);
struct netent	*getnetent(void);
struct servent	*getservbyname(const char *, const char *);
struct servent	*getservbyport(int, const char *);
struct servent	*getservent(void);
struct protoent	*getprotobyname(const char *);
struct protoent	*getprotobynumber(int);
struct protoent	*getprotoent(void);
void			herror(const char *);
const char		*hstrerror(int);
void			sethostent(int);
void			setnetent(int);
void			setprotoent(int);
void			setservent(int);
#if (_POSIX_C_SOURCE - 0) >= 200112L || (_XOPEN_SOURCE - 0) >= 520 || defined(__BSD_VISIBLE)
int				getaddrinfo(const char * __restrict, const char * __restrict, const struct addrinfo * __restrict, struct addrinfo ** __restrict);
int				getnameinfo(const struct sockaddr * __restrict, socklen_t, char * __restrict, socklen_t, char * __restrict, socklen_t, int);
void			freeaddrinfo(struct addrinfo *);
const char		*gai_strerror(int);
#endif
__END_DECLS

#endif /* !_NETDB_H_ */
