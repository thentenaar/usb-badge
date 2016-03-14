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
#include <getopt.h>

#include "badge.h"

static const char *actions[MAX_ACTION + 1] = {
	"Move",
	"Flash, then Move",
	"Scroll Up",
	"Scroll Down",
	"Flash",
	"Freeze"
};

/**
 * Convert a hexadecimal nibble to binary in place.
 *
 * \param[in] c Pointer to char to convert
 * \return 0 on success, -1 on failure.
 */
static int h2c(char *c)
{
	int x;

	if (!c) goto err;
	x = (int)*c;

	if (x >= 'A' && x <= 'F')
		*c = (char)(x - 'A' + 10);
	else if (x >= '0' && x <= '9')
		*c = (char)(x - '0');
	else goto err;

	return 0;

err:
	fputs("Invalid hex bytes\n", stderr);
	return -1;
}

/**
 * Convert a string of hexadecimal digits to binary (in place.)
 *
 * \param[in] hex Hex string
 * \return Number of bytes converted, or 0 on error.
 */
static size_t hexdec(char *hex)
{
	size_t i, hexlen;

	i = strlen(hex);
	hexlen = ((i % 2) ? i-- : i) >> 1;

	for (i = 0; i < hexlen; i++) {
		if (h2c(hex + (i << 1)) || h2c(hex + 1 + (i << 1)))
			goto err;
		hex[i << 1] = (char)((hex[i << 1] << 8) | hex[1 + (i << 1)]);
	}

	return i;

err:
	return 0;
}

static const char *usage[5] = {
	"USB Badge CLI\n"
	"Copyright (C) 2009-2016 Tim Hentenaar\n\n"
	"Usage: %s [options...]\n",

	"\nOptions:\n"
	"\t-h Show this message\n"
	"\t-d Dump message data. Valid values are: 1 - Dump, 0 - Don't.\n"
	"\t-l Set the brighness of the display. Valid values are 0-7.\n"
	"\t-i Index of message to dump or modify. Valid values are 0-4.\n"
	"\t-a Set the display action. Valid values are:\n"
	"\t\t0 - Move                    1 - Flash, then move\n"
	"\t\t2 - Scroll Up               3 - Scroll Down\n"
	"\t\t4 - Flash                   5 - Freeze\n",

	"\t-s Set the update speed of the message. Valid values are 0-7.\n"
	"\t-m Set the message text (136 chars max.)\n"
	"\t-x Set the message data as a hexadecimal string (136 bytes max.)\n",

	"\nExamples:\n"
	"\tDumping all message data:     %s -d\n"
	"\tDumping a specific message:   %s -d -i <index>\n"
	"\tSetting luminance:            %s -l 2\n"
	"\tSetting speed/action:         %s -i <index> -s 2 -a 1\n"
	"\tUpdating message text:        %s -i <index> -m Message\n",

	"\nNotes:\n"
	"\t-a,-s,-m can be combined to operate in tandum. An index is required "
	"for any of these options.\n"
	"\t-l will work with any valid combination of operands, except -d.\n"
	"\t-d and -a,-s,-m,-l are mutually exclusive. -d takes prescedence.\n"
	"\tThis means that when -d is specified, nothing will be set!\n"
};

static void show_usage(char *pn);

