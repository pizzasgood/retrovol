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

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), 190, 160);
	gtk_window_set_title(GTK_WINDOW(window), "custom slider test");

	frame = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(window), frame);

	retro_slider slider_a(frame, 10, 10, 20, 100);
	retro_slider slider_b(frame, 40, 10, 20, 100);
	retro_slider slider_c(frame, 70, 10, 20, 100);
	retro_slider slider_d(frame, 100, 10, 20, 100);
	retro_slider slider_e(frame, 130, 10, 20, 100);
	retro_slider slider_f(frame, 160, 10, 20, 100);


	gtk_widget_show_all(window);

	g_signal_connect(window, "destroy",
		G_CALLBACK (gtk_main_quit), NULL);

	gtk_main();

	return(0);
}
