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


static ConfigSettings tmp_settings;
GtkWidget *window;

//load the current settings into a temporary tmp_settings variable
void load_settings(ConfigSettings *settings){
	tmp_settings.copy_settings(settings);
}

//save the current settings back to the rc file and apply them
void save_settings(ConfigSettings *settings){
	//tmp_settings.write();
	//settings.copy_settings(tmp_settings);
	//settings.apply_new();
}

//create a preferences window
void build_config_window(){

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(window), "Retrovol - Configuration");

	//use a scrolled window in a viewport
        GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy((GtkScrolledWindow*)scrolled_window, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(window), scrolled_window);
        GtkWidget *viewport = gtk_viewport_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled_window), viewport);

	//place the vbox
        GtkWidget *vbox = gtk_vbox_new(TRUE, 2);
	gtk_container_add(GTK_CONTAINER(viewport), vbox);

        GtkWidget *label1 = gtk_label_new("<< Unimplemented >>");
	gtk_container_add(GTK_CONTAINER(vbox), label1);

	gtk_widget_show_all(window);

}

