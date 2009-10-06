/* main.cpp */
/* This code is part of the Public Domain. */

/*
    retrovol - a retro-styled volume mixer, by Pizzasgood
*/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include "retro_slider.h"
#include "alsa_classes.h"
#include "config_settings.h"
#include "main.h"
#include "eggtrayicon.h"



static ConfigSetttings settings;
//add the leading slash here, so that it can simply be concatenated with the results of getenv("HOME") later.
const char config_file[] = "/.retrovolrc";
static ElementList *list_ptr;



//callback that handles changing an enumerated control
void change_combo_box(GtkWidget *combo_box, Element *elem){
	int state = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_box));
	elem->set((int)state);
}

//callback that handles muting/unmuting a control
void toggle_checkbox(GtkWidget *chkbx, Element *elem){
	bool state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkbx));
	elem->set((int)state);
}

void refresh_checkbox(GtkWidget *chkbx, GdkEventExpose *event, Element *elem){
	bool state = (bool)elem->get();
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkbx), state);
}


//callback that handles clicking the tray icon
gboolean tray_button_press_event_callback (GtkWidget *widget, GdkEventButton *event, GtkWidget *slider_window){
	
	switch(event->button){
		case 1:		//left mouse button - toggle slider_window
			if (GTK_WIDGET_VISIBLE(slider_window)){
				gtk_widget_hide_all(slider_window);
			} else {
				int x, y;
				gtk_window_get_size(GTK_WINDOW(slider_window), &x, &y);
				gtk_widget_set_uposition(slider_window, event->x_root - event->x - x/2 + widget->allocation.width/2, event->y_root-event->y-y-3);
				gtk_widget_show_all(slider_window);
			}
			break;
		case 3:		//right mouse button - display main window
			if (GTK_WIDGET_VISIBLE(settings.main_window)){
				gtk_widget_hide_all(settings.main_window);
			} else {
				gtk_widget_show_all(settings.main_window);
			}
			break;
		case 2:		//middle mouse button - mute
			if (settings.tray_control->switch_id >= 0){
				bool state = !(bool)list_ptr->elems[settings.tray_control->switch_id].get();
				list_ptr->elems[settings.tray_control->switch_id].set((int)(state));
			}
			break;
		default:
			break;
	}
	
	update(NULL);
	return(true);
}

//update the tray-icon and refresh the window
gboolean update(gpointer data){
	bool state = true;
	if (settings.enable_tray_icon){
		int val = settings.tray_control->get();
		char tooltiptext[16];
		if (settings.tray_control->switch_id >= 0){
			state = (bool)list_ptr->elems[settings.tray_control->switch_id].get();
		}
		if (state && val != 0){
			int image = 1+3*val/100;
			if (image > 3){
				image=3;
			} else if (image < 0){
				image=0;
			}
			sprintf(tooltiptext, "Volume: %d%%", val);
			gtk_image_set_from_file(GTK_IMAGE(settings.tray_icon_image), settings.icon_file_names[image]);
		} else {
			sprintf(tooltiptext, "Volume: Muted");
			gtk_image_set_from_file(GTK_IMAGE(settings.tray_icon_image), settings.icon_file_names[0]);
		}
		gtk_widget_set_tooltip_text(settings.tray_icon_image, tooltiptext);
	}
	if (GTK_WIDGET_VISIBLE(settings.main_window)){
		gtk_widget_queue_draw(settings.main_window);
	}
	
	return(true);
}


//replace the second space (or the first if preceeding a '(' or '-') with '\n'
//else append a '\n' to keep things lined up
void word_wrap(char *wrapped, char *orig){
	strcpy(wrapped, orig);
	unsigned int i,n;
	for (i=0, n=0; i<strlen(wrapped); i++){
		if (wrapped[i] == ' '){
			n++;
			if (n==2 || (n==1 && (wrapped[i+1] == '(' || wrapped[i+1] == '-'))){
				n=2;
				wrapped[i]='\n';
				while(wrapped[i+1] == '-' || wrapped[i+1] == ' '){
					strcpy(&wrapped[i+1], &wrapped[i+2]);
				}
				break;
			}
		}
	}
	if (n<2){
		wrapped[i]='\n';
		wrapped[i+1]='\0';
	}
}





