/* retro_slider.h */
/* This code is part of the Public Domain. */

#ifndef __RETRO_SLIDER__
#define __RETRO_SLIDER__


class retro_slider{
	public:
		//used to specify which channels an action should impact
		enum channel_enum {
			left_chan = 1,
			right_chan = 2,
			all_chan = 3
		};


	private:
		int channels;				//the number of channels

		int y_to_seg(int ypos);
		int y_to_val(int ypos);
		int val_to_seg(float _val);
		float seg_to_val(int _seg);
		void update();
		void *obj;
		float (*get_func)(void*, int);
		float (*set_func)(void*,float, int);
		
		//pointers to the GtkWidgets
		GtkWidget *frame;
		GtkWidget *drawing_area;
		GtkWidget *label;
		
		//determine which channels should be modified
		channel_enum get_chan_mask(GdkEventKey *event);
		channel_enum get_chan_mask(GdkEventScroll *event);
		channel_enum get_chan_mask(GdkEventMotion *event);
		channel_enum get_chan_mask(GdkEventButton *event);
		
	public:
		retro_slider();
		retro_slider(GtkWidget *_frame, int _width, int _height, void *_obj, float (*_get_func)(void*,int), float (*_set_func)(void*,float,int), bool _stereo=false);
		void init(GtkWidget *_frame, void *_obj, float (*_get_func)(void*,int), float (*_set_func)(void*,float,int), bool _stereo=false);
		
		
		bool stereo;				//whether it is one or two channels
		float val[2];				//the current value
		int seg[2];				//index of the highest segment to be 'lit', with 0 at the top

		int margin;
		int seg_thickness;		//thickness of each line
		int seg_spacing;		//space between the bottom of one segment and the top of the next

		int seg_offset;			//distance from the top of one segment to the top of the next

		int height;				//dimensions of the slider
		int width;				//

		int num_segs;			//number of segments
		int slider_height;		//the height of the slider itself (no margins and no "slack" from funny-sized widgets)
		float val_per_seg;  	//how much each segment is "worth"

		bool vertical;			//when true, the sliders are vertical
		
		float background_color[3];	//background color
		float border_color[3];		//border color
		float unlit_color[3];		//color of unlit segments
		float lit_color[3];			//color of lit segments
		
		//slides the slider, updates the val and seg variables
		void slide_the_slider(float ypos, channel_enum chan_mask = all_chan);
		
		//all the callbacks for button presses and whatnot
		static gboolean button_press_event_callback (GtkWidget *widget, GdkEventButton *event, retro_slider *slider);
		static gboolean key_event_callback (GtkWidget *widget, GdkEventKey *event, retro_slider *slider);
		static gboolean motion_notify_event_callback( GtkWidget *widget, GdkEventMotion *event, retro_slider *slider);
		static gboolean scroll_event_callback (GtkWidget *widget, GdkEventScroll *event, retro_slider *slider);
		static gboolean configure_event_callback (GtkWidget *widget, GdkEventConfigure *event, retro_slider *slider);
		static gboolean expose_event_callback (GtkWidget *widget, GdkEventExpose *event, retro_slider *slider);
		
		
};




#endif
