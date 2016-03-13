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

#include "badge.h"

static const char *actions[MAX_ACTION + 1] = {
	"Move",
	"Flash, then Move",
	"Scroll Up",
	"Scroll Down",
	"Flash",
	"Freeze"
};

int main(int argc, char *argv[])
{
	int i; struct badge *badge;
	(void)argc;
	(void)argv;

	/* Allocate a new badge structure, as well as the USB device */
	if (!(badge = badge_open())) {
		fputs("Unable open badge\n", stderr);
		goto err;
	}

	/* Read all the data on the badge */
	if (badge_get_data()) {
		fputs("Unable to read badge data\n", stderr);
		goto err;
	}

	/* Dump the data */
	printf("Luminance: %d\n",badge->luminance);
	for (i=0; i<N_MESSAGES; i++) {
		printf("Message #%d: %s\n", i + 1,
		       badge->messages[i].type ? "Bitmap" : "Text");
		printf("\tSpeed: %d\n", badge->messages[i].speed + 1);
		printf("\tAction: %s\n",
		       ((badge->messages[i].action <= 5) ?
		         actions[badge->messages[i].action] : "Invalid"));
		if (i < 4) printf("\tText: \"%s\"\n", badge->messages[i].data);
		printf("\n");
	}

	if (badge->messages[0].data)
		free(badge->messages[0].data);

	/* Set luminance and a message */
	badge->luminance = 2;
	badge->messages[0].speed  = 3;
	badge->messages[0].action = 5;
	badge->messages[0].data   = malloc(6); /* free()'d in badge_close */
	badge->messages[0].length = 5;
	memcpy(badge->messages[0].data, "Linux", 6);

	if (badge_set_data()) {
		fputs("Unable to set badge data\n", stderr);
		goto err;
	}

	badge_close();
	return EXIT_SUCCESS;

err:
	badge_close();
	exit(EXIT_FAILURE);
}

