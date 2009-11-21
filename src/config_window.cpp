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
	//tmp_settings.write();
	orig_settings->copy_settings(&tmp_settings);
	//settings.apply_new();
}

//close the window without saving anything
static void cancel_config_window(GtkWidget *widget, gpointer data){
	gtk_widget_destroy(window);
}

//close the window and save the settings
static void apply_config_window(GtkWidget *widget, gpointer data){
	save_settings();
	gtk_widget_destroy(window);
}

//update the value pointed to by the data pointer with the value contained by the widget
static void update_int(GtkWidget *widget, gpointer data){
	*((int *)data) = (int)gtk_adjustment_get_value(GTK_ADJUSTMENT(widget));
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
		//use a scrolled window in a viewport
		GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy((GtkScrolledWindow*)scrolled_window, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		GtkWidget *tab_label = gtk_label_new("Sliders");
		gtk_notebook_append_page( (GtkNotebook*)notebook, scrolled_window, tab_label );
		GtkWidget *viewport = gtk_viewport_new(NULL, NULL);
		gtk_container_add(GTK_CONTAINER(scrolled_window), viewport);

		GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
		gtk_container_add(GTK_CONTAINER(viewport), vbox);

		GtkWidget *hbox1 = gtk_hbox_new(TRUE, 2);
		gtk_box_pack_start(GTK_BOX(vbox), hbox1, FALSE, TRUE, 0);
		GtkWidget *label1 = gtk_label_new("Width");
		gtk_container_add(GTK_CONTAINER(hbox1), label1);
		GtkObject *adjustment1 = gtk_adjustment_new(tmp_settings.slider_width, 1, 9999, 1, 10, 10);
		GtkWidget *spin1 = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment1), 1, 0);
		gtk_container_add(GTK_CONTAINER(hbox1), spin1);
		g_signal_connect(adjustment1, "value-changed", G_CALLBACK(update_int), &tmp_settings.slider_width);

		GtkWidget *hbox2 = gtk_hbox_new(TRUE, 2);
		gtk_box_pack_start(GTK_BOX(vbox), hbox2, FALSE, TRUE, 0);
		GtkWidget *label2 = gtk_label_new("Height");
		gtk_container_add(GTK_CONTAINER(hbox2), label2);
		GtkObject *adjustment2 = gtk_adjustment_new(tmp_settings.slider_height, 1, 9999, 1, 10, 10);
		GtkWidget *spin2 = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment2), 1, 0);
		gtk_container_add(GTK_CONTAINER(hbox2), spin2);
		g_signal_connect(adjustment2, "value-changed", G_CALLBACK(update_int), &tmp_settings.slider_height);

		GtkWidget *hbox3 = gtk_hbox_new(TRUE, 2);
		gtk_box_pack_start(GTK_BOX(vbox), hbox3, FALSE, TRUE, 0);
		GtkWidget *label3 = gtk_label_new("Margins");
		gtk_container_add(GTK_CONTAINER(hbox3), label3);
		GtkObject *adjustment3 = gtk_adjustment_new(tmp_settings.slider_margin, 1, 9999, 1, 10, 10);
		GtkWidget *spin3 = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment3), 1, 0);
		gtk_container_add(GTK_CONTAINER(hbox3), spin3);
		g_signal_connect(adjustment3, "value-changed", G_CALLBACK(update_int), &tmp_settings.slider_margin);

	}

	//Hardware tab
	//default card, sliders to use, order, etc.
	{
		//use a scrolled window in a viewport
		GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy((GtkScrolledWindow*)scrolled_window, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		GtkWidget *tab_label = gtk_label_new("Hardware");
		gtk_notebook_append_page( (GtkNotebook*)notebook, scrolled_window, tab_label );
		GtkWidget *viewport = gtk_viewport_new(NULL, NULL);
		gtk_container_add(GTK_CONTAINER(scrolled_window), viewport);

		GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
		gtk_container_add(GTK_CONTAINER(viewport), vbox);
		GtkWidget *label = gtk_label_new("Hardware stuff");
		gtk_container_add(GTK_CONTAINER(vbox), label);
	}

	//Main window tab
	//Main window dimensions, slider spacing, orientation, etc.
	{
		//use a scrolled window in a viewport
		GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_set_policy((GtkScrolledWindow*)scrolled_window, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
		GtkWidget *tab_label = gtk_label_new("Window");
		gtk_notebook_append_page( (GtkNotebook*)notebook, scrolled_window, tab_label );
		GtkWidget *viewport = gtk_viewport_new(NULL, NULL);
		gtk_container_add(GTK_CONTAINER(scrolled_window), viewport);
		
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

