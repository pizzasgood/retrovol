/* retro_slider.cpp */
/* This code is part of the Public Domain. */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <math.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "retro_slider.h"


//Constructor/initializer for retro_slider - takes care of all the annoying gtk initialization jazz,
//just supply the frame (a GtkContaier of some sort, like GtkAlign)
retro_slider::retro_slider(){
	//default values
	width=102;
	height=20;
	margin = 2;
	seg_thickness = 2;
	seg_spacing = 1;
	vertical = true;
	
	background_color[0]=0.0;
	background_color[1]=0.0;
	background_color[2]=0.0;
	
	border_color[0]=0.0;
	border_color[1]=0.0;
	border_color[2]=0.0;
	
	unlit_color[0]=0.6;
	unlit_color[1]=0.2;
	unlit_color[2]=0.0;
	
	lit_color[0]=1.0;
	lit_color[1]=0.8;
	lit_color[2]=0.0;
}
retro_slider::retro_slider(GtkWidget *_frame, int _width, int _height, void *_obj, float (*_get_func)(void*,int), float (*_set_func)(void*,float,int), bool _stereo){
	width=_width;
	height=_height;
	//default values
	margin = 2;
	seg_thickness = 2;
	seg_spacing = 1;
	vertical = true;
	init(_frame, _obj, _get_func, _set_func, _stereo);
}
void retro_slider::init(GtkWidget *_frame, void *_obj, float (*_get_func)(void*,int), float (*_set_func)(void*,float,int), bool _stereo){
	frame = _frame;
	obj = _obj;
	get_func = _get_func;
	set_func = _set_func;
	stereo = _stereo;

	channels = stereo ? 2 : 1;

	//add the slider itself
	drawing_area = gtk_drawing_area_new ();
	gtk_container_add(GTK_CONTAINER(frame), drawing_area);
	gtk_widget_set_size_request (drawing_area, width, height);
	g_signal_connect (G_OBJECT (drawing_area), "button_press_event", G_CALLBACK (&retro_slider::button_press_event_callback), this);
	g_signal_connect (G_OBJECT (drawing_area), "key_press_event", G_CALLBACK (&retro_slider::key_event_callback), this);
	g_signal_connect (G_OBJECT (drawing_area), "motion_notify_event", G_CALLBACK (&retro_slider::motion_notify_event_callback), this);
	g_signal_connect (G_OBJECT (drawing_area), "scroll_event", G_CALLBACK (&retro_slider::scroll_event_callback), this);
	g_signal_connect (G_OBJECT (drawing_area), "expose_event", G_CALLBACK (retro_slider::expose_event_callback), this);
	g_signal_connect (G_OBJECT (drawing_area), "configure_event", G_CALLBACK (retro_slider::configure_event_callback), this);
	gtk_widget_set_events (drawing_area, GDK_EXPOSURE_MASK
							| GDK_LEAVE_NOTIFY_MASK
							| GDK_BUTTON_PRESS_MASK
							| GDK_KEY_PRESS_MASK
							| GDK_POINTER_MOTION_MASK
							| GDK_POINTER_MOTION_HINT_MASK
							| GDK_SCROLL_MASK);
	g_object_set(drawing_area, "can-focus", true, NULL);

	
	//total distance from the top of one segment to the top of the next
	seg_offset = seg_thickness + seg_spacing;	


	//load the initial value
	for (int i=0; i<channels; i++){
		val[i] = get_func(obj, i);
	}
	
}



