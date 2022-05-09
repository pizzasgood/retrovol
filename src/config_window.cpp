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

//i18n stuff
#include "gettext.h"
#include <locale.h>
#define _(String) gettext (String)


ConfigSettings tmp_settings;
ConfigSettings *orig_settings;
GtkWidget *window;
SwapStruc slider_swap_struc;
SwapStruc tray_slider_swap_struc;
OrderWidget order_widget;

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



GtkListStore *OrderWidget::build_list_from_names(int num_numids, const int numid_list[], const char name_list[][256]){
	GtkListStore *store = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);
	GtkTreeIter iter;
	for(int i=0; i<num_numids; i++){
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, 0, numid_list[i], 1, name_list[i], -1);
	}
	return(store);
}

//update the name_list to match the GtkListStore
int OrderWidget::update_names_from_list(int numid_list[], char name_list[][256]){
	GtkTreeIter iter;
	gboolean valid;
	int k = 0;
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(a_store), &iter);
	while (valid){
		gint int_data;
		gchar *str_data;
		gtk_tree_model_get(GTK_TREE_MODEL(a_store), &iter, 0, &int_data, 1, &str_data, -1);
		numid_list[k] = int_data;
		strcpy(name_list[k++], str_data);
		g_free(str_data);
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(a_store), &iter);
	}
	return(k);
}

//build the widgets
void OrderWidget::build(GtkContainer *parent_container, int *num_numids, int numid_list[], char name_list[][256], ElementList *list_ptr){
	//make a vbox to hold everything, and put it inside parent_container
	GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
	gtk_container_add(parent_container, vbox);
	GtkWidget *desc_label = gtk_label_new(_("Below you can chose which sliders to enable and in which order to display them."));
	gtk_box_pack_start(GTK_BOX(vbox), desc_label, FALSE, TRUE, 0);

	//make an hbox to hold the bulk of the widget, and put it inside the vbox
	hbox = gtk_hbox_new(FALSE, 2);
	gtk_container_add(GTK_CONTAINER(vbox), hbox);

	//make a vbox to hold the up/down buttons
	GtkWidget *up_down_box = gtk_vbox_new(FALSE, 2);
	gtk_container_add(GTK_CONTAINER(hbox), up_down_box);

	//make the up button
	GtkWidget *up_image = gtk_image_new_from_stock(GTK_STOCK_GO_UP, GTK_ICON_SIZE_BUTTON);
	GtkWidget *up_button = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(up_button), up_image);
	g_signal_connect(up_button, "clicked", G_CALLBACK(move_selected_up), this);
	gtk_box_pack_start(GTK_BOX(up_down_box), up_button, FALSE, TRUE, 0);

	//make the down button
	GtkWidget *down_image = gtk_image_new_from_stock(GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_BUTTON);
	GtkWidget *down_button = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(down_button), down_image);
	g_signal_connect(down_button, "clicked", G_CALLBACK(move_selected_down), this);
	gtk_box_pack_start(GTK_BOX(up_down_box), down_button, FALSE, TRUE, 0);

	//make a scrolled-window to hold the list
	GtkWidget *a_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(hbox), a_scrolled_window);

	//fill the active list
	a_store = build_list_from_names(*num_numids, numid_list, name_list);
	a_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(a_store));
	GtkCellRenderer *a_renderer;
	GtkTreeViewColumn *a_column;
	a_renderer = gtk_cell_renderer_text_new();
	a_column = gtk_tree_view_column_new_with_attributes(_("Active Sliders"), a_renderer, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(a_list), a_column);
	gtk_container_add(GTK_CONTAINER(a_scrolled_window), a_list);

	//make a vbox to hold the add/remove buttons
	GtkWidget *left_right_box = gtk_vbox_new(FALSE, 2);
	gtk_container_add(GTK_CONTAINER(hbox), left_right_box);

	//add the add button
	GtkWidget *left_image = gtk_image_new_from_stock(GTK_STOCK_GO_BACK, GTK_ICON_SIZE_BUTTON);
	GtkWidget *left_button = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(left_button), left_image);
	g_signal_connect(left_button, "clicked", G_CALLBACK(add_selected), this);
	gtk_box_pack_start(GTK_BOX(left_right_box), left_button, FALSE, TRUE, 0);
	
	//add the remove button
	GtkWidget *right_image = gtk_image_new_from_stock(GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_BUTTON);
	GtkWidget *right_button = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(right_button), right_image);
	g_signal_connect(right_button, "clicked", G_CALLBACK(remove_selected), this);
	gtk_box_pack_start(GTK_BOX(left_right_box), right_button, FALSE, TRUE, 0);

	//get the inactive items
	int unused_numid_list[256]; //NEED TO MAKE THIS DYNAMIC!
	char unused_name_list[256][256]; //NEED TO MAKE THIS DYNAMIC!
	int num_unused_numids;
	int num_unused_names;
	if (*num_numids == 0){
		num_unused_numids = list_ptr->list_all_numids(unused_numid_list);
		num_unused_names = list_ptr->list_all_names(unused_name_list);
	} else {
		num_unused_numids = list_ptr->list_other_numids(unused_numid_list);
		num_unused_names = list_ptr->list_other_names(unused_name_list);
	}

	//make a scrolled-window to hold the list
	GtkWidget *i_scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(hbox), i_scrolled_window);

	//fill the inactive list
	i_store = build_list_from_names(num_unused_numids, unused_numid_list, unused_name_list);
	i_list = gtk_tree_view_new_with_model(GTK_TREE_MODEL(i_store));
	GtkCellRenderer *i_renderer;
	GtkTreeViewColumn *i_column;
	i_renderer = gtk_cell_renderer_text_new();
	i_column = gtk_tree_view_column_new_with_attributes(_("Inactive Sliders"), i_renderer, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(i_list), i_column);
	gtk_container_add(GTK_CONTAINER(i_scrolled_window), i_list);

}

