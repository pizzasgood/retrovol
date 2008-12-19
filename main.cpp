/* main.cpp */

/*
    retrovol - a retro-styled volume mixer, by Pizzasgood
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include "retro_slider.h"
#include "alsa_classes.h"
#include "config_settings.h"
#include "main.h"



static ConfigSetttings settings;
char config_file[] = "/root/.retrovolrc"; //CHANGE this to use the home-dir!


//callback that handles muting/unmuting a control
void toggle_it(GtkWidget *chkbx, Element *elem){
	bool state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkbx));
	elem->set((int)state);
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
	settings.parse_config(config_file);
	//load the controls into list
	ElementList list(settings.card);
	//reorder the controls to the order specified in the config file
	settings.reorder_list(&list);
	
	//set up the window
	GtkWidget *window;
	gtk_init(&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), settings.window_width, settings.window_height);
	gtk_window_set_title(GTK_WINDOW(window), "Retrovol");
	
	//use a scrolled window
	GtkWidget *scrolled_window;
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy((GtkScrolledWindow*)scrolled_window, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(window), scrolled_window);
	
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
			//bind to the toggle_it function
			g_signal_connect (GTK_TOGGLE_BUTTON(chkbx), "toggled", G_CALLBACK (toggle_it), list.items[i]);
			gtk_container_add(GTK_CONTAINER(alignment), chkbx);
		} else if (strcmp(list.items[i]->type, "ENUMERATED") == 0){
			//tempory stuff - put label for enumerated
			GtkWidget *alignment;
			if (settings.vertical){
				alignment = gtk_alignment_new(0.5,0.5,0,0);
				gtk_box_pack_start(GTK_BOX(vbox), alignment, true, true, 0);
			} else {
				alignment = gtk_alignment_new(0.5,0.5,0,0);
				gtk_box_pack_end(GTK_BOX(vbox), alignment, true, true, 0);
			}
			GtkWidget *label;
			char text[16];
			list.items[i]->sget(text);
			label = gtk_label_new(text);
			gtk_container_add(GTK_CONTAINER(alignment), label);
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
			//bind to the toggle_it function
			g_signal_connect (GTK_TOGGLE_BUTTON(chkbx), "toggled", G_CALLBACK (toggle_it), &(list.elems[list.items[i]->switch_id]));
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
	
	
	//finish the gtk stuff
	gtk_widget_show_all(window);
	g_signal_connect(window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
	
	gtk_main();
	
	return(0);
}
