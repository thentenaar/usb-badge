/**
 * Control Software for the Inland (FURI KEYSHINE) USB LED Badge
 * Copyright (C) 2009 Tim Hentenaar. http://hentenaar.com
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the 
 * Free Software Foundation, either version 3 of the License, or (at your option) any later version. This program is distributed in the hope that 
 * it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this 
 * program. If not, see http://www.gnu.org/licenses/. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hid.h>
#include <endian.h>

#include "badge.h"

/* Macros to handle byte swapping */
#if __BYTE_ORDER == __BIG_ENDIAN 
#define SET_OFFSET(b,x) {\
	*(unsigned char *)(b+4) = ((x) & 0x00ff);\
	*(unsigned char *)(b+5) = (((x) & 0xff00) >> 8);\
}
#else
#define SET_OFFSET(b,x) *(unsigned short *)(b+4) = (x);
#endif

#if __BYTE_ORDER == __BIG_ENDIAN 
#define SET_LEN(b,x) {\
	*(unsigned char *)(b+6) = ((x) & 0x00ff);\
	*(unsigned char *)(b+7) = (((x) & 0xff00) >> 8);\
}
#else
#define SET_LEN(b,x) *(unsigned short *)(b+6) = (x);
#endif

#if __BYTE_ORDER == __BIG_ENDIAN
#define BADGE_SWAP16(x) ((((x) & 0xff00) >> 8) | (((x) & 0x00ff) << 8))
#else
#define BADGE_SWAP16(x) (x)
#endif

static unsigned int  did_libhid_init = 0;

#define BADGE_USBVEND	0x04d9
#define BADGE_USBPROD	0xe002

/* HID Paths */
#define PATH_LEN 2
static const unsigned int PATH_IN[2] = { 0xffa00001, 0x00000000 }; /* 1st usage of data page */

/* Badge Protocol 
 * 
 * Control Packet:
 * 	word 1: 0x55aa - ??
 * 	word 2: 0x0100 - Get Data
 *          0x0200 - Set Data
 * 	word 3: address (little endian)
 * 	word 4: length (little endian)
 *
 * Data follows.
 *
 * Addresses:
 * 	Lumanance data is at 0x02.
 * 	Message 1 starts at 0x08.
 * 	Message 2 starts at 0x98.
 * 	Message 3 starts at 0x128.
 * 	Message 4 starts at 0x1B8.
 * 	Message 5 starts at 0x248
 * 	Message 6 starts at 0x508.
 */
static const unsigned char packet[8]  = { 0x55, 0xAA, 0x01, 0x00, 0x00, 0x00, 0x08, 0x00 };

badge_t *badge_new() {
	badge_t *badge;
	HIDInterface *hid;
	HIDInterfaceMatcher matcher = {BADGE_USBVEND, BADGE_USBPROD, NULL, NULL, 0};

	/* Initialize libhid */
	if (!did_libhid_init) {
		if (hid_init() != HID_RET_SUCCESS) return NULL;
		did_libhid_init = 1;
	}

	/* Allocate a new badge_t */
	if (!(badge = calloc(sizeof(badge_t),1))) {
		hid_cleanup();
		return NULL;
	}

	/* Allocate a new HIDInterface */
	if (!(hid = hid_new_HIDInterface())) {
		free(badge);
		hid_cleanup();
		return NULL;
	}

	/* Detach any kernel drivers hogging the device, and claim it. */
	if (hid_force_open(hid,0,&matcher,3) != HID_RET_SUCCESS) {
		free(badge);
		hid_cleanup();
		return NULL;
	}

	badge->device = (void *)hid;
	return badge;
}