//move the selected item up in the list
void OrderWidget::move_selected_up(GtkWidget *widget, gpointer data){
	OrderWidget *order_widget = (OrderWidget *)data;
	GtkListStore *store = order_widget->a_store;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(order_widget->a_list));
	GtkTreeIter iter, position;
	if(gtk_tree_selection_get_selected(selection, NULL, &iter)){
		//since for some reason they don't have a gtk_tree_model_iter_prev() function, we'll convert to a path and then navigate backwards, then get an iter from that...
		GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &iter);
		if(gtk_tree_path_prev(path)){
			gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &position, path);
			gtk_list_store_move_before(store, &iter, &position);
		}
		gtk_tree_path_free(path);
	}
}

//move the selected item down in the list
void OrderWidget::move_selected_down(GtkWidget *widget, gpointer data){
	OrderWidget *order_widget = (OrderWidget *)data;
	GtkListStore *store = order_widget->a_store;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(order_widget->a_list));
	GtkTreeIter iter;
	if(gtk_tree_selection_get_selected(selection, NULL, &iter)){
		GtkTreeIter position = iter;
		if (gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &position)){
			gtk_list_store_move_after(store, &iter, &position);
		}

	}
}

//add the selected item to the list
void OrderWidget::add_selected(GtkWidget *widget, gpointer data){
	OrderWidget *order_widget = (OrderWidget *)data;
	GtkListStore *a_store = order_widget->a_store;
	GtkListStore *i_store = order_widget->i_store;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(order_widget->i_list));
	GtkTreeIter a_iter, i_iter;
	if(gtk_tree_selection_get_selected(selection, NULL, &i_iter)){
		gint int_data;
		gchar *str_data;
		gtk_tree_model_get(GTK_TREE_MODEL(i_store), &i_iter, 0, &int_data, 1, &str_data, -1);
		gtk_list_store_remove(i_store, &i_iter);
		gtk_list_store_append(a_store, &a_iter);
		gtk_list_store_set(a_store, &a_iter, 0, int_data, 1, str_data, -1);
		g_free(str_data);
	}
}

