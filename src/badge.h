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

#ifndef BADGE_H
#define BADGE_H

enum badge_msg_type{
	BADGE_MSG_TYPE_TEXT  = 0,
	BADGE_MSG_TYPE_BITMAP
};

typedef struct badge_message {
	enum badge_msg_type type;
	unsigned short length;
	unsigned char  speed;
	unsigned char  action;
	char          *data;
} badge_message_t;

typedef struct badge {
	unsigned int     luminance;
	badge_message_t  messages[6];
	void            *device;
} badge_t;

/**
 * Create a new instance of badge_t and claim the first badge found.
 */
badge_t *badge_new();

/**
 * Set all data on the badge 
 */
void badge_set_data(badge_t *badge);

/**
 * Get all values from the badge.
 */
void badge_get_data(badge_t *badge);

/**
 * Free the badge
 */
void badge_free(badge_t *badge);

#endif	/* BADGE_H */
/* vim: set ts=4: */
