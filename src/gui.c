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

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <gdk-pixbuf/gdk-pixdata.h>

#include "icon.h"
#include "badge.h"
#include "bitmap_editor.h"

/* Imported by bitmap_editor.c */
GtkWidget *window;

static GtkWidget *dialog;
static GtkWidget *text[4];
static GtkWidget *spin[6];
static GtkWidget *combo[6];
static GtkWidget *progress;
static GtkWidget *lum;
static struct bitmap_editor *bitmp[2];
static gchar *row_text[6];
static struct badge *badge;

/**
 * Called when the user clicks the "Send" button.
 */
static void send_cb(GtkWidget *widget, gpointer data)
{
	int i; gchar *tmp;
	(void)widget;
	(void)data;

	/* Read new data from the UI */
	i = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lum));
	badge->luminance = (i - 1) & 7;

	for (i = 0; i < N_MESSAGES; i++) {
		badge->messages[i].speed  = (gtk_spin_button_get_value_as_int(
		       GTK_SPIN_BUTTON(spin[i])) - 1) & 7;
		badge->messages[i].action = (gtk_combo_box_get_active(
		       GTK_COMBO_BOX(combo[i]))) & 7;

		if (i < 4) { /* Text (ASCII or ISO-8859-1 per the docs) */
			tmp = g_convert(gtk_entry_get_text(GTK_ENTRY(text[i])),
			                -1, "ISO-8859-1", "UTF-8", NULL,
			                NULL, NULL);
			free(badge->messages[i].data);
			badge->messages[i].data   = (unsigned char *)strdup(tmp);
			badge->messages[i].length = strlen(tmp);
			g_free(tmp);
		} else badge->messages[i].length = bitmp[i - 4]->length;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress),
		                              (gdouble)((i + 1) / 8));
	}

	/* Send it to the device */
	if (badge_set_data()) {
		g_object_set(dialog, "secondary-text",
		             _("Failed to update the badge!"), NULL);
		gtk_dialog_run(GTK_DIALOG(dialog));
	}

	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), 1.0);
}
/**
 * Called when the user requests that the window be closed.
 */
static gboolean window_closed(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	(void)widget;
	(void)event;
	(void)data;
	gtk_main_quit();
	return FALSE;
}

int main(int argc, char *argv[])
{
	unsigned int i;
	GdkPixbuf *pb;
	GtkWidget *vbox, *table, *hbox, *button;

	bitmp[0] = bitmp[1] = NULL;
	memset(row_text,0,sizeof(row_text));

	/* Initialize GTK */
	gtk_init(&argc, &argv);

	/* Create the error dialog */
	dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL,
	                                GTK_MESSAGE_ERROR,
	                                GTK_BUTTONS_CLOSE,
	                                "Error");

	badge = badge_open();

	/* Allocate a new badge structure, and open the device */
	if (!(badge = badge_open())) {
		g_object_set(dialog, "secondary-text",
		             _("Unable to open the badge!"), NULL);
		goto err;
	}

	/* Load data from the badge */
	if (badge_get_data()) {
		g_object_set(dialog, "secondary-text",
		             _("Unable to load data from the badge!"), NULL);
		goto err;
	}

	/* Create the window, and the "closed" handler */
	pb = gdk_pixbuf_from_pixdata(&WINDOW_ICON, FALSE, NULL);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), _("USB LED Badge"));
	gtk_window_set_icon(GTK_WINDOW(window), pb);
	gtk_window_set_default_size(GTK_WINDOW(window), 415, 275);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	g_signal_connect(G_OBJECT(window), "delete_event",
	                 G_CALLBACK(window_closed), NULL);
	g_object_unref(pb);

	/**
	 * Create the layout.
	 *
	 * This layout is loosely based on the manufacturer's supplied
	 * program.
	 */
	vbox  = gtk_vbox_new(FALSE, 0);
	hbox  = gtk_hbox_new(FALSE, 0);
	table = gtk_table_new(7, 4, FALSE);

	/* Table Headers */
	gtk_table_attach_defaults(GTK_TABLE(table),
	                          gtk_label_new(_("#")), 0, 1, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table),
	                          gtk_label_new(_("Message")), 1, 2, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table),
	                          gtk_label_new(_("Speed")), 2, 3, 0, 1);
	gtk_table_attach_defaults(GTK_TABLE(table),
	                          gtk_label_new(_("Action")), 3, 4, 0, 1);

	/* Rows */
	for (i = 1; i < 7; i++) {
		row_text[i - 1] = g_strdup_printf("%d", i);
		gtk_table_attach_defaults(GTK_TABLE(table),
		                gtk_label_new(row_text[i - 1]), 0, 1, i, i + 1);

		if (i < 5) {
			/* Text entry */
			text[i - 1] = gtk_entry_new_with_max_length(136);
			gtk_entry_set_text(GTK_ENTRY(text[i - 1]),
			                   (gchar *)(badge->messages[i - 1].data));
			gtk_table_attach_defaults(GTK_TABLE(table),
			                          text[i - 1], 1, 2, i, i + 1);
		} else {
			/* Bitmap editors */
			bitmp[i - 5] = bitmap_editor_new(
				(unsigned char **)&badge->messages[i - 1].
				data, (unsigned int)badge->messages[i - 1].length);
			gtk_table_attach_defaults(GTK_TABLE(table),
			               bitmp[i - 5]->evbox_small,
			               1, 2, i, i + 1);
		}

		/* Spin button for speed */
		spin[i - 1] = gtk_spin_button_new_with_range(1, 8, 1);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin[i - 1]),
		                   (gdouble)(badge->messages[i - 1].speed + 1));
		gtk_table_attach_defaults(GTK_TABLE(table), spin[i - 1],
		                          2, 3, i, i + 1);

		/* Combo box for actions */
		combo[i - 1] = gtk_combo_box_new_text();
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo[i - 1]),
		                          _("Move"));
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo[i - 1]),
		                          _("Flash, then Move"));
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo[i - 1]),
		                          _("Scroll Up"));
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo[i - 1]),
		                          _("Scroll Down"));
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo[i - 1]),
		                          _("Flash"));
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo[i - 1]),
		                          _("Freeze"));
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo[i - 1]),
		                         badge->messages[i - 1].action);
		gtk_table_attach_defaults(GTK_TABLE(table), combo[i - 1],
		                          3, 4, i, i + 1);
	}

	/* Now the hbox */
	progress = gtk_progress_bar_new();
	lum      = gtk_spin_button_new_with_range(1, 5, 1);
	button   = gtk_button_new_with_label(_("Send"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(lum),
	                          badge->luminance + 1);
	g_signal_connect(G_OBJECT(button), "clicked",
	                 G_CALLBACK(send_cb), NULL);

	/* Pack it. */
	gtk_box_pack_start(GTK_BOX(hbox), progress, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(_("Luminance:")),
	                   TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hbox), lum, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), button,TRUE,TRUE, 0);

	/* Add the table and the hbox to the vbox */
	gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

	/* Add the vbox to the window */
	gtk_container_add(GTK_CONTAINER(window), vbox);

	/* Show the window (and children) */
	gtk_widget_show_all(window);

	/* Start Gtk's main loop */
	gtk_main();

	/* Destroy the window and its children */
	for (i = 0; i < 6; i++) g_free(row_text[i]);
	if (bitmp[0]) bitmap_editor_free(bitmp[0]);
	if (bitmp[1]) bitmap_editor_free(bitmp[1]);
	badge_close();
	return EXIT_SUCCESS;

err:
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	exit(EXIT_FAILURE);
}