//remove the selected item from the list
void OrderWidget::remove_selected(GtkWidget *widget, gpointer data){
	OrderWidget *order_widget = (OrderWidget *)data;
	GtkListStore *a_store = order_widget->a_store;
	GtkListStore *i_store = order_widget->i_store;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(order_widget->a_list));
	GtkTreeIter a_iter, i_iter;
	if(gtk_tree_selection_get_selected(selection, NULL, &a_iter)){
		gint int_data;
		gchar *str_data;
		gtk_tree_model_get(GTK_TREE_MODEL(a_store), &a_iter, 0, &int_data, 1, &str_data, -1);
		gtk_list_store_remove(a_store, &a_iter);
		gtk_list_store_append(i_store, &i_iter);
		gtk_list_store_set(i_store, &i_iter, 0, int_data, 1, str_data, -1);
		g_free(str_data);
	}
}






//load the current settings into a temporary tmp_settings variable
void load_settings(ConfigSettings *settings){
	tmp_settings.copy_settings(settings);
	orig_settings = settings;
}

//save the current settings back to the rc file and apply them
void save_settings(){
	tmp_settings.num_numids = order_widget.update_names_from_list(tmp_settings.numid_list, tmp_settings.name_list);
	orig_settings->copy_settings(&tmp_settings);
	orig_settings->write_config();
}

//close the window without saving anything
static void cancel_config_window(GtkWidget *widget, gpointer data){
	gtk_widget_destroy(window);
}

//close the windows and save the settings
static void apply_config_window(GtkWidget *widget, gpointer data){
	bool enable_tray_icon = orig_settings->enable_tray_icon;
	save_settings();
	//set the restart flag and close all the windows
	orig_settings->restart = true;
	if (GTK_WIDGET_VISIBLE(orig_settings->main_window)){
		orig_settings->resume_main = true;
	}
	gtk_widget_destroy(window);
	if (enable_tray_icon){
		gtk_widget_destroy(orig_settings->slider_window);
#if GTK_CHECK_VERSION(2,16,0)
		gtk_status_icon_set_visible(orig_settings->tray_icon, false);
#else
		gtk_widget_destroy(orig_settings->tray_icon);
#endif
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

//create an entry to edit an unsigned int value w/ spinbutton
GtkAdjustment *add_entry_uint(GtkWidget *vbox, const char *label_text, int *item, unsigned int min=0, unsigned int max=9999){
	GtkWidget *hbox = gtk_hbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	GtkWidget *label = gtk_label_new(label_text);
	gtk_container_add(GTK_CONTAINER(hbox), label);
	GtkObject *adjustment = gtk_adjustment_new(*item, min, max, 1, 10, 0);
	GtkWidget *spin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment), 1, 0);
	gtk_container_add(GTK_CONTAINER(hbox), spin);
	g_signal_connect(adjustment, "value-changed", G_CALLBACK(update_int), item);
	return(GTK_ADJUSTMENT(adjustment));
}

//create an entry to edit an int value w/ spinbutton
GtkAdjustment *add_entry_int(GtkWidget *vbox, const char *label_text, int *item, int min=-9999, int max=9999){
	GtkWidget *hbox = gtk_hbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	GtkWidget *label = gtk_label_new(label_text);
	gtk_container_add(GTK_CONTAINER(hbox), label);
	GtkObject *adjustment = gtk_adjustment_new(*item, min, max, 1, 10, 0);
	GtkWidget *spin = gtk_spin_button_new(GTK_ADJUSTMENT(adjustment), 1, 0);
	gtk_container_add(GTK_CONTAINER(hbox), spin);
	g_signal_connect(adjustment, "value-changed", G_CALLBACK(update_int), item);
	return(GTK_ADJUSTMENT(adjustment));
}

//update the string pointed to by the data pointer with the value contained by the widget
static void update_txt_d(GtkWidget *widget, GtkDeleteType arg1, int arg2, gpointer data){
	update_txt(widget, data);
}
static void update_txt_i(GtkWidget *widget, char *arg1, int pos, char *arg2, gpointer data){
	update_txt(widget, data);
}
inline static void update_txt(GtkWidget *widget, gpointer data){
	strcpy((char*)data, gtk_entry_get_text(GTK_ENTRY(widget)));
}

