/**
 * Control Software for the Inland (FURI KEYSHINE) USB LED Badge
 * Copyright (C) 2011 Tim Hentenaar. http://hentenaar.com
 *
 * CLI code and udev rules proposed and contributed by Jeff Jahr - http://www.jeffrika.com adapted and integrated by Tim Hentenaar.
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
#include <getopt.h>

#include "badge.h"

const char *actions[6] = {
	"Move",
	"Flash, then Move",
	"Scroll Up",
	"Scroll Down",
	"Flash",
	"Freeze"
};

/* Macro for handling simple integer args */
#define PROCESS_ARG(A,I,M) {\
	case A :\
		if (optarg) {\
			args[I] = (*optarg)-0x30;\
			args[I] = (args[I] < 0 || args[I] > M) ? -1 : args[I];\
		}\
	break;\
}

/* Display usage info */
void usage(char *pn) {
	printf("USB Badge CLI\n");
	printf("Copyright (C) 2011 Tim Hentenaar. http://hentenaar.com\n");
	printf("With contributions by Jeff Jahr - http:///www.jeffrika.com\n\n");

	printf("This program is covered under the terms of the GNU General Public License,\n");
	printf("version 3 or later. See http://www.gnu.org/licenses/gpl.txt for details.\n\n");

	printf("Usage: %s [options]\n\n",pn);

	printf("Options:\n");
	printf("\t-h Show this message\n");
	printf("\t-d Dump message data. Valid values are: 1 - Dump, 0 - Don't dump.\n");
	printf("\t-l Set the brighness of the display. Valid values are 0-7.\n");
	printf("\t-i Index of message to dump or modify. Valid values are 0-4.\n");
	printf("\t-a Set the display action. Valid values are:\n");
	printf("\t\t0 - Move                    1 - Flash, then move\n");
	printf("\t\t2 - Scroll Up               3 - Scroll Down\n");
	printf("\t\t4 - Flash                   5 - Freeze\n");
	printf("\t-s Set the update speed of the message. Valid values are 0-7.\n");
	printf("\t-m Set the message text (136 chars max.)\n\n");

	printf("Examples:\n");
	printf("\tDumping all message data:     %s -d\n",pn);
	printf("\tDumping a specific message:   %s -d -i <index>\n",pn);
	printf("\tSetting luminance:            %s -l 2\n",pn);
	printf("\tSetting speed/action:         %s -i <index> -s 2 -a 1\n",pn);
	printf("\tUpdating message text:        %s -i <index> -m Message\n\n",pn);

	printf("Notes:\n");
	printf("\t-a,-s,-m can be combined to operate in tandum. An index is required for any of these options.\n");
	printf("\t-l will work with any valid combination of operands, except -d.\n");
	printf("\t-d and -a,-s,-m,-l are mutually exclusive. -d takes prescedence.\n\tThis means that when -d is specified, nothing will be set!\n\n");
}

int main(int argc, char **argv) {
	int i; badge_t *badge; int optc,args[4] = { -1,-1,-1,-1 },dump=0; 
	char *message=NULL;

	/* Parse arguments */
	while ((optc = getopt(argc,argv,"hdl:i:a:m:s:")) != -1) {
		switch (optc) {
			default: 
			case 'h': usage(argv[0]); exit(0);              break; 
			case 'd': dump = 1;                             break;
			case 'm': if (optarg) message = strdup(optarg); break;
			PROCESS_ARG('a',0,5);
			PROCESS_ARG('i',1,3);
			PROCESS_ARG('l',2,4);
			PROCESS_ARG('s',3,7);
		}
	}

	/* An index must be specified... */
	if (args[1] == -1 && !dump && args[2] == -1) {
		printf("An index must be specified for setting anything other than luminance!\n");
		if (message) free(message);
		exit(1);
	}

	/* Allocate a new badge structure, as well as the USB device */
	if (!(badge = badge_new())) {
		printf("Unable to allocate badge structure or insufficent permissions!\n");
		if (message) free(message);
		exit(1);
	}

	/* Read all the data on the badge */
	badge_get_data(badge);

	/* Dump data if requested */
	if (dump) {
		printf("Luminance: %d\n",badge->luminance);
		for (i=((args[1] == -1) ? 0 : args[1]);i<((args[1] == -1) ? 4 : (args[1]+1));i++) {
			printf("Message #%d: Text\n",i+1);
			printf("\tSpeed: %d\n",badge->messages[i].speed);
			printf("\tAction: %s (%d)\n",((badge->messages[i].action <= 5) ? actions[badge->messages[i].action] : "Invalid"),badge->messages[i].action);
			printf("\tText: \"%s\"\n\n",badge->messages[i].data);
		}
	} else {
		/* Apply any changes to the badge structure */
		if (args[2] > -1) badge->luminance = args[2];
		if (args[1] != -1) {
			if (args[0] > -1)  badge->messages[args[1]].action = args[0];
			if (args[3] > -1)  badge->messages[args[1]].speed  = args[3];
			if (message && args[1] < 4) {
				if (strlen(message) > 136) *(message+136) = 0;
				badge->messages[args[1]].data   = strdup(message);
				badge->messages[args[1]].length = strlen(message);
			}
		}

		/* Set data */
		badge_set_data(badge);
	}

	if (message) free(message);
	badge_free(badge);
	return 0;
}


/* vim: set ts=4: */
