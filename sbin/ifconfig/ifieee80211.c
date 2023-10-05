/*	$NetBSD: ifconfig.c,v 1.141.4.2 2005/07/24 01:58:38 snj Exp $	*/

/*-
 * Copyright (c) 1997, 1998, 2000 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe of the Numerical Aerospace Simulation Facility,
 * NASA Ames Research Center.
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
 *	This product includes software developed by the NetBSD
 *	Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_media.h>
#include <net/if_ether.h>
#include <net80211/ieee80211.h>
#include <net80211/ieee80211_ioctl.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <util.h>

#include "ifconfig.h"

void	setifnwid(const char *, int);
void	setifnwkey(const char *, int);
void	setifbssid(const char *, int);
void	setifchan(const char *, int);
void	setifpowersave(const char *, int);
void	setifpowersavesleep(const char *, int);

const struct cmd ieee80211_cmds[] = {
		{ "bssid",	NEXTARG,	0,		setifbssid },
		{ "-bssid",	-1,		0,		setifbssid },
		{ "chan",	NEXTARG,	0,		setifchan },
		{ "-chan",	-1,		0,		setifchan },
		{ "ssid",	NEXTARG,	0,		setifnwid },
		{ "nwid",	NEXTARG,	0,		setifnwid },
		{ "nwkey",	NEXTARG,	0,		setifnwkey },
		{ "-nwkey",	-1,		0,		setifnwkey },
		{ "powersave",	1,		0,		setifpowersave },
		{ "-powersave",	0,		0,		setifpowersave },
		{ "powersavesleep", NEXTARG,	0,		setifpowersavesleep },
};

void	ieee80211_status(void);

struct afswtch af_ieee80211 = {
		.af_name	= "af_ieee80211",
		.af_af		= AF_UNSPEC,
		.af_status = ieee80211_status,
};

void
ieee80211_init(void)
{
	size_t i;

	for (i = 0; i < nitems(ieee80211_cmds);  i++) {
		cmd_register(&ieee80211_cmds[i]);
	}
	af_register(&af_ieee80211);
}

void
setifnwid(const char *val, int d)
{
	struct ieee80211_nwid nwid;
	int len;

	len = sizeof(nwid.i_nwid);
	if (get_string(val, NULL, nwid.i_nwid, &len) == NULL)
		return;
	nwid.i_len = len;
	(void)strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
	ifr.ifr_data = (void *)&nwid;
	if (ioctl(s, SIOCS80211NWID, &ifr) == -1)
		warn("SIOCS80211NWID");
}

void
setifbssid(const char *val, int d)
{
	struct ieee80211_bssid bssid;
	struct ether_addr *ea;

	if (d != 0) {
		/* no BSSID is especially desired */
		memset(&bssid.i_bssid, 0, sizeof(bssid.i_bssid));
	} else {
		ea = ether_aton(val);
		if (ea == NULL) {
			warnx("malformed BSSID: %s", val);
			return;
		}
		memcpy(&bssid.i_bssid, ea->ether_addr_octet,
		    sizeof(bssid.i_bssid));
	}
	(void)strncpy(bssid.i_name, name, sizeof(bssid.i_name));
	if (ioctl(s, SIOCS80211BSSID, &bssid) == -1)
		warn("SIOCS80211BSSID");
}

void
setifchan(const char *val, int d)
{
	struct ieee80211chanreq channel;
	int chan;

	if (d != 0)
		chan = IEEE80211_CHAN_ANY;
	else {
		chan = atoi(val);
		if (chan < 0 || chan > 0xffff) {
			warnx("invalid channel: %s", val);
			return;
		}
	}

	(void)strncpy(channel.i_name, name, sizeof(channel.i_name));
	channel.i_channel = (u_int16_t) chan;
	if (ioctl(s, SIOCS80211CHANNEL, &channel) == -1)
		warn("SIOCS80211CHANNEL");
}

void
setifnwkey(const char *val, int d)
{
	struct ieee80211_nwkey nwkey;
	int i;
	u_int8_t keybuf[IEEE80211_WEP_NKID][16];

	nwkey.i_wepon = IEEE80211_NWKEY_WEP;
	nwkey.i_defkid = 1;
	for (i = 0; i < IEEE80211_WEP_NKID; i++) {
		nwkey.i_key[i].i_keylen = sizeof(keybuf[i]);
		nwkey.i_key[i].i_keydat = keybuf[i];
	}
	if (d != 0) {
		/* disable WEP encryption */
		nwkey.i_wepon = 0;
		i = 0;
	} else if (strcasecmp("persist", val) == 0) {
		/* use all values from persistent memory */
		nwkey.i_wepon |= IEEE80211_NWKEY_PERSIST;
		nwkey.i_defkid = 0;
		for (i = 0; i < IEEE80211_WEP_NKID; i++)
			nwkey.i_key[i].i_keylen = -1;
	} else if (strncasecmp("persist:", val, 8) == 0) {
		val += 8;
		/* program keys in persistent memory */
		nwkey.i_wepon |= IEEE80211_NWKEY_PERSIST;
		goto set_nwkey;
	} else {
  set_nwkey:
		if (isdigit((unsigned char)val[0]) && val[1] == ':') {
			/* specifying a full set of four keys */
			nwkey.i_defkid = val[0] - '0';
			val += 2;
			for (i = 0; i < IEEE80211_WEP_NKID; i++) {
				val = get_string(val, ",", keybuf[i],
				    &nwkey.i_key[i].i_keylen);
				if (val == NULL)
					return;
			}
			if (*val != '\0') {
				warnx("SIOCS80211NWKEY: too many keys.");
				return;
			}
		} else {
			val = get_string(val, NULL, keybuf[0],
			    &nwkey.i_key[0].i_keylen);
			if (val == NULL)
				return;
			i = 1;
		}
	}
	for (; i < IEEE80211_WEP_NKID; i++)
		nwkey.i_key[i].i_keylen = 0;
	(void)strncpy(nwkey.i_name, name, sizeof(nwkey.i_name));
	if (ioctl(s, SIOCS80211NWKEY, &nwkey) == -1)
		warn("SIOCS80211NWKEY");
}