int main(int argc, char *argv[])
{
	struct badge *badge;
	int optc;
	char *message = NULL;
	size_t msglen = 0;
	int dump = 0, action = -1, index = -1, lum = -1, speed = -1, i;

	/* Parse arguments */
	while ((optc = getopt(argc, argv, "hdl:i:a:m:s:x:")) != -1) {
		switch (optc) {
		default:
		case 'h':
			show_usage(argv[0]);
			goto err;
		case 'd':
			dump = 1;
		break;
		case 'm': /* Message */
			if (optarg) {
				message = strdup(optarg);
				msglen = strlen(message);
			}
		break;
		case 'x': /* Message (as a hex string) */
			if (optarg) {
				message = strdup(optarg);
				msglen = hexdec(message);
			}
		break;
		case 'a': /* Action */
		if (optarg) {
			action = (*optarg) - 0x30;
			if (action < MIN_ACTION || action > MAX_ACTION)
				action = -1;
		}
		break;
		case 'i': /* Index */
		if (optarg) {
			index = (*optarg) - 0x30;
			if (index < 0 || action > N_MESSAGES - 1)
				index = -1;
		}
		break;
		case 'l': /* Luminance */
		if (optarg) {
			lum = (*optarg) - 0x30;
			if (lum < MIN_LUMINANCE || lum > MAX_LUMINANCE)
				lum = -1;
		}
		break;
		case 's': /* Speed */
		if (optarg) {
			speed = (*optarg) - 0x30;
			if (speed < MIN_SPEED || speed > MAX_SPEED)
				speed = -1;
		}
		break;
		}
	}

	/* An index must be specified for anything other than luminance */
	if (index == -1 && !dump && lum == -1 &&
	    (speed != -1 || action != -1 || message)) {
		fputs("An index must be specified!\n", stderr);
		goto err;
	}

	/* Open the badge */
	if (!(badge = badge_open())) {
		fputs("Unable to open badge!\n", stderr);
		goto err;
	}

	/* Read all the data on the badge */
	if (badge_get_data()) {
		fputs("Failed to get badge data\n", stderr);
		goto err;
	}

	/* Dump data if requested */
	if (dump) {
		printf("Luminance: %d\n", badge->luminance);
		i = (index == -1) ? 0 : index;
		for (;i<((index == -1) ? 4 : index + 1); i++) {
			printf("Message #%d: Text\n", i + 1);
			printf("\tSpeed: %d\n", badge->messages[i].speed);
			printf("\tAction: %s (%d)\n",
				((badge->messages[i].action <= 5)
				    ? actions[badge->messages[i].action]
				    : "Invalid"),
				    badge->messages[i].action);
			printf("\tText: \"%s\"\n\n", badge->messages[i].data);
		}
		goto ret;
	}

	/* Apply any changes to the badge structure */
	if (lum != -1) badge->luminance = lum & 7;
	if (index != -1) {
		if (action != -1)  badge->messages[index].action = action & 7;
		if (speed  != -1)  badge->messages[index].speed  = speed  & 7;
		if (message) {
			if (msglen > 136) msglen = 136;

			/* This will be free()'d by badge_close */
			badge->messages[index].data = malloc(msglen);
			memcpy(badge->messages[index].data, message, msglen);
			badge->messages[index].length = msglen & 0xff;
		}
	}

	/* Set data */
	if (badge_set_data()) {
		fputs("Failed to set badge data\n", stderr);
		goto err;
	}

ret:
	if (message) free(message);
	badge_close();
	return 0;

err:
	badge_close();
	if (message) free(message);
	exit(EXIT_FAILURE);
}

/* {{{ GCC >= 3.x: ignore -Wformat-security here
 * We know that the contents of usage are okay, even if they
 * aren't passed as string literals.
 */
#ifdef __GNUC__
#if (__GNUC__ >= 3) && (((__GNUC__ * 100) + __GNUC_MINOR__) < 402)
#pragma GCC system_header
#endif /* GCC >= 3 && GCC <= 4.2 */
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 406
#pragma GCC diagnostic push
#endif /* GCC >= 4.6 */
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 402
#pragma GCC diagnostic ignored "-Wformat-security"
#endif /* GCC >= 4.2 */
#endif /* }}} */

/* Display usage info */
static void show_usage(char *pn)
{
	printf(usage[0],pn);
	puts(usage[1]);
	puts(usage[2]);
	printf(usage[3],pn,pn,pn,pn,pn);
	exit(EXIT_FAILURE);
}

/* {{{ GCC >= 3.x: ignore -Wformat-security here
 * We know that the contents of usage are okay, even if they
 * aren't passed as string literals.
 */
#ifdef __GNUC__
#if (__GNUC__ >= 3) && (((__GNUC__ * 100) + __GNUC_MINOR__) < 402)
#pragma GCC system_header
#endif /* GCC >= 3 && GCC <= 4.2 */
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 406
#pragma GCC diagnostic push
#endif /* GCC >= 4.6 */
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 402
#pragma GCC diagnostic ignored "-Wformat-security"
#endif /* GCC >= 4.2 */
#endif /* }}} */

