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

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixdata.h>
#include <glib/gi18n.h>

#include "icon.h"
#include "badge.h"
#include "bitmap_editor.h"

GtkWidget *window;
static badge_t *badge;
static GtkWidget *text[4];
static GtkWidget *spin[6];
static GtkWidget *combo[6];
static GtkWidget *progress;
static GtkWidget *lum;
static bitmap_editor_t *bitmp[2];
gchar *row_text[6];

/**
 * Called when the user clicks the "Send" button. 
 */
static void send_cb(GtkWidget *widget, gpointer data) {
	int i; gchar *tmp;

	/* Read new data from the UI */
	badge->luminance = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(lum)) - 1;

	for (i=0;i<6;i++) {
		badge->messages[i].speed  = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin[i])) - 1;
		badge->messages[i].action = gtk_combo_box_get_active(GTK_COMBO_BOX(combo[i]));

		if (i < 4) { /* Text (ASCII or ISO-8859-1 according to the docs) */
			tmp = g_convert(gtk_entry_get_text(GTK_ENTRY(text[i])),-1,"ISO-8859-1","UTF-8",NULL,NULL,NULL);
			free(badge->messages[i].data);
			badge->messages[i].data   = strdup(tmp);
			badge->messages[i].length = strlen(tmp);
			g_free(tmp);
		} else badge->messages[i].length = bitmp[i-4]->length; 
		gtk_progress_bar_update(GTK_PROGRESS_BAR(progress),(gdouble)((i+1)/8));
	}

	/* Send it to the device */
	badge_set_data(badge);
	gtk_progress_bar_update(GTK_PROGRESS_BAR(progress),1.0);
}

/**
 * Called when the user requests that the window be closed.
 */
static gboolean window_closed(GtkWidget *widget, GdkEvent *event, gpointer data) {
	gtk_main_quit();
	return FALSE;
}

int main(int argc, char **argv) {
	GtkWidget *vbox, *table, *hbox, *button; GdkPixbuf *pb;
	int i;

	/* Allocate a new badge structure, and open the device */
	if (!(badge = badge_new())) {
		printf("Unable to allocate badge structure, or open device!\nPlease ensure you have the proper permissions.\n");
		exit(1);
	} bitmp[0] = bitmp[1] = NULL; memset(row_text,0,sizeof(row_text));

	/* Load data from the badge */
	badge_get_data(badge);

	/* Initialize GTK */
	gtk_init(&argc,&argv);

	/* Create the window, and the "closed" handler */
	pb     = gdk_pixbuf_from_pixdata((GdkPixdata *)&WINDOW_ICON,FALSE,NULL);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window),_("USB LED Badge"));
	gtk_window_set_icon(GTK_WINDOW(window),pb); 
	gtk_window_set_default_size(GTK_WINDOW(window),415,275);
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	g_signal_connect(G_OBJECT(window),"delete_event",G_CALLBACK(window_closed),NULL);
	g_object_unref(pb); 

	/* Create the layout. This layout is loosely based on the manufacturer's supplied program. */
	vbox  = gtk_vbox_new(FALSE,0);
	hbox  = gtk_hbox_new(FALSE,0);
	table = gtk_table_new(7,4,FALSE);

	/* Table Headers */
	gtk_table_attach_defaults(GTK_TABLE(table),gtk_label_new(_("#")),0,1,0,1);
	gtk_table_attach_defaults(GTK_TABLE(table),gtk_label_new(_("Message")),1,2,0,1);
	gtk_table_attach_defaults(GTK_TABLE(table),gtk_label_new(_("Speed")),2,3,0,1);
	gtk_table_attach_defaults(GTK_TABLE(table),gtk_label_new(_("Action")),3,4,0,1);

	/* Rows */
	for (i=1;i<7;i++) {
		row_text[i-1] = g_strdup_printf("%d",i);
		gtk_table_attach_defaults(GTK_TABLE(table),gtk_label_new(row_text[i-1]),0,1,i,i+1);
		
		if (i < 5) {
			/* Text entry */
			text[i-1] = gtk_entry_new_with_max_length(136);
			gtk_entry_set_text(GTK_ENTRY(text[i-1]),badge->messages[i-1].data);
			gtk_table_attach_defaults(GTK_TABLE(table),text[i-1],1,2,i,i+1);
		} else { 
			/* Bitmap editors */
			bitmp[i-5] = bitmap_editor_new((unsigned char **)&badge->messages[i-1].data,badge->messages[i-1].length);
			gtk_table_attach_defaults(GTK_TABLE(table),bitmp[i-5]->evbox_small,1,2,i,i+1);
		}

		/* Spin button for speed */
		spin[i-1] = gtk_spin_button_new_with_range(1,8,1);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin[i-1]),(gdouble)(badge->messages[i-1].speed+1));
		gtk_table_attach_defaults(GTK_TABLE(table),spin[i-1],2,3,i,i+1);

		/* Combo box for actions */
		combo[i-1] = gtk_combo_box_new_text();
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo[i-1]),_("Move"));
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo[i-1]),_("Flash, then Move"));
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo[i-1]),_("Scroll Up"));
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo[i-1]),_("Scroll Down"));
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo[i-1]),_("Flash"));
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo[i-1]),_("Freeze"));
		gtk_combo_box_set_active(GTK_COMBO_BOX(combo[i-1]),badge->messages[i-1].action);
		gtk_table_attach_defaults(GTK_TABLE(table),combo[i-1],3,4,i,i+1);
	}

	/* Now the hbox */
	progress = gtk_progress_bar_new();
	lum      = gtk_spin_button_new_with_range(1,5,1);
	button   = gtk_button_new_with_label(_("Send"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(lum),(gdouble)badge->luminance + 1);
	g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(send_cb),NULL);

	/* Pack it. */
	gtk_box_pack_start(GTK_BOX(hbox),progress,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hbox),gtk_label_new(_("Luminance:")),TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hbox),lum,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(hbox),button,TRUE,TRUE,0);
	
	/* Add the table and the hbox to the vbox */
	gtk_box_pack_start(GTK_BOX(vbox),table,TRUE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,TRUE,TRUE,0);

	/* Add the vbox to the window */
	gtk_container_add(GTK_CONTAINER(window),vbox);

	/* Show the window (and children) */
	gtk_widget_show_all(window);

	/* Start Gtk's main loop */
	gtk_main();

	/* Destroy the window and its children */
	for (i=0;i<6;i++) g_free(row_text[i]);
	if (badge)        badge_free(badge);
	if (bitmp[0])     bitmap_editor_free(bitmp[0]);
	if (bitmp[1])     bitmap_editor_free(bitmp[1]);
	return 0;
}

/* vim: set ts=4: */
