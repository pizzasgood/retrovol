/* retro_slider.h */

#ifndef __RETRO_SLIDER__
#define __RETRO_SLIDER__

class retro_slider{
	private:
		int y_to_seg(int ypos);
		int val_to_seg(float _val);
		float seg_to_val(int _seg);
	public:
		retro_slider();
		retro_slider(int _val);
		
		float val;			//the current value
		int seg;			//index of the highest segment to be 'lit', with 0 at the top
		
		
		int margin;
		int seg_thickness;	//thickness of each line
		int seg_spacing;	//space between the bottom of one segment and the top of the next
		
		int seg_offset;
		int height;
		int width;
		
		int num_segs;
		int slider_height;
		float val_per_seg;
		
		
		
		
		//translates the y-coord of a click into a value and updates the val variable
		void translate_to_val(float ypos);
		static gboolean button_press_event_callback (GtkWidget *widget, GdkEventButton *event, retro_slider *slider);
		static gboolean motion_notify_event_callback( GtkWidget *widget, GdkEventMotion *event, retro_slider *slider);
		static gboolean expose_event_callback (GtkWidget *widget, GdkEventExpose *event, retro_slider *slider);
		gboolean expose_event (GtkWidget *widget, GdkEventExpose *event);
		
		
		
};




#endif
