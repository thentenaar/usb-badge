/**
 * Control Software for the Inland (FURI KEYSHINE) USB LED Badge
 * Copyright (C) 2009-2016 Tim Hentenaar.
 *
 * This code is licenced under the Simplified BSD License.
 * See the LICENSE file for details.
 */

#ifndef BADGE_H
#define BADGE_H

/**
 * Message types
 */
#define BADGE_MSG_TYPE_TEXT   0
#define BADGE_MSG_TYPE_BITMAP 1

/**
 * Bounds for luminance value
 */
#define MIN_LUMINANCE 2 
#define MAX_LUMINANCE 4

/**
 * Bounds for message speed
 */
#define MIN_SPEED 0
#define MAX_SPEED 7

/**
 * Bounds for action
 */
#define MIN_ACTION 0
#define MAX_ACTION 5

/**
 *
 */
struct badge_message {
	unsigned char type;
	size_t        length;
	unsigned char speed;
	unsigned char action;
	unsigned char *data;
};

#define N_MESSAGES 6

/**
 *
 */
struct badge {
	unsigned char        luminance;
	struct badge_message messages[N_MESSAGES];
};

/**
 * Claim the first badge found.
 *
 * \return pointer to the \a badge struct if found, NULL otherwise.
 */
struct badge *badge_open(void);

/**
 * Set all data on the badge.
 *
 * \return 0 on success, -1 on error.
 */
int badge_set_data(void);

/**
 * Get all values from the badge.
 *
 * \return 0 on success, -1 on error.
 */
int badge_get_data(void);

/**
 * Release the badge.
 */
void badge_close(void);

#endif	/* BADGE_H */
