/**
 * Control Software for the Inland (FURI KEYSHINE) USB LED Badge
 * Copyright (C) 2009-2016 Tim Hentenaar.
 *
 * This code is licenced under the Simplified BSD License.
 * See the LICENSE file for details.
 */

#ifndef BITMAP_EDITOR_H
#define BITMAP_EDITOR_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>

/**
 * The bitmap editor widget.
 *
 * Basically, this uses a GtkImage with a GdkPixbuf/GdkPixmap to draw
 * both the large image (for the dialog), and the small image (drawn to
 * the main window.)
 */
struct bitmap_editor {
	GtkWidget     *image; /**< 16 : 1 scale of the actual bitmap */
	GtkWidget     *evbox;
	GdkPixmap     *pixmap;
	GdkGC         *gc;
	GtkWidget     *dialog;
	GtkWidget     *scroll;
	GtkWidget     *popup;
	GtkWidget     *image_small; /**< 3 : 1 scale of the actual bitmap. */
	GtkWidget     *evbox_small;
	GdkPixmap     *pixmap_small;
	GdkGC         *gc_small;
	unsigned char **bitmap; /**< The bitmap we'll send to the device */
	unsigned int    length; /**< Used columns (bytes) */
};

struct bitmap_editor *bitmap_editor_new(unsigned char **bmp,
                                        unsigned int ncols);
void bitmap_editor_free(struct bitmap_editor *ed);

#endif	/* BITMAP_EDITOR_H */
