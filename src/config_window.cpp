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
SwapStruc slider_swap_struc;
SwapStruc tray_slider_swap_struc;

void SwapStruc::swap(){
	int tmp = *iA;
	*iA = *iB;
	*iB = tmp;
	gtk_adjustment_set_value(adj_A, *iA);
	gtk_adjustment_set_value(adj_B, *iB);
}
void SwapStruc::toggle(){
	*control = !*control;
	swap();
}
void SwapStruc::set(GtkToggleButton *button){
	if ((bool)gtk_toggle_button_get_active(button) != *control){
		toggle();
	}
}

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
	bool enable_tray_icon = orig_settings->enable_tray_icon;
	save_settings();
	//set the restart flag and close all the windows
	orig_settings->restart = true;
	gtk_widget_destroy(window);
	if (enable_tray_icon){
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
GtkAdjustment *add_entry_int(GtkWidget *vbox, const char *label_text, int *item){
	GtkWidget *hbox = gtk_hbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	GtkWidget *label = gtk_label_new(label_text);
	gtk_container_add(GTK_CONTAINER(hbox), label);
	GtkObject *adjustment = gtk_adjustment_new(*item, 0, 9999, 1, 10, 10);
	GtkWidget *spin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment), 1, 0);
	gtk_container_add(GTK_CONTAINER(hbox), spin);
	g_signal_connect(adjustment, "value-changed", G_CALLBACK(update_int), item);
	return(GTK_ADJUSTMENT(adjustment));
}

//update the value pointed to by the data pointer with the value contained by the widget
static void update_bool(GtkWidget *widget, gpointer data){
	*((bool *)data) = (bool)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

//update the bool and swap the ints pointed to by the data pointer with the value contained by the widget
static void update_bool_s(GtkWidget *widget, gpointer data){
	((SwapStruc*)data)->set(GTK_TOGGLE_BUTTON(widget));
}

//create an entry to edit a bool value w/ checkbox
void add_entry_bool_c(GtkWidget *vbox, const char *label_text, bool *item, SwapStruc *swap_struc){
	if (swap_struc != NULL){
		item = swap_struc->control;
	}
	GtkWidget *hbox = gtk_hbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	GtkWidget *label = gtk_label_new(label_text);
	gtk_container_add(GTK_CONTAINER(hbox), label);
	GtkWidget *check_button = gtk_check_button_new();
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button), *item);
	gtk_container_add(GTK_CONTAINER(hbox), check_button);
	if (swap_struc == NULL){
		g_signal_connect(check_button, "toggled", G_CALLBACK(update_bool), item);
	} else {
		g_signal_connect(check_button, "toggled", G_CALLBACK(update_bool_s), swap_struc);
	}
}

//create an entry to edit a bool value w/ radiobutton
void add_entry_bool_r(GtkWidget *vbox, const char *label_text, const char *true_label, const char *false_label, bool *item, SwapStruc *swap_struc){
	if (swap_struc != NULL){
		item = swap_struc->control;
	}
	GtkWidget *hbox = gtk_hbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	GtkWidget *label = gtk_label_new(label_text);
	gtk_container_add(GTK_CONTAINER(hbox), label);
	GtkWidget *radio_vbox = gtk_vbox_new(TRUE, 2);
	gtk_container_add(GTK_CONTAINER(hbox), radio_vbox);
	GtkWidget *true_button = gtk_radio_button_new_with_label(NULL, true_label);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(true_button), *item);
	gtk_container_add(GTK_CONTAINER(radio_vbox), true_button);
	GtkWidget *false_button = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(true_button), false_label);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(false_button), !*item);
	gtk_container_add(GTK_CONTAINER(radio_vbox), false_button);
	if (swap_struc == NULL){
		g_signal_connect(true_button, "toggled", G_CALLBACK(update_bool), item);
	} else {
		g_signal_connect(true_button, "toggled", G_CALLBACK(update_bool_s), swap_struc);
	}
}

//update the value pointed to by the data pointer with the value contained by the widget
static void update_color(GtkWidget *widget, gpointer data){
	GdkColor color;
	gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), &color);
	ConfigSettings::gtonf((float *)data, &color);
}

//create an entry to edit a color value
void add_entry_color(GtkWidget *vbox, const char *label_text, float *item){
	GdkColor color;
	ConfigSettings::nftog(item, &color);
	GtkWidget *hbox = gtk_hbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	GtkWidget *label = gtk_label_new(label_text);
	gtk_container_add(GTK_CONTAINER(hbox), label);
	GtkWidget *color_button = gtk_color_button_new_with_color(&color);
	gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(color_button), FALSE);
	gtk_container_add(GTK_CONTAINER(hbox), color_button);
	g_signal_connect(color_button, "color-set", G_CALLBACK(update_color), item);
}

