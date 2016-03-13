/**
 * Control Software for the Inland (FURI KEYSHINE) USB LED Badge
 * Copyright (C) 2009-2016 Tim Hentenaar.
 *
 * This code is licenced under the Simplified BSD License.
 * See the LICENSE file for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hidapi/hidapi.h>
#include "badge.h"

/* Badge VID, PID, and interface */
#define BADGE_VID       0x04d9
#define BADGE_PID       0xe002
#define BADGE_INTERFACE 0

/* Usage page and Usage */
#define BADGE_USAGE_PAGE 0xffa0
#define BADGE_USAGE      0x0001

/* Report payload size */
#define REPORT_SIZE 9

/**
 * Badge Protocol (Report #0)
 *
 * Data (16-bit words):
 *	word 1: 0x55aa - ??
 *	word 2: 0x0100 - Get Data
 *	        0x0200 - Set Data
 *	word 3: address
 *	word 4: length
 *
 * Data follows.
 *
 * Addresses:
 *	Lumanance data is at 0x02.
 *	Message 1 starts at 0x08.
 *	Message 2 starts at 0x98.
 *	Message 3 starts at 0x128.
 *	Message 4 starts at 0x1B8.
 *	Message 5 starts at 0x248
 *	Message 6 starts at 0x508.
 */
static const unsigned char report[REPORT_SIZE]  = {
	0x00, 0x55, 0xaa, 0x02, 0x00, 0x00, 0x00, 0x08, 0x00
};

static struct badge badge;
static hid_device *device = NULL;
static unsigned char buf[REPORT_SIZE];

/**
 * Claim the first badge found.
 *
 * \return pointer to the \a badge struct if found, NULL otherwise.
 */
struct badge *badge_open(void)
{
	int is_hidraw = 0;
	struct hid_device_info *devs = NULL, *cur_dev;

	hid_init();
	memset(&badge, 0, sizeof(struct badge));
	devs = hid_enumerate(BADGE_VID, BADGE_PID);
	if (!devs) goto err;

	cur_dev = devs;
	do {
		/* XXX: hidapi's usage page info is worthless with hidraw */
		is_hidraw = strstr(cur_dev->path, "/dev/") != NULL;

		if (!is_hidraw &&
		    cur_dev->usage      == BADGE_USAGE &&
		    cur_dev->usage_page == BADGE_USAGE_PAGE)
			break;

		/* Search by interface if we don't have the usage info */
		if ((is_hidraw || (!cur_dev->usage && !cur_dev->usage_page)) &&
		    cur_dev->interface_number == BADGE_INTERFACE)
			break;
		cur_dev = cur_dev->next;
	} while (cur_dev);

	if (!cur_dev || !(device = hid_open_path(cur_dev->path)))
		goto err;

	/* Free the enumeration data */
	hid_free_enumeration(devs);
	return &badge;

err:
	if (devs) hid_free_enumeration(devs);
	return NULL;
}

/**
 * Set all data on the badge.
 *
 * \return 0 on success, -1 on error.
 */
