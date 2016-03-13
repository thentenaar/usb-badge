/**
 * Control Software for the Inland (FURI KEYSHINE) USB LED Badge
 * Copyright (C) 2009-2016 Tim Hentenaar.
 *
 * This code is licenced under the Simplified BSD License.
 * See the LICENSE file for details.
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