//create a set of widgets that let you enable/disable and rearrange the visible sliders
void add_slider_config(GtkWidget *vbox, int *num_names, char name_list[80][80], ElementList *list_ptr){
	GtkWidget *label = gtk_label_new("Hardware stuff");
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, TRUE, 0);
	GtkWidget *hbox = gtk_hbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	GtkWidget *up_down_box = gtk_vbox_new(FALSE, 2);
	gtk_container_add(GTK_CONTAINER(hbox), up_down_box);
	GtkWidget *up_label = gtk_label_new("UP");
	gtk_container_add(GTK_CONTAINER(up_down_box), up_label);
	GtkWidget *down_label = gtk_label_new("DOWN");
	gtk_container_add(GTK_CONTAINER(up_down_box), down_label);
	GtkWidget *a_box = gtk_vbox_new(FALSE, 2);
	gtk_container_add(GTK_CONTAINER(hbox), a_box);
	GtkWidget *active_label = gtk_label_new("ACTIVE");
	gtk_container_add(GTK_CONTAINER(a_box), active_label);
	for (int i=0; i<*num_names; i++){
		GtkWidget *item_label = gtk_label_new(name_list[i]);
		gtk_container_add(GTK_CONTAINER(a_box), item_label);
	}
	GtkWidget *left_right_box = gtk_vbox_new(FALSE, 2);
	gtk_container_add(GTK_CONTAINER(hbox), left_right_box);
	GtkWidget *left_label = gtk_label_new("<<");
	gtk_container_add(GTK_CONTAINER(left_right_box), left_label);
	GtkWidget *right_label = gtk_label_new(">>");
	gtk_container_add(GTK_CONTAINER(left_right_box), right_label);
	GtkWidget *i_box = gtk_vbox_new(FALSE, 2);
	gtk_container_add(GTK_CONTAINER(hbox), i_box);
	GtkWidget *inactive_label = gtk_label_new("INACTIVE");
	gtk_container_add(GTK_CONTAINER(i_box), inactive_label);
	char unused_name_list[80][80]; //NEED TO MAKE THIS DYNAMIC!
	int num_unused_names = list_ptr->list_other_names(unused_name_list);
	for (int i=0; i<num_unused_names; i++){
		GtkWidget *item_label = gtk_label_new(unused_name_list[i]);
		gtk_container_add(GTK_CONTAINER(i_box), item_label);
	}

}

//create a preferences window
void build_config_window(ConfigSettings *settings){
	load_settings(settings);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), 300, 500);
	gtk_window_set_title(GTK_WINDOW(window), "Retrovol - Configuration");

	//create the overall vbox
	GtkWidget *over_box;
	over_box = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), over_box);

	//create the notebook
	GtkWidget *notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos((GtkNotebook*)notebook, GTK_POS_TOP);
	gtk_container_add(GTK_CONTAINER(over_box), notebook);

	//Main tab
	//dimensions, colors, etc.
	{
		//initialize the tab
		GtkWidget *viewport = tab_init(notebook, "Main");
		GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
		gtk_container_add(GTK_CONTAINER(viewport), vbox);

		//add the widgets
		add_entry_int(vbox, "Window Width", &tmp_settings.window_width);
		add_entry_int(vbox, "Window Height", &tmp_settings.window_height);
		slider_swap_struc.iA = &(tmp_settings.slider_width);
		slider_swap_struc.iB = &(tmp_settings.slider_height);
		slider_swap_struc.control = &tmp_settings.vertical;
		add_entry_bool_r(vbox, "Slider Orientation", "Vertical", "Horizontal", NULL, &slider_swap_struc);
		slider_swap_struc.adj_A = add_entry_int(vbox, "Slider Width", slider_swap_struc.iA);
		slider_swap_struc.adj_B = add_entry_int(vbox, "Slider Height", slider_swap_struc.iB);
		add_entry_int(vbox, "Slider Margins", &tmp_settings.slider_margin);
		add_entry_int(vbox, "Segment Thickness", &tmp_settings.seg_thickness);
		add_entry_int(vbox, "Segment Spacing", &tmp_settings.seg_spacing);
		add_entry_color(vbox, "Background Color", tmp_settings.background_color);
		add_entry_color(vbox, "Border Color", tmp_settings.border_color);
		add_entry_color(vbox, "Unlit Color", tmp_settings.unlit_color);
		add_entry_color(vbox, "Lit Color", tmp_settings.lit_color);
		add_entry_bool_c(vbox, "Enable Tray Icon", &tmp_settings.enable_tray_icon);
		tray_slider_swap_struc.iA = &(tmp_settings.tray_slider_width);
		tray_slider_swap_struc.iB = &(tmp_settings.tray_slider_height);
		tray_slider_swap_struc.control = &tmp_settings.tray_slider_vertical;
		add_entry_bool_r(vbox, "Tray Slider Orientation", "Vertical", "Horizontal", NULL, &tray_slider_swap_struc);
		tray_slider_swap_struc.adj_A = add_entry_int(vbox, "Tray Slider Width", tray_slider_swap_struc.iA);
		tray_slider_swap_struc.adj_B = add_entry_int(vbox, "Tray Slider Height", tray_slider_swap_struc.iB);
	}

	//Hardware tab
	//default card, sliders to use, order, etc.
	{
		//initialize the tab
		GtkWidget *viewport = tab_init(notebook, "Hardware");
		GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
		gtk_container_add(GTK_CONTAINER(viewport), vbox);

		//add the widgets
		add_slider_config(vbox, &tmp_settings.num_names, tmp_settings.name_list, tmp_settings.list_ptr);
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