void badge_set_data(badge_t *badge) {
	unsigned char *buf, *tb; unsigned short i,j,tmp;

	if (!badge || !badge->device) return;
	if (!(buf = calloc(8,1))) return;

	/* Set luminance */
	memcpy(buf,packet,8); buf[2] = 0x02; 
	hid_set_output_report((HIDInterface *)(badge->device),PATH_IN,PATH_LEN,buf,8);
	buf[0] = packet[1]; buf[1] = packet[0]; buf[2] = (badge->luminance & 0xff);
	hid_set_output_report((HIDInterface *)(badge->device),PATH_IN,PATH_LEN,buf,8);

	/* Set messages */
	memcpy(buf,packet,8); buf[2] = 0x02; 

	for (j=0;j<6;j++) {
		tmp = *(unsigned short *)(buf+4) + 8; SET_OFFSET(buf,tmp);
		if (j == 5) SET_OFFSET(buf,0x508);
		SET_LEN(buf,badge->messages[j].length + 4);
		hid_set_output_report((HIDInterface *)(badge->device),PATH_IN,PATH_LEN,buf,8);

		tb = calloc(8,1);
		*(unsigned short *)(tb)  = BADGE_SWAP16(badge->messages[j].length);
		*(unsigned char *)(tb+2) = badge->messages[j].speed;
		*(unsigned char *)(tb+3) = badge->messages[j].action;
		memcpy(tb+4,badge->messages[j].data,(badge->messages[j].length < 4) ? badge->messages[j].length : 4);
		hid_set_output_report((HIDInterface *)(badge->device),PATH_IN,PATH_LEN,tb,8);
		free(tb);

		for (i=4;i<badge->messages[j].length;i+=8) {
			if ((badge->messages[j].length - i) < 8) {
				tb = calloc(8,1);
				memcpy(tb,badge->messages[j].data+i,badge->messages[j].length-i);
			  	hid_set_output_report((HIDInterface *)(badge->device),PATH_IN,PATH_LEN,tb,8);
				free(tb);
			} else hid_set_output_report((HIDInterface *)(badge->device),PATH_IN,PATH_LEN,badge->messages[j].data+i,8);
		}
		tmp = *(unsigned short *)(buf+4) + 0x88; SET_OFFSET(buf,tmp);
	}

	free(buf);
}

void badge_get_data(badge_t *badge) {
	unsigned char *iobuf, *buf, *tb; unsigned short i,j,tmp,tmp2;

	if (!badge || !badge->device) return;
	if (!(buf = calloc(8,1))) return;
	if (!(iobuf = calloc(8,1))) {
		free(buf);
		return;
	}

	/* Get luminance */
	hid_set_output_report((HIDInterface *)(badge->device),PATH_IN,PATH_LEN,packet,8);
	hid_interrupt_read((HIDInterface *)(badge->device),USB_ENDPOINT_IN+1,iobuf,8,0);
	badge->luminance = iobuf[2];

	/* Get messages */
	memcpy(buf,packet,8);
	memset(iobuf,0,8); 
	tb = calloc(8,1);
	memcpy(tb,buf,8);

	for (j=0;j<6;j++) {
		tmp = *(unsigned short *)(buf+4) + 8; SET_OFFSET(buf,tmp); 
		if (j == 5) SET_OFFSET(buf,0x508);
		hid_set_output_report((HIDInterface *)(badge->device),PATH_IN,PATH_LEN,buf,8);
		hid_interrupt_read((HIDInterface *)(badge->device),USB_ENDPOINT_IN+1,iobuf,8,0);
		badge->messages[j].type   = ((j < 4) ? BADGE_MSG_TYPE_TEXT : BADGE_MSG_TYPE_BITMAP);
		badge->messages[j].length = (j >= 4 || (j < 4 && BADGE_SWAP16(*(unsigned short *)(iobuf)) <= 0x88)) ? BADGE_SWAP16(*(unsigned short *)(iobuf)) : 0;
		badge->messages[j].speed  = *(unsigned char *)(iobuf+2);
		badge->messages[j].action = *(unsigned char *)(iobuf+3);

		if (!(badge->messages[j].data = calloc(badge->messages[j].length+1,1))) {
			free(iobuf);
			free(buf);
			free(tb);
			return;
		} if (badge->messages[j].length > 0) memcpy(badge->messages[j].data,iobuf+4,4); i = 4;

		/* Get the next 8 bytes, etc. */
		memcpy(tb,buf,8);
		while (i < badge->messages[j].length) {	
			tmp2 = *(unsigned short *)(tb+4) + 8; SET_OFFSET(tb,tmp2);
			hid_set_output_report((HIDInterface *)(badge->device),PATH_IN,PATH_LEN,tb,8);
			hid_interrupt_read((HIDInterface *)(badge->device),USB_ENDPOINT_IN+1,iobuf,8,0);
			if ((badge->messages[j].length - i) < 8) 
				 memcpy(badge->messages[j].data+i,iobuf,(badge->messages[j].length - i));
			else memcpy(badge->messages[j].data+i,iobuf,8);
			i += 8;
		}

		tmp = *(unsigned short *)(buf+4) + 0x88; SET_OFFSET(buf,tmp);
	}

	free(tb);
	free(iobuf);
	free(buf);
}

void badge_free(badge_t *badge) {
	unsigned int i;
	if (!badge) return;

	for (i=0;i<6;i++) if (badge->messages[i].data) free(badge->messages[i].data);
	hid_close((HIDInterface *)(badge->device));
	hid_delete_HIDInterface((HIDInterface **)&(badge->device));
	hid_cleanup();
	free(badge);
}

/* vim: set ts=4: */
