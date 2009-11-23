/* config_settings.h */
/* This code is part of the Public Domain. */

#ifndef __CONFIG_SETTINGS__
#define __CONFIG_SETTINGS__

//define the paths to the volume images
#define VOL_MUTED_IMAGE IMAGEDIR"/audio-volume-muted.png"
#define VOL_LOW_IMAGE IMAGEDIR"/audio-volume-low.png"
#define VOL_MEDIUM_IMAGE IMAGEDIR"/audio-volume-medium.png"
#define VOL_HIGH_IMAGE IMAGEDIR"/audio-volume-high.png"

class ConfigSettings {
	public:
		ConfigSettings();
		
		//some "global" variables - I put them in here to not have so much global stuff scattered around
		GtkWidget *main_window;
		GtkWidget *config_window;
		GtkWidget *slider_window;
		GtkWidget *tray_icon;
		GtkWidget *tray_icon_image;
		retro_slider *tray_slider;
		Element *tray_control;
		bool restart;

		char _config_file[80]; //NEED TO MAKE THIS DYNAMIC
		
		//the rest is the actual settings stuff
		
		char card[16], _d_card[16];
		
		int num_names;
		char name_list[80][80]; //NEED TO MAKE THIS DYNAMIC
		
		bool vertical, _d_vertical;			//when true, the normal sliders are vertical
		
		int window_width, _d_window_width;		//dimensions of the window
		int window_height, _d_window_height;		//
		
		int slider_width, _d_slider_width;		//dimensions of the slider
		int slider_height, _d_slider_height;		//
		
		int slider_margin, _d_slider_margin;		//the space around the segments of the slider
		
		int seg_thickness, _d_seg_thickness;		//thickness of each line
		int seg_spacing, _d_seg_spacing;  		//space between the bottom of one segment and the top of the next
		
		float background_color[3], _d_background_color[3];	//background color
		float border_color[3], _d_border_color[3];		//border color
		float unlit_color[3], _d_unlit_color[3];		//color of unlit segments
		float lit_color[3], _d_lit_color[3];			//color of lit segments
		
		bool enable_tray_icon, _d_enable_tray_icon;		//enable the tray icon
		bool tray_slider_vertical, _d_tray_slider_vertical;	//when true, the tray_slider is vertical
		int tray_slider_width, _d_tray_slider_width;		//dimensions of the tray_slider
		int tray_slider_height, _d_tray_slider_height;		//
		char tray_control_name[80]; //NEED TO MAKE THIS DYNAMIC
		
		char icon_file_names[4][80]; //NEED TO MAKE THIS DYNAMIC
		
		
		//apply the defaults
		void apply_defaults();

		//copy the settings of another ConfigSettings into this one
		void copy_settings(ConfigSettings *ptr);

		//parse the config file
		void parse_config(char *config_file);

		//write the config file
		void write_config();

		//reorder the list to match the config file
		void reorder_list(ElementList *list);
		
		//look through the list and set the tray_slider_control to the matching element
		void set_tray_slider(ElementList *list);
		
		//apply settings to a slider
		void apply_to_slider(retro_slider *slider);
		void apply_to_tray_slider(retro_slider *slider);

		//take a hex string like #AAFF88 and put it into a three item integer array
		void htoi(int *array, char *string);

		//take a hex string like #AAFF88 and put it into a three item float array, normalized so 255=1.0, 0=0.0
		void htonf(float *array, char *string);

		//take a 3 item integer array and convert it into a hex string like #AAFF88
		void itoh(int *array, char *string);

		//take a 3 item normalized float array and convert it into a hex string like #AAFF88
		void nftoh(float *array, char *string);
		
	private:
		
};

#endif