//create an entry to edit a text value
GtkEntry *add_entry_txt(GtkWidget *vbox, const char *label_text, char *item, int len){
	GtkWidget *hbox = gtk_hbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	GtkWidget *label = gtk_label_new(label_text);
	gtk_container_add(GTK_CONTAINER(hbox), label);
	GtkWidget *entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), item);
	gtk_entry_set_max_length(GTK_ENTRY(entry), len);
	gtk_container_add(GTK_CONTAINER(hbox), entry);
	g_signal_connect_after(entry, "activate", G_CALLBACK(update_txt), item);
	g_signal_connect_after(entry, "cut-clipboard", G_CALLBACK(update_txt), item);
	g_signal_connect_after(entry, "paste-clipboard", G_CALLBACK(update_txt), item);
	g_signal_connect_after(entry, "backspace", G_CALLBACK(update_txt), item);
	g_signal_connect_after(entry, "delete-from-cursor", G_CALLBACK(update_txt_d), item);
	g_signal_connect_after(entry, "insert_text", G_CALLBACK(update_txt_i), item);
	return(GTK_ENTRY(entry));
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

//update the int pointed to by the data pointer with the value contained by the combo box widget
static void update_int_combo(GtkWidget *widget, gpointer data){
	*(int*)data = (int)gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
}

//update the string pointed to by the data pointer with the string contained by the widget
static void update_slider(GtkWidget *widget, gpointer data){
	gchar *text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(widget));
	*(int*)data = atoi(text);
	g_free(text);
}

//create an entry to choose a volume scale with a dropdown
void add_entry_scaling_dropdown(GtkWidget *vbox, const char *label_text, int *scaling){
	GtkWidget *hbox = gtk_hbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	GtkWidget *label = gtk_label_new(label_text);
	gtk_container_add(GTK_CONTAINER(hbox), label);
	GtkWidget *combo = gtk_combo_box_new_text();
	//get the text
	char *name_list[NUM_SCALE_T];
	name_list[Element::LINEAR] = _("Linear");
	name_list[Element::LOGARITHMIC] = _("Logarithmic");
	name_list[Element::EXPONENTIAL] = _("Exponential");
	for(int i=0; i<NUM_SCALE_T; i++){
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo), name_list[i]);
		if (i == *scaling){
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo), i);
		}
	}
	gtk_container_add(GTK_CONTAINER(hbox), combo);
	g_signal_connect(combo, "changed", G_CALLBACK(update_int_combo), scaling);
}

//create an entry to choose a slider with a dropdown
void add_entry_slider_dropdown(GtkWidget *vbox, const char *label_text, int *tray_control_numid, ElementList *list_ptr){
	GtkWidget *hbox = gtk_hbox_new(TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
	GtkWidget *label = gtk_label_new(label_text);
	gtk_container_add(GTK_CONTAINER(hbox), label);
	GtkWidget *combo = gtk_combo_box_new_text();
	//get the text
	int numid_list[256]; //NEED TO MAKE THIS DYNAMIC!
	char name_list[256][256]; //NEED TO MAKE THIS DYNAMIC!
	char tmpstring[256]; //NEED TO MAKE THIS DYNAMIC!
	int num_numids = list_ptr->list_all_int_numids(numid_list);
	list_ptr->list_all_int_names(name_list);
	for(int i=0; i<num_numids; i++){
		sprintf(tmpstring, "%d:%s", numid_list[i], name_list[i]);
		gtk_combo_box_append_text(GTK_COMBO_BOX(combo), tmpstring);
		if (numid_list[i] == *tray_control_numid){
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo), i);
		}
	}
	gtk_container_add(GTK_CONTAINER(hbox), combo);
	g_signal_connect(combo, "changed", G_CALLBACK(update_slider), tray_control_numid);
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

