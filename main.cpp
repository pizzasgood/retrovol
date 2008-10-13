/* main.cpp */

#include <gtk/gtk.h>
#include "retro_slider.h"


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

	GtkWidget *window;
	GtkWidget *frame;

	GtkWidget *drawing_area;


	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);
	gtk_window_set_title(GTK_WINDOW(window), "custom slider test");

	frame = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(window), frame);


	drawing_area = gtk_drawing_area_new ();
	gtk_fixed_put(GTK_FIXED(frame), drawing_area, 10, 10);
	gtk_widget_set_size_request (drawing_area, 20, 100);
	retro_slider slider_one;
	g_signal_connect (G_OBJECT (drawing_area), "expose_event", G_CALLBACK (retro_slider::expose_event_callback), &slider_one);
	g_signal_connect (G_OBJECT (drawing_area), "button_press_event", G_CALLBACK (&retro_slider::button_press_event_callback), &slider_one);
	g_signal_connect (G_OBJECT (drawing_area), "motion_notify_event", G_CALLBACK (&retro_slider::motion_notify_event_callback), &slider_one);
	gtk_widget_set_events (drawing_area, GDK_EXPOSURE_MASK
							| GDK_LEAVE_NOTIFY_MASK
							| GDK_BUTTON_PRESS_MASK
							| GDK_POINTER_MOTION_MASK
							| GDK_POINTER_MOTION_HINT_MASK);






	gtk_widget_show_all(window);

	g_signal_connect(window, "destroy",
		G_CALLBACK (gtk_main_quit), NULL);

	gtk_main();

	return(0);
}
