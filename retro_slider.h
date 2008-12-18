/* retro_slider.h */

#ifndef __RETRO_SLIDER__
#define __RETRO_SLIDER__


class retro_slider{
	private:
		int y_to_seg(int ypos);
		int y_to_val(int ypos);
		int val_to_seg(float _val);
		float seg_to_val(int _seg);
		void *obj;
		float (*get_func)(void*);
		float (*set_func)(void*,float);
		
		//pointers to the GtkWidgets
		GtkWidget *frame;
		GtkWidget *drawing_area;
		GtkWidget *label;
		
	public:
		retro_slider();
		retro_slider(GtkWidget *_frame, int _width, int _height, void *_obj, float (*_get_func)(void*), float (*_set_func)(void*,float));
		void init(GtkWidget *_frame, void *_obj, float (*_get_func)(void*), float (*_set_func)(void*,float));
		
		
		float val;			//the current value
		int seg;			//index of the highest segment to be 'lit', with 0 at the top
		
		int margin;
		int seg_thickness;	//thickness of each line
		int seg_spacing;	//space between the bottom of one segment and the top of the next
		
		int seg_offset;		//distance from the top of one segment to the top of the next
		
		int height;			//dimensions of the slider
		int width;			//
		
		int num_segs;		//number of segments
		int slider_height;	//the height of the slider itself (no margins and no "slack" from funny-sized widgets)
		float val_per_seg;  //how much each segment is "worth"
		
		bool vertical;	//when true, the sliders are vertical
		
		
		//slides the slider, updates the val and seg variables
		void slide_the_slider(float ypos);
		
		//all the callbacks for button presses and whatnot
		static gboolean button_press_event_callback (GtkWidget *widget, GdkEventButton *event, retro_slider *slider);
		static gboolean motion_notify_event_callback( GtkWidget *widget, GdkEventMotion *event, retro_slider *slider);
		static gboolean scroll_event_callback (GtkWidget *widget, GdkEventScroll *event, retro_slider *slider);
		static gboolean configure_event_callback (GtkWidget *widget, GdkEventConfigure *event, retro_slider *slider);
		static gboolean expose_event_callback (GtkWidget *widget, GdkEventExpose *event, retro_slider *slider);
		
		
};




#endif