//this translates from the y coordinate above the widget to the index of a segment
//segments are indexed from 0, starting at the top
int retro_slider::y_to_seg(int ypos){
	int ret = (ypos - margin + seg_spacing/2)/seg_offset;
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
	int ret = round((100-_val)/val_per_seg);
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

//updates the slider to have the latest value
void retro_slider::update(){
	for (int i=0; i<channels; i++){
		val[i] = get_func(obj, i);
		seg[i] = val_to_seg(val[i]);
	}
}



//slide that slider!
void retro_slider::slide_the_slider(float ypos, channel_enum chan_mask){
	for (int i=0; i<channels; i++){
		//note: this trick only works if channels <= 2....
		if ((i+1) & chan_mask){
			val[i] = set_func(obj, y_to_val(ypos), i);
			seg[i] = val_to_seg(val[i]);
		}
	}
}




//determine which channels should be modified
retro_slider::channel_enum retro_slider::get_chan_mask(GdkEventKey *event){
	channel_enum chan_mask = all_chan;
	if (event->state & GDK_SHIFT_MASK){
		chan_mask = left_chan;
	} else if (event->state & GDK_CONTROL_MASK){
		chan_mask = right_chan;
	}
	return(chan_mask);
}
retro_slider::channel_enum retro_slider::get_chan_mask(GdkEventScroll *event){
	channel_enum chan_mask = all_chan;
	if (event->state & GDK_SHIFT_MASK){
		chan_mask = left_chan;
	} else if (event->state & GDK_CONTROL_MASK){
		chan_mask = right_chan;
	}
	return(chan_mask);
}
retro_slider::channel_enum retro_slider::get_chan_mask(GdkEventMotion *event){
	channel_enum chan_mask = all_chan;
	if (event->state & GDK_SHIFT_MASK){
		chan_mask = left_chan;
	} else if (event->state & GDK_CONTROL_MASK){
		chan_mask = right_chan;
	}
	return(chan_mask);
}
retro_slider::channel_enum retro_slider::get_chan_mask(GdkEventButton *event){
	channel_enum chan_mask = all_chan;
	if (event->state & GDK_SHIFT_MASK){
		chan_mask = left_chan;
	} else if (event->state & GDK_CONTROL_MASK){
		chan_mask = right_chan;
	}
	return(chan_mask);
}




//if there was a button press, update the slider
gboolean retro_slider::button_press_event_callback (GtkWidget *widget, GdkEventButton *event, retro_slider *slider){
	//ensure we start with the latest value
	slider->update();

	//slide it
	if (slider->vertical){
		slider->slide_the_slider(event->y, slider->get_chan_mask(event));
	} else {
		slider->slide_the_slider(slider->width - event->x - 1, slider->get_chan_mask(event));
	}
	gtk_widget_queue_draw_area(widget, 0, 0, widget->allocation.width, widget->allocation.height);
	return(true);
}

//handle keypresses
gboolean retro_slider::key_event_callback (GtkWidget *widget, GdkEventKey *event, retro_slider *slider){
	//ensure we start with the latest value
	slider->update();

	//check channel
	channel_enum chan_mask = slider->get_chan_mask(event);

	//check direction
	int delta;
	switch(event->keyval){
		case GDK_Up:
			if (slider->vertical){delta = -1;} else {return(false);}
			break;
		case GDK_Right:
			if (!slider->vertical){delta = -1;} else {return(false);}
			break;
		case GDK_Down:
			if (slider->vertical){delta = 1;} else {return(false);}
			break;
		case GDK_Left:
			if (!slider->vertical){delta = 1;} else {return(false);}
			break;
		case GDK_Page_Up:
			delta = -(slider->num_segs * 0.2);
			if (delta == 0){ delta = -1; }
			break;
		case GDK_Page_Down:
			delta = slider->num_segs * 0.2;
			if (delta == 0){ delta = 1; }
			break;
		default:
			return(false);
			break;
	}

	int direction = (delta > 0 ? 1 : -1);

	for (int i=0; i<slider->channels; i++){
		//note: this trick only works if channels <= 2....
		if ((i+1) & chan_mask){
			//save the original value
			float orig = slider->val[i];
			//if the hardware value hasn't changed yet, keep increasing the segment until it does
			while (slider->seg[i] <= slider->num_segs && slider->seg[i] >= 0 && slider->val[i] == orig){
				//move one segment
				slider->seg[i] += delta;
				//set the value to match the new segment, then read it back
				slider->val[i] = slider->set_func(slider->obj, slider->seg_to_val(slider->seg[i]), i);
			}
		}
	}

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
		//ensure we start with the latest value
		slider->update();

		//do the sliding
		if (slider->vertical){
			slider->slide_the_slider(event->y, slider->get_chan_mask(event));
		} else {
			slider->slide_the_slider(slider->width - event->x - 1, slider->get_chan_mask(event));
		}
		gtk_widget_queue_draw_area(widget, 0, 0, widget->allocation.width, widget->allocation.height);
  	}

	return(true);
}