int main(int argc, char** argv) {
	//parse the config file
	settings.parse_config(strcat(getenv("HOME"), config_file));
	//load the controls into list
	ElementList list(settings.card);
	list_ptr = &list;
	//reorder the controls to the order specified in the config file
	settings.reorder_list(&list);
	
	//set up the window
	gtk_init(&argc, &argv);
	settings.main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(settings.main_window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(settings.main_window), settings.window_width, settings.window_height);
	gtk_window_set_title(GTK_WINDOW(settings.main_window), "Retrovol");
	
	//if the tray icon is enabled, we want the window to hide rather than closing
	if (settings.enable_tray_icon){
		g_signal_connect(settings.main_window, "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);
	}

	//use a scrolled window
	GtkWidget *scrolled_window;
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy((GtkScrolledWindow*)scrolled_window, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(settings.main_window), scrolled_window);
	
	//put the stuff into a viewport manually, so we can specify that it should have no shadow
	GtkWidget *viewport;
	viewport = gtk_viewport_new(NULL, NULL);
	gtk_viewport_set_shadow_type(GTK_VIEWPORT(viewport), GTK_SHADOW_NONE);
	gtk_container_add(GTK_CONTAINER(scrolled_window), viewport);
	
			
	//and create an Hbox to hold all the stuff
	GtkWidget *hbox;
	if (settings.vertical){
		hbox = gtk_hbox_new(TRUE, 2);
		gtk_container_add(GTK_CONTAINER(viewport), hbox);
	} else {
		hbox = gtk_vbox_new(TRUE, 2);
		gtk_container_add(GTK_CONTAINER(viewport), hbox);
	}
			
	//add the sliders
	retro_slider *sliders = new retro_slider[list.num_items];
	
	for(int i=0; i<list.num_items; i++){
		//use a vbox w/ slider on top and label on bottom
		GtkWidget *vbox;
		if (settings.vertical){
			vbox = gtk_vbox_new(FALSE, 2);
		} else {
			vbox = gtk_hbox_new(FALSE, 2);
		}
		gtk_box_pack_start(GTK_BOX(hbox), vbox, false, false, 0);
		
		if (strcmp(list.items[i]->type, "INTEGER") == 0){
			//integers need sliders
			//the rslider pseudo-widget likes to be inside a container, lets use a GtkAlignment
			GtkWidget *frame;
			if (settings.vertical){
				frame = gtk_alignment_new(0.5,0.0,0,0);
				gtk_box_pack_start(GTK_BOX(vbox), frame, false, false, 0);
			} else {
				frame = gtk_alignment_new(0.0,0.5,0,0);
				gtk_box_pack_end(GTK_BOX(vbox), frame, false, false, 0);
			}
			//make the slider and associate with a control
			settings.apply_to_slider(&sliders[i]);
			sliders[i].init(frame, (void*)list.items[i], &Element::get_callback, &Element::set_callback);
		
		} else if (strcmp(list.items[i]->type, "BOOLEAN") == 0){
			//booleans need checkboxes
			GtkWidget *alignment;
			if (settings.vertical){
				alignment = gtk_alignment_new(0.5,1.0,0,0);
				gtk_box_pack_start(GTK_BOX(vbox), alignment, true, true, 0);
			} else {
				alignment = gtk_alignment_new(1.0,0.5,0,0);
				gtk_box_pack_end(GTK_BOX(vbox), alignment, true, true, 0);
			}
			GtkWidget *chkbx;
			chkbx = gtk_check_button_new();
			//set it to the current state
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkbx), (bool)list.items[i]->get());
			//bind to the toggle_checkbox function
			Element* ptr = list.items[i];
			g_signal_connect(GTK_TOGGLE_BUTTON(chkbx), "toggled", G_CALLBACK (toggle_checkbox), ptr);
			g_signal_connect_after(GTK_TOGGLE_BUTTON(chkbx), "expose-event", G_CALLBACK (refresh_checkbox), ptr);
			gtk_container_add(GTK_CONTAINER(alignment), chkbx);
		} else if (strcmp(list.items[i]->type, "ENUMERATED") == 0){
			GtkWidget *alignment;
			if (settings.vertical){
				alignment = gtk_alignment_new(0.5,0.5,0,0);
				gtk_box_pack_start(GTK_BOX(vbox), alignment, true, true, 0);
			} else {
				alignment = gtk_alignment_new(1.0,0.5,0,0);
				gtk_box_pack_end(GTK_BOX(vbox), alignment, true, true, 0);
			}
			//insert a combobox with the different options
			GtkWidget *combo_box;
			combo_box=gtk_combo_box_new_text();
			for(unsigned int n=0; n<list.items[i]->number_of_enums; n++){
				gtk_combo_box_append_text(GTK_COMBO_BOX(combo_box), list.items[i]->enums[n]);
			}
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box), list.items[i]->get());
			//bind to the change_combo_box function
			g_signal_connect(GTK_COMBO_BOX(combo_box), "changed", G_CALLBACK (change_combo_box), list.items[i]);
			gtk_container_add(GTK_CONTAINER(alignment), combo_box);
		}
		
		//add a checkbox for sliders that are muteable
		if (list.items[i]->switch_id >= 0){
			GtkWidget *alignment;
			if (settings.vertical){
				alignment = gtk_alignment_new(0.5,1.0,0,0);
				gtk_box_pack_start(GTK_BOX(vbox), alignment, true, true, 0);
			} else {
				alignment = gtk_alignment_new(1.0,0.5,0,0);
				gtk_box_pack_end(GTK_BOX(vbox), alignment, true, true, 0);
			}
			GtkWidget *chkbx;
			chkbx = gtk_check_button_new();
			//set it to the current state
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkbx), (bool)list.elems[list.items[i]->switch_id].get());
			//bind to the toggle_checkbox function
			g_signal_connect(GTK_TOGGLE_BUTTON(chkbx), "toggled", G_CALLBACK (toggle_checkbox), &(list.elems[list.items[i]->switch_id]));
			g_signal_connect_after(GTK_TOGGLE_BUTTON(chkbx), "expose-event", G_CALLBACK (refresh_checkbox), &(list.elems[list.items[i]->switch_id]));
			
			gtk_container_add(GTK_CONTAINER(alignment), chkbx);
		}
		
		//display the name of the control
		GtkWidget *alignment;
		char wrapped[80];
		if (settings.vertical){
			alignment = gtk_alignment_new(0.5,1.0,0,0);
			gtk_box_pack_end(GTK_BOX(vbox), alignment, false, false, 0);
			word_wrap(wrapped, list.items[i]->short_name);
		} else {
			alignment = gtk_alignment_new(1.0,0.5,0,0);
			gtk_box_pack_start(GTK_BOX(vbox), alignment, false, false, 0);
			strcpy(wrapped, list.items[i]->short_name);
		}
		GtkWidget *label;
		label = gtk_label_new(wrapped);
		gtk_container_add(GTK_CONTAINER(alignment), label);
	}
	
	//finish the window stuff
	gtk_widget_show_all(settings.main_window);
	g_signal_connect(settings.main_window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
	

	//set up the tray_slider that goes in the tray
	if (settings.enable_tray_icon){
		GtkWidget *tray_frame;
		tray_frame = gtk_alignment_new(0.5,0.0,0,0);
		settings.tray_slider = new retro_slider;
		settings.set_tray_slider(&list);
		settings.apply_to_tray_slider(settings.tray_slider);
		settings.tray_slider->init(tray_frame, (void*)settings.tray_control, &Element::get_callback, &Element::set_callback);

		//set up the small window that holds the tray_slider
		GtkWidget *slider_window;
		slider_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_window_set_resizable(GTK_WINDOW(slider_window), false);
		gtk_window_set_decorated(GTK_WINDOW(slider_window), false);
		gtk_window_set_skip_taskbar_hint(GTK_WINDOW(slider_window), true);
		gtk_window_set_skip_pager_hint(GTK_WINDOW(slider_window), true);
		gtk_widget_set_usize(slider_window, settings.tray_slider->width, settings.tray_slider->height);
		//don't want accidental closure of the slider window to destroy the window
		g_signal_connect(slider_window, "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);
		//want the widow to go away when it loses focus
		g_signal_connect(slider_window, "focus-out-event", G_CALLBACK (gtk_widget_hide), NULL);
		gtk_container_add( GTK_CONTAINER(slider_window), tray_frame );
		//we want it hidden by default, but it must be shown at least once or else scrolling over the icon will cause a hang
		gtk_widget_show_all(slider_window);
		gtk_widget_hide_all(slider_window);
		
			
		//set up tray icon
		GtkWidget *tray_icon;
		tray_icon = GTK_WIDGET(egg_tray_icon_new("Retrovol Tray Icon"));
		settings.tray_icon_image = gtk_image_new();
		gtk_container_add( GTK_CONTAINER(tray_icon), settings.tray_icon_image );
		gtk_image_set_from_file(GTK_IMAGE(settings.tray_icon_image), VOL_MEDIUM_IMAGE);
		g_signal_connect(G_OBJECT(tray_icon), "button_press_event", G_CALLBACK (&tray_button_press_event_callback), slider_window);
		g_signal_connect(G_OBJECT(tray_icon), "scroll_event", G_CALLBACK (&retro_slider::scroll_event_callback), settings.tray_slider);
		gtk_widget_set_events (tray_icon, GDK_BUTTON_PRESS_MASK | GDK_SCROLL_MASK);
		
		gtk_widget_show_all(tray_icon);
	}
	
	//add some periodic refreshment to keep the icon and window up-to-date
	g_timeout_add_seconds(1, update, NULL);
	
	//finished with gtk setup
	gtk_main();
	
	return(0);
}


