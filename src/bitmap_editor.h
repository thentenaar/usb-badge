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

#ifndef BITMAP_EDITOR_H
#define BITMAP_EDITOR_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>

/**
 * The bitmap editor widget.
 *
 * Basically, this uses a GtkImage with a GdkPixbuf/GdkPixmap to draw both
 * the large image (for the dialog), and the small image (drawn to the main window.)
 */
typedef struct bitmap_editor {
	GtkWidget     *image;		/**< The large image is 16 : 1 of the actual bitmap */
	GtkWidget     *evbox;
	GdkPixmap     *pixmap;
	GdkGC         *gc;
	GtkWidget     *dialog;
	GtkWidget     *scroll;
	GtkWidget     *popup;
	GtkWidget     *image_small; /**< The small image is 3 : 1 of the actual bitmap. */
	GtkWidget     *evbox_small;
	GdkPixmap     *pixmap_small;
	GdkGC         *gc_small;
	unsigned char **bitmap;	/**< The bitmap we'll send to the device */
	unsigned short length;	/**< Used columns (bytes) */
} bitmap_editor_t;

bitmap_editor_t *bitmap_editor_new(unsigned char **bmp, unsigned short ncols);
void bitmap_editor_free(bitmap_editor_t *ed);

#endif	/* BITMAP_EDITOR_H */
/* vim: set ts=4: */
