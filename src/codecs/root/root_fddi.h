/*
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#ifndef ROOT_FDDI_H
#define ROOT_FDDI_H


/* Start FDDI */
#define FDDI_ALLC_LEN                   13
#define FDDI_ALEN                       6
#define FDDI_MIN_HLEN                   (FDDI_ALLC_LEN + 3)

#define FDDI_DSAP_SNA                   0x08    /* SNA */
#define FDDI_SSAP_SNA                   0x00    /* SNA */
#define FDDI_DSAP_STP                   0x42    /* Spanning Tree Protocol */
#define FDDI_SSAP_STP                   0x42    /* Spanning Tree Protocol */
#define FDDI_DSAP_IP                    0xaa    /* IP */
#define FDDI_SSAP_IP                    0xaa    /* IP */

#define FDDI_ORG_CODE_ETHR              0x000000    /* Encapsulated Ethernet */
#define FDDI_ORG_CODE_CDP               0x00000c    /* Cisco Discovery */
#define ETHERNET_TYPE_CDP               0x2000    /* Cisco Discovery Protocol */


void DecodeFDDIPkt(Packet *, const DAQ_PktHdr_t*, const uint8_t *);


#endif

