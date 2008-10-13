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
	//do the alsa stuff
	char card[] = "hw:0";
	ElementList list(card);
	
	
	//start the gtk stuff
	GtkWidget *window;
	GtkWidget *frame;
	gtk_init(&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), 10+list.num_items*30, 160);
	gtk_window_set_title(GTK_WINDOW(window), "custom slider test");
	frame = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(window), frame);
	
	
	//add the sliders
	retro_slider *sliders = new retro_slider[list.num_items];
	for(int i=0; i<list.num_items; i++){
		if (strcmp(list.items[i]->type, "INTEGER") == 0){
			sliders[i].init(frame, 10+i*30, 10, 20, 102, (void*)list.items[i], &Element::get_callback, &Element::set_callback);
		}
	}
	
	
	//finish the gtk stuff
	gtk_widget_show_all(window);
	g_signal_connect(window, "destroy",
		G_CALLBACK (gtk_main_quit), NULL);
	
	gtk_main();
	
	return(0);
}
