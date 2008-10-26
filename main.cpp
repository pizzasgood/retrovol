/* main.cpp */

/*
    retrovol - a retro-styled volume mixer, by Pizzasgood
*/

#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>
#include "retro_slider.h"
#include "alsa_classes.h"



//callback that handles muting/unmuting a control
void toggle_it(GtkWidget *chkbx, Element *elem){
	bool state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkbx));
	elem->set((int)state);
}




int main(int argc, char** argv) {
	//load the controls into list
	char card[] = "hw:0";
	ElementList list(card);

	
	//set up the window
	GtkWidget *window;
	gtk_init(&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), 480, 164);
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
	hbox = gtk_hbox_new(TRUE, 2);
	gtk_container_add(GTK_CONTAINER(viewport), hbox);
	
			
	//add the sliders
	retro_slider *sliders = new retro_slider[list.num_items];
	for(int i=0; i<list.num_items; i++){
		//use a vbox w/ slider on top and label on bottom
		GtkWidget *vbox;
		vbox = gtk_vbox_new(FALSE, 2);
		gtk_box_pack_start(GTK_BOX(hbox), vbox, false, false, 0);
		
		if (strcmp(list.items[i]->type, "INTEGER") == 0){
			//integers need sliders
			//the rslider pseudo-widget likes to be inside a container, lets use a GtkAlignment
			GtkWidget *frame;
			frame = gtk_alignment_new(0.5,0.0,0,0);
			gtk_box_pack_start(GTK_BOX(vbox), frame, false, false, 0);
			//make the slider and associate with a control
			sliders[i].init(frame, 20, 102, (void*)list.items[i], &Element::get_callback, &Element::set_callback);
		
		} else if (strcmp(list.items[i]->type, "BOOLEAN") == 0){
			//booleans need checkboxes
			GtkWidget *alignment;
			alignment = gtk_alignment_new(0.5,1.0,0,0);
			gtk_box_pack_start(GTK_BOX(vbox), alignment, true, true, 0);
			GtkWidget *chkbx;
			chkbx = gtk_check_button_new();
			//set it to the current state
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkbx), (bool)list.items[i]->get());
			//bind to the toggle_it function
			g_signal_connect (GTK_TOGGLE_BUTTON(chkbx), "toggled", G_CALLBACK (toggle_it), list.items[i]);
			gtk_container_add(GTK_CONTAINER(alignment), chkbx);
		}
		
		//add a checkbox for sliders that are muteable
		if (list.items[i]->switch_id >= 0){
			GtkWidget *alignment;
			alignment = gtk_alignment_new(0.5,1.0,0,0);
			gtk_box_pack_start(GTK_BOX(vbox), alignment, true, true, 0);
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
		alignment = gtk_alignment_new(0.5,1.0,0,0);
		gtk_box_pack_end(GTK_BOX(vbox), alignment, false, false, 0);
		GtkWidget *label;
		label = gtk_label_new(list.items[i]->short_name);
		gtk_container_add(GTK_CONTAINER(alignment), label);
	}
	
	
	//finish the gtk stuff
	gtk_widget_show_all(window);
	g_signal_connect(window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
	
	gtk_main();
	
	return(0);
}
