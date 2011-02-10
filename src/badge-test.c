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

#include "badge.h"

const char *actions[6] = {
	"Move",
	"Flash, then Move",
	"Scroll Up",
	"Scroll Down",
	"Flash",
	"Freeze"
};

int main(int argc, char **argv) {
	int i; badge_t *badge;

	/* Allocate a new badge structure, as well as the USB device */
	if (!(badge = badge_new())) {
		printf("Unable to allocate badge structure or insufficent permissions!\n");
		exit(1);
	}

	/* Read all the data on the badge */
	badge_get_data(badge);

	/* Dump the data */
	printf("Luminance: %d\n",badge->luminance);
	for (i=0;i<6;i++) {
		printf("Message #%d: %s\n",i+1,(badge->messages[i].type ? "Bitmap" : "Text"));
		printf("\tSpeed: %d\n",badge->messages[i].speed+1);
		printf("\tAction: %s\n",((badge->messages[i].action <= 5) ? actions[badge->messages[i].action] : "Invalid"));
		if (i < 4) printf("\tText: \"%s\"\n",badge->messages[i].data);
		printf("\n");
	}

	/* Set luminance and a message */
	badge->luminance = 2;
	badge->messages[0].speed  = 3;
	badge->messages[0].action = 5; free(badge->messages[0].data);
	badge->messages[0].data   = strdup("Linux");
	badge->messages[0].length = strlen(badge->messages[0].data);

	badge_set_data(badge);
	badge_free(badge);
	return 0;
}
/* vim: set ts=4: */
