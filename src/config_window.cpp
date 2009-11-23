/* config_window.cpp */
/* This code is part of the Public Domain. */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h> //this is needed due to retro
#include "retro_slider.h"
#include "alsa_classes.h"
#include "config_settings.h"
#include "config_window.h"


ConfigSettings tmp_settings;
ConfigSettings *orig_settings;
GtkWidget *window;

//load the current settings into a temporary tmp_settings variable
void load_settings(ConfigSettings *settings){
	tmp_settings.copy_settings(settings);
	orig_settings = settings;
}

//save the current settings back to the rc file and apply them
void save_settings(){
	orig_settings->copy_settings(&tmp_settings);
	orig_settings->write_config();
}

//close the window without saving anything
static void cancel_config_window(GtkWidget *widget, gpointer data){
	gtk_widget_destroy(window);
}

//close the window and save the settings
static void apply_config_window(GtkWidget *widget, gpointer data){
	save_settings();
	//set the restart flag and close all the windows
	orig_settings->restart = true;
	gtk_widget_destroy(window);
	if (orig_settings->enable_tray_icon){
		gtk_widget_destroy(orig_settings->slider_window);
		gtk_widget_destroy(orig_settings->tray_icon);
	}
	gtk_widget_destroy(orig_settings->main_window);
	//and quit gtk
	gtk_main_quit();
}

//return a pointer to a viewport in a scrolled window in a notebook tab
GtkWidget *tab_init(GtkWidget *notebook, const char *label_text){
	GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy((GtkScrolledWindow*)scrolled_window, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	GtkWidget *tab_label = gtk_label_new(label_text);
	gtk_notebook_append_page( (GtkNotebook*)notebook, scrolled_window, tab_label );
	GtkWidget *viewport = gtk_viewport_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled_window), viewport);
	return(viewport);
}

//update the value pointed to by the data pointer with the value contained by the widget
static void update_int(GtkWidget *widget, gpointer data){
	*((int *)data) = (int)gtk_adjustment_get_value(GTK_ADJUSTMENT(widget));
}

//create an entry to edit an int value w/ spinbutton
void add_entry_int(GtkWidget *vbox, const char *label_text, int *item){
	GtkWidget *hbox = gtk_hbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	GtkWidget *label = gtk_label_new(label_text);
	gtk_container_add(GTK_CONTAINER(hbox), label);
	GtkObject *adjustment = gtk_adjustment_new(*item, 0, 9999, 1, 10, 10);
	GtkWidget *spin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment), 1, 0);
	gtk_container_add(GTK_CONTAINER(hbox), spin);
	g_signal_connect(adjustment, "value-changed", G_CALLBACK(update_int), item);
}

//create a preferences window
void build_config_window(ConfigSettings *settings){
	load_settings(settings);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
	gtk_window_set_title(GTK_WINDOW(window), "Retrovol - Configuration");

	//create the overall vbox
	GtkWidget *over_box;
	over_box = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), over_box);

	//create the notebook
	GtkWidget *notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos((GtkNotebook*)notebook, GTK_POS_TOP);
	gtk_container_add(GTK_CONTAINER(over_box), notebook);

	//Slider tab
	//slider dimensions, colors, etc.
	{
		//initialize the tab
		GtkWidget *viewport = tab_init(notebook, "Sliders");
		GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
		gtk_container_add(GTK_CONTAINER(viewport), vbox);

		//add the widgets
		add_entry_int(vbox, "Width", &tmp_settings.slider_width);
		add_entry_int(vbox, "Height", &tmp_settings.slider_height);
		add_entry_int(vbox, "Margins", &tmp_settings.slider_margin);
		add_entry_int(vbox, "Segment Thickness", &tmp_settings.seg_thickness);
		add_entry_int(vbox, "Segment Spacing", &tmp_settings.seg_spacing);
	}

	//Hardware tab
	//default card, sliders to use, order, etc.
	{
		//initialize the tab
		GtkWidget *viewport = tab_init(notebook, "Hardware");
		GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
		gtk_container_add(GTK_CONTAINER(viewport), vbox);

		GtkWidget *label = gtk_label_new("Hardware stuff");
		gtk_container_add(GTK_CONTAINER(vbox), label);
	}

	//Main window tab
	//Main window dimensions, slider spacing, orientation, etc.
	{
		//initialize the tab
		GtkWidget *viewport = tab_init(notebook, "Window");
		GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
		gtk_container_add(GTK_CONTAINER(viewport), vbox);

		GtkWidget *label = gtk_label_new("Window stuff");
		gtk_container_add(GTK_CONTAINER(vbox), label);
	}



	//handle the apply and cancel buttons
	GtkWidget *apply_cancel_box = gtk_hbox_new(TRUE, 2);
	gtk_box_pack_end(GTK_BOX(over_box), apply_cancel_box, FALSE, TRUE, 0);
	GtkWidget *cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	GtkWidget *apply = gtk_button_new_from_stock(GTK_STOCK_APPLY);
	gtk_container_add(GTK_CONTAINER(apply_cancel_box), cancel);
	gtk_container_add(GTK_CONTAINER(apply_cancel_box), apply);
	g_signal_connect(cancel, "clicked", G_CALLBACK(cancel_config_window), NULL);
	g_signal_connect(apply, "clicked", G_CALLBACK(apply_config_window), NULL);

	gtk_widget_show_all(window);

}

