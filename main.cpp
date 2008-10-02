#include <gtk/gtk.h>
#include "retro_slider.h"


gint count = 0;
char buf[5];




static void set_value(GtkWidget * widget, gpointer data)
{
  GdkRegion *region;

  GtkRange *range = (GtkRange *) widget;
  GtkWidget *rslider = (GtkWidget *) data;
  GTK_rslider(rslider)->sel = gtk_range_get_value(range);

  region = gdk_drawable_get_clip_region(rslider->window);
  gdk_window_invalidate_region(rslider->window, region, TRUE);
  gdk_window_process_updates(rslider->window, TRUE);
}




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

	GtkWidget *label;
	GtkWidget *window;
	GtkWidget *frame;
	GtkWidget *plus;
	GtkWidget *minus;
	GtkWidget *fancy_bar;
GtkWidget *scale;
GtkWidget *rslider;

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), 250, 400);
	gtk_window_set_title(GTK_WINDOW(window), "custom slider test");

	frame = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(window), frame);

	plus = gtk_button_new_with_label("+");
	gtk_widget_set_size_request(plus, 80, 35);
	gtk_fixed_put(GTK_FIXED(frame), plus, 50, 20);

	minus = gtk_button_new_with_label("-");
	gtk_widget_set_size_request(minus, 80, 35);
	gtk_fixed_put(GTK_FIXED(frame), minus, 50, 80);

	label = gtk_label_new("0");
	gtk_fixed_put(GTK_FIXED(frame), label, 180, 58); 
	
	fancy_bar = gtk_progress_bar_new();
	gtk_widget_set_size_request(fancy_bar, 30, 150);
	gtk_fixed_put(GTK_FIXED(frame), fancy_bar, 200, 24);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(fancy_bar), 0);
	gtk_progress_bar_set_orientation(GTK_PROGRESS_BAR(fancy_bar), GTK_PROGRESS_BOTTOM_TO_TOP);
	


rslider = gtk_rslider_new();
gtk_fixed_put(GTK_FIXED(frame), rslider, 30, 220);
gtk_widget_set_size_request(rslider, 20, 160);

scale = gtk_vscale_new_with_range(0.0, 100.0, 1.0);
gtk_range_set_inverted(GTK_RANGE(scale), TRUE);
gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_TOP);
gtk_widget_set_size_request(scale, 50, 120);
gtk_fixed_put(GTK_FIXED(frame), scale, 130, 200);

g_signal_connect(G_OBJECT(scale), "value_changed", G_CALLBACK(set_value), (gpointer) rslider);

gtk_widget_show(rslider);




	gtk_widget_show_all(window);

	g_signal_connect(window, "destroy",
		G_CALLBACK (gtk_main_quit), NULL);

	g_signal_connect(plus, "clicked", 
		G_CALLBACK(increase), fancy_bar);

	g_signal_connect(minus, "clicked", 
		G_CALLBACK(decrease), fancy_bar);

	gtk_main();

	return(0);
}
