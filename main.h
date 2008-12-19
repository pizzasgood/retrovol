/* main.h */

class ConfigSetttings {
	public:
		ConfigSetttings();
		
		char card[16];
		
		int num_names;
		char name_list[80][80]; //NEED TO MAKE THIS DYNAMIC
		
		bool vertical;			//when true, the sliders are vertical
		
		int window_width;		//dimensions of the window
		int window_height;		//
		
		int slider_width;		//dimensions of the slider
		int slider_height;		//
		
		int slider_margin;		//the space around the segments of the slider
		
		int seg_thickness;		//thickness of each line
		int seg_spacing;  		//space between the bottom of one segment and the top of the next
		
		float background_color[3];	//background color
		float border_color[3];		//border color
		float unlit_color[3];		//color of unlit segments
		float lit_color[3];			//color of lit segments
		
		
		//parse the config file
		void parse_config(char *config_file);

		//reorder the list to match the config file
		void reorder_list(ElementList *list);
		
		//apply settings to a slider
		void apply_to_slider(retro_slider *slider);

		//take a hex string like #AAFF88 and put it into a three item integer array
		void htoi(int *array, char *string);

		//take a hex string like #AAFF88 and put it into a three item float array, normalized so 255=1.0, 0=0.0
		void htonf(float *array, char *string);
		
	private:
		
};



//callback that handles muting/unmuting a control
void toggle_it(GtkWidget *chkbx, Element *elem);

//replace the second space (or the first if preceeding a '(' or '-') with '\n'
//else append a '\n' to keep things lined up
void word_wrap(char *wrapped, char *orig);




