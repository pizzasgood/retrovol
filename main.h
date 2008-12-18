/* main.h */

class ConfigSetttings {
	public:
		ConfigSetttings();
		
		char card[16];
		
		int num_names;
		char name_list[80][80]; //NEED TO MAKE THIS DYNAMIC
		
		bool vertical;
		
		int window_width;
		int window_height;
		
		int slider_width;
		int slider_height;
		
		int slider_margin;
		
		int seg_thickness;
		int seg_spacing;
		
		
		//parse the config file
		void parse_config(char *config_file);

		//reorder the list to match the config file
		void reorder_list(ElementList *list);
		
		//apply settings to a slider
		void apply_to_slider(retro_slider *slider);
		
	private:
		
};



//callback that handles muting/unmuting a control
void toggle_it(GtkWidget *chkbx, Element *elem);

//replace the second space (or the first if preceeding a '(' or '-') with '\n'
//else append a '\n' to keep things lined up
void word_wrap(char *wrapped, char *orig);