//handle scrolling
gboolean retro_slider::scroll_event_callback (GtkWidget *widget, GdkEventScroll *event, retro_slider *slider){
	int delta;
	
	//ensure we start with the latest value
	slider->update();

	//check channel
	channel_enum chan_mask = slider->get_chan_mask(event);

	//check direction
	switch(event->direction){
		case GDK_SCROLL_UP:
		case GDK_SCROLL_RIGHT:
			delta = -1;
			break;
		default:
			delta = 1;
			break;
	}

	for(int i=0; i<slider->channels; i++){
		//note: this trick only works if channels <= 2....
		if ((i+1) & chan_mask){
			//save the original value
			float orig = slider->val[i];
			//if the hardware value hasn't changed yet, keep increasing the segment until it does
			while (slider->seg[i] <= slider->num_segs && slider->seg[i] >= 0 && slider->val[i] == orig){
				//move one segment
				slider->seg[i] += delta;
				//set the value to match the new segment, then read it back
				slider->val[i] = slider->set_func(slider->obj, slider->seg_to_val(slider->seg[i]), i);
			}
		}
	}

	//use slider->drawing_area instead of widget so that we can use this same function externally to scroll on the tray icon
	gtk_widget_queue_draw_area(slider->drawing_area, 0, 0, slider->drawing_area->allocation.width, slider->drawing_area->allocation.height);
	return(true);
}


//set the size and calculate how many segments there should be and what-not
gboolean retro_slider::configure_event_callback (GtkWidget *widget, GdkEventConfigure *event, retro_slider *slider){
	//width and height of the widget, margin included
	slider->width = widget->allocation.width;
	slider->height = widget->allocation.height;
	
	//need to find out how many segments will be used
	int size = slider->vertical ? slider->height : slider->width;
	slider->num_segs = (size - 2*slider->margin + slider->seg_spacing) / slider->seg_offset;
	slider->num_segs = slider->num_segs > 0 ? slider->num_segs : 1;
	
	//and find how much value each segment is worth
	slider->val_per_seg = 100/((float)slider->num_segs);
	
	//update the seg variable
	for (int i=0; i<slider->channels; i++){
		slider->seg[i] = slider->val_to_seg(slider->val[i]);
	}

	return(true);
}

//draw the slider
gboolean retro_slider::expose_event_callback (GtkWidget *widget, GdkEventExpose *event, retro_slider *slider){
	
	//update the value
	for (int i=0; i<slider->channels; i++){
		slider->val[i] = slider->get_func(slider->obj, i);
		slider->seg[i] = slider->val_to_seg(slider->val[i]);
	}
	
	//load the cairo object and draw the border
	cairo_t *cr;
	cr = gdk_cairo_create(widget->window);
	cairo_set_source_rgb(cr, slider->border_color[0], slider->border_color[1], slider->border_color[2]);
	cairo_paint(cr);
	
	//draw the background
	cairo_set_source_rgb(cr, slider->background_color[0], slider->background_color[1], slider->background_color[2]);
	if (slider->vertical){
		cairo_rectangle(cr, slider->margin, slider->margin, slider->width-2*slider->margin, slider->num_segs*slider->seg_offset-slider->seg_spacing);
	} else {
		cairo_rectangle(cr, slider->margin, slider->margin, slider->num_segs*slider->seg_offset-slider->seg_spacing, slider->height-2*slider->margin);
	}
	cairo_fill(cr);
	
	//draw the segments
	int chan_width = slider->stereo ? (slider->width-2*slider->margin)/2 : slider->width-2*slider->margin;
	int chan_height = slider->stereo ? (slider->height-2*slider->margin)/2 : slider->height-2*slider->margin;
	for (int c=0; c<slider->channels; c++){
		for (int i=0; i<slider->num_segs; i++){
			if (i < slider->seg[c]){
				cairo_set_source_rgb(cr, slider->unlit_color[0], slider->unlit_color[1], slider->unlit_color[2]); //unlit
			} else {
				cairo_set_source_rgb(cr, slider->lit_color[0], slider->lit_color[1], slider->lit_color[2]); //lit
			}
			if (slider->vertical){
				cairo_rectangle(cr, slider->margin + c*chan_width, slider->margin+i*slider->seg_offset, chan_width, slider->seg_thickness);
			} else {
				cairo_rectangle(cr, slider->width-slider->margin-(i+1)*slider->seg_offset+1, slider->margin + c*chan_height, slider->seg_thickness, chan_height);
			}
			cairo_fill(cr);
		}
	}
	
	//clean up
	cairo_destroy(cr);

	return(true);
}






