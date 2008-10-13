/* retro_slider.cpp */

#include <gtk/gtk.h>
#include "retro_slider.h"



retro_slider::retro_slider(){
	val = 0;

	margin = 0;
	seg_thickness = 2;
	seg_spacing = 1;
	
	//total distance from the top of one segment to the top of the next
	seg_offset = seg_thickness + seg_spacing;
}
retro_slider::retro_slider(int _val){
	val = _val;

	margin = 0;
	seg_thickness = 1;
	seg_spacing = 1;

	//total distance from the top of one segment to the top of the next
	seg_offset = seg_thickness + seg_spacing;
}





void retro_slider::translate_to_val(float ypos){
	seg = y_to_seg(ypos);
	val = seg_to_val(seg);
	printf("Y: %f\tval: %f\t seg: %d\n", ypos, val, seg);
}

gboolean retro_slider::button_press_event_callback (GtkWidget *widget, GdkEventButton *event, retro_slider *slider){
	
	slider->translate_to_val(event->y);
	return(true);
}

gboolean retro_slider::motion_notify_event_callback( GtkWidget *widget, GdkEventMotion *event, retro_slider *slider){
	int x, y;
	GdkModifierType state;

	if (event->is_hint){
		gdk_window_get_pointer (event->window, &x, &y, &state);
	} else {
		x = event->x;
		y = event->y;
		state = (GdkModifierType)event->state;
	}
	
	if (state & GDK_BUTTON1_MASK || state & GDK_BUTTON2_MASK || state & GDK_BUTTON3_MASK){
		slider->translate_to_val(event->y);
		g_signal_emit_by_name(widget, "expose_event");
  	}

	return(true);
}


gboolean retro_slider::expose_event_callback (GtkWidget *widget, GdkEventExpose *event, retro_slider *slider){
	return(slider->expose_event(widget, event));
}

gboolean retro_slider::expose_event (GtkWidget *widget, GdkEventExpose *event){

	/* need to put these into a better spot */
	//width and height of the widget, margin included
	width = widget->allocation.width;
	height = widget->allocation.height;
	//need to find out how many segments will be used
	num_segs = (height - 2*margin + seg_spacing) / seg_offset;
	slider_height = num_segs * seg_offset;
	//and find how much value each segment is worth
	val_per_seg = 100/((float)num_segs);
	
	
	
	cairo_t *cr;
	
	cr = gdk_cairo_create(widget->window);
	cairo_set_source_rgb(cr, 0, 0, 0);
	//cairo_paint(cr);
	
	for (int i=0; i<num_segs; i++){
		if (i < seg){
			cairo_set_source_rgb(cr, 0.6, 0.2, 0.0);
		} else {
			cairo_set_source_rgb(cr, 1.0, 0.8, 0.0);
		}
		cairo_rectangle(cr, margin, margin+i*seg_offset, width-2*margin, seg_thickness);
		cairo_fill(cr);
	}
		
	cairo_destroy(cr);

	return(true);
}


int retro_slider::y_to_seg(int ypos){
	int ret = (ypos - margin + 0.5*seg_spacing)/seg_offset;
	if (ret > num_segs){
		ret = num_segs;
	} else if (ret < 0){
		ret = 0;
	}
	return(ret);
}

int retro_slider::val_to_seg(float _val){
	int ret = (100-_val)/val_per_seg;
	if (ret > num_segs){
		ret = num_segs;
	} else if (ret < 0){
		ret = 0;
	}
	return(ret);
}

float retro_slider::seg_to_val(int _seg){
	float ret = 100-val_per_seg*_seg;
	if (ret > 100){
		ret = 100;
	} else if (ret < 0){
		ret = 0;
	}
	return(ret);
}

