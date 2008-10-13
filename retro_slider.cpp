/* retro_slider.cpp */

#include <gtk/gtk.h>
#include "retro_slider.h"


//Constructor for retro_slider - takes care of all the annoying gtk initialization jazz, just supply the frame
retro_slider::retro_slider(GtkWidget *frame, int xpos, int ypos, int _width, int _height, void *_obj, float (*_get_func)(void*), float (*_set_func)(void*,float)){
	obj = _obj;
	get_func = _get_func;
	set_func = _set_func;

	//I am a lazy bum, so I stuck all this crap in the constructor :)
	GtkWidget *drawing_area;
	drawing_area = gtk_drawing_area_new ();
	gtk_fixed_put(GTK_FIXED(frame), drawing_area, xpos, ypos);
	gtk_widget_set_size_request (drawing_area, _width, _height);
	g_signal_connect (G_OBJECT (drawing_area), "button_press_event", G_CALLBACK (&retro_slider::button_press_event_callback), this);
	g_signal_connect (G_OBJECT (drawing_area), "motion_notify_event", G_CALLBACK (&retro_slider::motion_notify_event_callback), this);
	g_signal_connect (G_OBJECT (drawing_area), "scroll_event", G_CALLBACK (&retro_slider::scroll_event_callback), this);
	g_signal_connect (G_OBJECT (drawing_area), "expose_event", G_CALLBACK (retro_slider::expose_event_callback), this);
	g_signal_connect (G_OBJECT (drawing_area), "configure_event", G_CALLBACK (retro_slider::configure_event_callback), this);
	gtk_widget_set_events (drawing_area, GDK_EXPOSURE_MASK
							| GDK_LEAVE_NOTIFY_MASK
							| GDK_BUTTON_PRESS_MASK
							| GDK_POINTER_MOTION_MASK
							| GDK_POINTER_MOTION_HINT_MASK
							| GDK_SCROLL_MASK);


	//default values
	margin = 2;
	seg_thickness = 2;
	seg_spacing = 1;
	
	//total distance from the top of one segment to the top of the next
	seg_offset = seg_thickness + seg_spacing;
	
	//load the initial value
	val = get_func(obj);
	
}



//this translates from the y coordinate above the widget to the index of a segment
//segments are indexed from 0, starting at the top
int retro_slider::y_to_seg(int ypos){
	int ret = (ypos - margin + 0.5*seg_spacing)/seg_offset;
	if (ret > num_segs){
		ret = num_segs;
	} else if (ret < 0){
		ret = 0;
	}
	return(ret);
}

//this translates from the y coordinate above the widget to the value of the segment
int retro_slider::y_to_val(int ypos){
	return(seg_to_val(y_to_seg(ypos)));
}

//translates from a 0-100 float to the index of the corresponding segment
int retro_slider::val_to_seg(float _val){
	int ret = (100-_val)/val_per_seg;
	if (ret > num_segs){
		ret = num_segs;
	} else if (ret < 0){
		ret = 0;
	}
	return(ret);
}

//translates from the index of a segment to the value it corresponds to
float retro_slider::seg_to_val(int _seg){
	float ret = 100-val_per_seg*_seg;
	if (ret > 100){
		ret = 100;
	} else if (ret < 0){
		ret = 0;
	}
	return(ret);
}




//slide that slider!
void retro_slider::slide_the_slider(float ypos){
	val = set_func(obj, y_to_val(ypos));
	seg = val_to_seg(val);
}



//if there was a button press, update the slider
gboolean retro_slider::button_press_event_callback (GtkWidget *widget, GdkEventButton *event, retro_slider *slider){
	slider->slide_the_slider(event->y);
	gtk_widget_queue_draw_area(widget, 0, 0, widget->allocation.width, widget->allocation.height);
	return(true);
}

//if there is clicking and dragging, update the slider
gboolean retro_slider::motion_notify_event_callback( GtkWidget *widget, GdkEventMotion *event, retro_slider *slider){
	int x, y;
	GdkModifierType state;
	
	//mumbo jumbo that is supposed to decrease the number of times this stuff gets run
	if (event->is_hint){
		gdk_window_get_pointer (event->window, &x, &y, &state);
	} else {
		x = event->x;
		y = event->y;
		state = (GdkModifierType)event->state;
	}
	
	//only do anything if a mouse button is pressed
	if (state & GDK_BUTTON1_MASK || state & GDK_BUTTON2_MASK || state & GDK_BUTTON3_MASK){
		slider->slide_the_slider(event->y);
		gtk_widget_queue_draw_area(widget, 0, 0, widget->allocation.width, widget->allocation.height);
  	}

	return(true);
}

//handle scrolling
gboolean retro_slider::scroll_event_callback (GtkWidget *widget, GdkEventScroll *event, retro_slider *slider){
	int ret;
	
	//check direction
	switch(event->direction){
		case GDK_SCROLL_UP:
		case GDK_SCROLL_RIGHT:
			ret = slider->seg - 1;
			break;
		default:
			ret = slider->seg + 1;
			break;
	}
	
	//limit to between 0 and num_segs
	if (ret > slider->num_segs){
		ret = slider->num_segs;
	} else if (ret < 0){
		ret = 0;
	}
	
	//update values
	slider->val = slider->set_func(slider->obj, slider->seg_to_val(ret));
	slider->seg = slider->val_to_seg(slider->val);
	
	gtk_widget_queue_draw_area(widget, 0, 0, widget->allocation.width, widget->allocation.height);
	return(true);
}


//set the size and calculate how many segments there should be and what-not
gboolean retro_slider::configure_event_callback (GtkWidget *widget, GdkEventConfigure *event, retro_slider *slider){
	//width and height of the widget, margin included
	slider->width = widget->allocation.width;
	slider->height = widget->allocation.height;
	
	//need to find out how many segments will be used
	slider->num_segs = (slider->height - 2*slider->margin + slider->seg_spacing) / slider->seg_offset;
	slider->slider_height = slider->num_segs * slider->seg_offset;
	
	//and find how much value each segment is worth
	slider->val_per_seg = 100/((float)slider->num_segs);
	
	//update the seg variable
	slider->seg = slider->val_to_seg(slider->val);

	return(true);
}

//draw the slider
gboolean retro_slider::expose_event_callback (GtkWidget *widget, GdkEventExpose *event, retro_slider *slider){
	
	//update the value
	slider->val = slider->get_func(slider->obj);
	slider->seg = slider->val_to_seg(slider->val);
	
	//load the cairo object and draw the bg
	cairo_t *cr;
	cr = gdk_cairo_create(widget->window);
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_paint(cr);
	
	//draw the segments
	for (int i=0; i<slider->num_segs; i++){
		if (i < slider->seg){
			cairo_set_source_rgb(cr, 0.6, 0.2, 0.0); //unlit
		} else {
			cairo_set_source_rgb(cr, 1.0, 0.8, 0.0); //lit
		}
		cairo_rectangle(cr, slider->margin, slider->margin+i*slider->seg_offset, slider->width-2*slider->margin, slider->seg_thickness);
		cairo_fill(cr);
	}
	
	//clean up
	cairo_destroy(cr);

	return(true);
}