int badge_set_data(void)
{
	size_t len;
	unsigned int address = 0, i, j;
	if (!device) goto err;

	if (badge.luminance < MIN_LUMINANCE)
		badge.luminance = MIN_LUMINANCE;

	if (badge.luminance > MAX_LUMINANCE)
		badge.luminance = MAX_LUMINANCE;

	/* Set luminance */
	memcpy(buf, report, REPORT_SIZE);
	if (hid_write(device, buf, REPORT_SIZE) < 0)
		goto err;

	memset(buf, 0, REPORT_SIZE);
	buf[1] = report[2];
	buf[2] = report[1];
	buf[3] = badge.luminance;
	if (hid_write(device, buf, REPORT_SIZE) < 0)
		goto err;

	/* Set messages */
	memcpy(buf, report, REPORT_SIZE);
	for (i = 0; i < N_MESSAGES; i++, address += 0x88) {
		if (i == 5) address = 0x0508;
		else address += 8;
		len = badge.messages[i].length + 4;

		/**
		 * Set the destination address and message
		 * length.
		 */
		buf[5] = address & 0xff;
		buf[6] = (address >> 8) & 0xff;
		buf[7] = len & 0xff;
		buf[8] = (len >> 8) & 0xff;
		if (hid_write(device, buf, REPORT_SIZE) < 0)
			goto err;

		if (badge.messages[i].speed > MAX_SPEED)
			badge.messages[i].speed = MAX_SPEED;

		/**
		 * Write the message properties, and the first
		 * 4 bytes of the message's data.
		 */
		len -= 4;
		buf[1] = len & 0xff;
		buf[2] = (len >> 8) & 0xff;
		buf[3] = badge.messages[i].speed;
		buf[4] = badge.messages[i].action;
		memcpy(buf + 5, badge.messages[i].data, (len < 4) ? len : 4);
		if (hid_write(device, buf, REPORT_SIZE) < 0)
			goto err;

		/* Write the remainder of the message data. */
		for (j = 4; j < len; j += 8) {
			memset(buf, 0, REPORT_SIZE);
			memcpy(buf+1, badge.messages[i].data + j,
			       ((len - j) < 8) ? (len - j) : 8);
			if (hid_write(device, buf, REPORT_SIZE) < 0)
				goto err;
		}
	}

	return 0;

err:
	return -1;
}

int badge_get_data(void)
{
	size_t len;
	unsigned int i, j, address, tmp;
	if (!device) goto err;

	/* Get luminance */
	memcpy(buf, report, REPORT_SIZE);
	buf[3] = 0x01;
	if (hid_write(device, buf, REPORT_SIZE) < 0 ||
	    hid_read_timeout(device, buf, REPORT_SIZE, 250) < 0)
		goto err;
	badge.luminance = buf[3];

	/* Get messages */
	for (i = 0, address = 0x08; i < N_MESSAGES; i++, address += 0x88) {
		memcpy(buf, report, REPORT_SIZE);
		buf[3] = 0x01;
		if (i == 5) address = 0x0508;
		else address += 8;

		/* Get the message properties */
		buf[5] = address & 0xff;
		buf[6] = (address >> 8) & 0xff;
		if (hid_write(device, buf, REPORT_SIZE) < 0 ||
		    hid_read_timeout(device, buf, REPORT_SIZE, 250) < 0)
			goto err;

		badge.messages[i].type =  (i < 4) ? BADGE_MSG_TYPE_TEXT :
		                                    BADGE_MSG_TYPE_BITMAP;
		badge.messages[i].speed  = buf[2];
		badge.messages[i].action = buf[3];
		badge.messages[i].length = (unsigned)((buf[1] << 8) | buf[0]);

		if (i < 4 && badge.messages[i].length > 0x88)
			badge.messages[i].length = 0;

		if (!badge.messages[i].length)
			continue;

		/* Allocate space for the message */
		badge.messages[i].data = malloc(badge.messages[i].length);
		if (!badge.messages[i].data)
			goto err;

		/* Copy the first four bytes */
		memcpy(badge.messages[i].data, buf + 4, 4);

		/* Get the rest of the message data */
		tmp = address + 8;
		len = badge.messages[i].length;
		for (j = 4; j < badge.messages[i].length; j += 8, tmp += 8) {
			memcpy(buf, report, REPORT_SIZE);
			buf[3] = 0x01;
			buf[5] = tmp & 0xff;
			buf[6] = (tmp >> 8) & 0xff;
			if (hid_write(device, buf, REPORT_SIZE) < 0 ||
			    hid_read_timeout(device, buf, REPORT_SIZE, 250) < 0)
				goto err;
			memcpy(badge.messages[i].data + j, buf,
			       ((len - j) < 8) ? len - j : 8);
		}

	}

	return 0;
err:
	return -1;
}

/**
 * Release the badge.
 */
void badge_close(void)
{
	int i;

	for (i = 0;i < N_MESSAGES; i++) {
		if (badge.messages[i].data)
			free(badge.messages[i].data);
	}

	hid_close(device);
	hid_exit();
	memset(&badge, 0, sizeof(struct badge));
	device = NULL;
}

