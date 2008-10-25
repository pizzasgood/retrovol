/* main.cpp */

#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>
#include "retro_slider.h"
#include "alsa_classes.h"


gint count = 0;
char buf[5];



void increase(GtkWidget *widget, GtkWidget *fancy_bar){
	count+=4;
	if (count > 100){
		count=100;
	}

	sprintf(buf, "%d", count);
	//gtk_label_set_text(label, buf);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(fancy_bar), buf);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(fancy_bar), count/100.0);
}

void decrease(GtkWidget *widget, GtkWidget *fancy_bar){
	count-=4;
	if (count < 0){
		count=0;
	}
	sprintf(buf, "%d", count);
	//gtk_label_set_text(label, buf);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(fancy_bar), buf);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(fancy_bar), count/100.0);
}

int main(int argc, char** argv) {
	//do the alsa stuff, load the controls into list
	char card[] = "hw:0";
	ElementList list(card);

	
	//set up the window
	GtkWidget *window;
	gtk_init(&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), 480, 160);
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
			//the rslider pseudo-widget likes to be inside a container, lets use a GtkAlignment
			GtkWidget *alignment;
			alignment = gtk_alignment_new(0.5,0.5,0,0);
			gtk_box_pack_start(GTK_BOX(vbox), alignment, false, false, 0);
			
			//make the slider and associate with a control
			sliders[i].init(alignment, 20, 102, (void*)list.items[i], &Element::get_callback, &Element::set_callback);
		}
		//display the name of the control
		GtkWidget *label;
		label = gtk_label_new(list.items[i]->short_name);
		gtk_box_pack_start(GTK_BOX(vbox), label, false, false, 0);
	}
	
	
	//finish the gtk stuff
	gtk_widget_show_all(window);
	g_signal_connect(window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
	
	gtk_main();
	
	return(0);
}