void
setifpowersave(const char *val, int d)
{
	struct ieee80211_power power;

	(void)strncpy(power.i_name, name, sizeof(power.i_name));
	if (ioctl(s, SIOCG80211POWER, &power) == -1) {
		warn("SIOCG80211POWER");
		return;
	}

	power.i_enabled = d;
	if (ioctl(s, SIOCS80211POWER, &power) == -1)
		warn("SIOCS80211POWER");
}

void
setifpowersavesleep(const char *val, int d)
{
	struct ieee80211_power power;

	(void)strncpy(power.i_name, name, sizeof(power.i_name));
	if (ioctl(s, SIOCG80211POWER, &power) == -1) {
		warn("SIOCG80211POWER");
		return;
	}

	power.i_maxsleep = atoi(val);
	if (ioctl(s, SIOCS80211POWER, &power) == -1)
		warn("SIOCS80211POWER");
}

void
ieee80211_status(void)
{
	int i, nwkey_verbose;
	struct ieee80211_nwid nwid;
	struct ieee80211_nwkey nwkey;
	struct ieee80211_power power;
	u_int8_t keybuf[IEEE80211_WEP_NKID][16];
	struct ieee80211_bssid bssid;
	struct ieee80211chanreq channel;
	struct ether_addr ea;
	static const u_int8_t zero_macaddr[IEEE80211_ADDR_LEN];

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_data = (void *)&nwid;
	(void)strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
	if (ioctl(s, SIOCG80211NWID, &ifr) == -1)
		return;
	if (nwid.i_len > IEEE80211_NWID_LEN) {
		warnx("SIOCG80211NWID: wrong length of nwid (%d)", nwid.i_len);
		return;
	}
	printf("\tssid ");
	print_string(nwid.i_nwid, nwid.i_len);
	memset(&nwkey, 0, sizeof(nwkey));
	(void)strncpy(nwkey.i_name, name, sizeof(nwkey.i_name));
	/* show nwkey only when WEP is enabled */
	if (ioctl(s, SIOCG80211NWKEY, &nwkey) == -1 ||
	    nwkey.i_wepon == 0) {
		printf("\n");
		goto skip_wep;
	}

	printf(" nwkey ");
	/* try to retrieve WEP keys */
	for (i = 0; i < IEEE80211_WEP_NKID; i++) {
		nwkey.i_key[i].i_keydat = keybuf[i];
		nwkey.i_key[i].i_keylen = sizeof(keybuf[i]);
	}
	if (ioctl(s, SIOCG80211NWKEY, &nwkey) == -1) {
		printf("*****");
	} else {
		nwkey_verbose = 0;
		/* check to see non default key or multiple keys defined */
		if (nwkey.i_defkid != 1) {
			nwkey_verbose = 1;
		} else {
			for (i = 1; i < IEEE80211_WEP_NKID; i++) {
				if (nwkey.i_key[i].i_keylen != 0) {
					nwkey_verbose = 1;
					break;
				}
			}
		}
		/* check extra ambiguity with keywords */
		if (!nwkey_verbose) {
			if (nwkey.i_key[0].i_keylen >= 2 &&
			    isdigit(nwkey.i_key[0].i_keydat[0]) &&
			    nwkey.i_key[0].i_keydat[1] == ':')
				nwkey_verbose = 1;
			else if (nwkey.i_key[0].i_keylen >= 7 &&
			    strncasecmp("persist", nwkey.i_key[0].i_keydat, 7)
			    == 0)
				nwkey_verbose = 1;
		}
		if (nwkey_verbose)
			printf("%d:", nwkey.i_defkid);
		for (i = 0; i < IEEE80211_WEP_NKID; i++) {
			if (i > 0)
				printf(",");
			if (nwkey.i_key[i].i_keylen < 0)
				printf("persist");
			else
				print_string(nwkey.i_key[i].i_keydat,
				    nwkey.i_key[i].i_keylen);
			if (!nwkey_verbose)
				break;
		}
	}
	printf("\n");

 skip_wep:
	(void)strncpy(power.i_name, name, sizeof(power.i_name));
	if (ioctl(s, SIOCG80211POWER, &power) == -1)
		goto skip_power;
	printf("\tpowersave ");
	if (power.i_enabled)
		printf("on (%dms sleep)", power.i_maxsleep);
	else
		printf("off");
	printf("\n");

 skip_power:
	(void)strncpy(bssid.i_name, name, sizeof(bssid.i_name));
	if (ioctl(s, SIOCG80211BSSID, &bssid) == -1)
		return;
	(void)strncpy(channel.i_name, name, sizeof(channel.i_name));
	if (ioctl(s, SIOCG80211CHANNEL, &channel) == -1)
		return;
	if (memcmp(bssid.i_bssid, zero_macaddr, IEEE80211_ADDR_LEN) == 0) {
		if (channel.i_channel != (u_int16_t)-1)
			printf("\tchan %d\n", channel.i_channel);
	} else {
		memcpy(ea.ether_addr_octet, bssid.i_bssid,
		    sizeof(ea.ether_addr_octet));
		printf("\tbssid %s", ether_ntoa(&ea));
		if (channel.i_channel != IEEE80211_CHAN_ANY)
			printf(" chan %d", channel.i_channel);
		printf("\n");
	}
}