//create a preferences window
void build_config_window(ConfigSettings *settings){
	load_settings(settings);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), 550, 300);
	gtk_window_set_title(GTK_WINDOW(window), _("Retrovol - Configuration"));

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
		GtkWidget *viewport = tab_init(notebook, _("Main"));
		GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
		gtk_container_add(GTK_CONTAINER(viewport), vbox);

		//add the widgets
		slider_swap_struc.iA = &(tmp_settings.slider_width);
		slider_swap_struc.iB = &(tmp_settings.slider_height);
		slider_swap_struc.control = &tmp_settings.vertical;
		add_entry_scaling_dropdown(vbox, _("Volume Scaling"), &tmp_settings.scaling);
		add_entry_bool_c(vbox, _("Auto Mute"), &tmp_settings.auto_mute);
		add_entry_bool_r(vbox, _("Slider Orientation"), _("Vertical"), _("Horizontal"), NULL, &slider_swap_struc);
		slider_swap_struc.adj_A = add_entry_uint(vbox, _("Slider Width"), slider_swap_struc.iA);
		slider_swap_struc.adj_B = add_entry_uint(vbox, _("Slider Height"), slider_swap_struc.iB);
		add_entry_uint(vbox, _("Slider Margins"), &tmp_settings.slider_margin);
		add_entry_uint(vbox, _("Segment Thickness"), &tmp_settings.seg_thickness, 1);
		add_entry_uint(vbox, _("Segment Spacing"), &tmp_settings.seg_spacing);
		add_entry_color(vbox, _("Background Color"), tmp_settings.background_color);
		add_entry_color(vbox, _("Border Color"), tmp_settings.border_color);
		add_entry_color(vbox, _("Unlit Color"), tmp_settings.unlit_color);
		add_entry_color(vbox, _("Lit Color"), tmp_settings.lit_color);
	}

	//Tray tab
	//dimensions, colors, etc.
	{
		//initialize the tab
		GtkWidget *viewport = tab_init(notebook, _("Tray"));
		GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
		gtk_container_add(GTK_CONTAINER(viewport), vbox);

		//add the widgets
		add_entry_bool_c(vbox, _("Enable Tray Icon"), &tmp_settings.enable_tray_icon);
		add_entry_bool_c(vbox, _("Enable Tray Menu"), &tmp_settings.enable_tray_menu);
		add_entry_slider_dropdown(vbox, _("Tray Slider"), &tmp_settings.tray_control_numid, tmp_settings.list_ptr);
		tray_slider_swap_struc.iA = &(tmp_settings.tray_slider_width);
		tray_slider_swap_struc.iB = &(tmp_settings.tray_slider_height);
		tray_slider_swap_struc.control = &tmp_settings.tray_slider_vertical;
		add_entry_bool_r(vbox, _("Tray Slider Orientation"), _("Vertical"), _("Horizontal"), NULL, &tray_slider_swap_struc);
		tray_slider_swap_struc.adj_A = add_entry_uint(vbox, _("Tray Slider Width"), tray_slider_swap_struc.iA);
		tray_slider_swap_struc.adj_B = add_entry_uint(vbox, _("Tray Slider Height"), tray_slider_swap_struc.iB);
		add_entry_int(vbox, _("Tray Slider Offset"), &tmp_settings.tray_slider_offset);
		add_entry_bool_c(vbox, _("Enable Tray Icon Background Color"), &tmp_settings.enable_tray_icon_background_color);
		add_entry_color(vbox, _("Tray Icon Background Color"), tmp_settings.tray_icon_background_color);
	}

	//Hardware tab
	//default card, sliders to use, order, etc.
	{
		//initialize the tab
		GtkWidget *viewport = tab_init(notebook, _("Hardware"));
		GtkWidget *vbox = gtk_vbox_new(FALSE, 2);
		gtk_container_add(GTK_CONTAINER(viewport), vbox);

		//add the widgets
		add_entry_txt(vbox, _("Sound Card"), tmp_settings.card, 15);
		order_widget.build(GTK_CONTAINER(vbox), &tmp_settings.num_numids, tmp_settings.numid_list, tmp_settings.name_list, tmp_settings.list_ptr);
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

