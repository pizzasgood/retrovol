/* config_settings.cpp */
/* This code is part of the Public Domain. */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h> //this is needed due to retro
#include "retro_slider.h"
#include "alsa_classes.h"
#include "config_settings.h"


ConfigSettings::ConfigSettings(){
	num_names = 0;
	//defaults
	strcpy(_d_card, "hw:0");
	_d_vertical = false;
	_d_window_width=270;
	_d_window_height=256;
	_d_slider_width=102;
	_d_slider_height=20;
	_d_slider_margin = 2;
	_d_seg_thickness = 2;
	_d_seg_spacing = 1;
	
	_d_background_color[0]=0.0;
	_d_background_color[1]=0.0;
	_d_background_color[2]=0.0;
	
	_d_border_color[0]=0.0;
	_d_border_color[1]=0.0;
	_d_border_color[2]=0.0;
	
	_d_unlit_color[0]=0.6;
	_d_unlit_color[1]=0.2;
	_d_unlit_color[2]=0.0;
	
	_d_lit_color[0]=1.0;
	_d_lit_color[1]=0.8;
	_d_lit_color[2]=0.0;
	
	_d_enable_tray_icon = true;
	_d_enable_tray_icon_background_color = false;
	_d_tray_icon_background_color[0]=0.8;
	_d_tray_icon_background_color[1]=0.8;
	_d_tray_icon_background_color[2]=0.8;
	_d_tray_slider_vertical = true;
	_d_tray_slider_width=20;
	_d_tray_slider_height=102;
	
	
	strcpy(icon_file_names[0], VOL_MUTED_IMAGE);
	strcpy(icon_file_names[1], VOL_LOW_IMAGE);
	strcpy(icon_file_names[2], VOL_MEDIUM_IMAGE);
	strcpy(icon_file_names[3], VOL_HIGH_IMAGE);

	apply_defaults();
}


void ConfigSettings::apply_defaults(){
	restart = false;
	strcpy(card, _d_card);
	vertical = _d_vertical;
	window_width = _d_window_width;
	window_height = _d_window_height;
	slider_width = _d_slider_width;
	slider_height = _d_slider_height;
	slider_margin = _d_slider_margin;
	seg_thickness = _d_seg_thickness;
	seg_spacing = _d_seg_spacing;
	
	background_color[0] = _d_background_color[0];
	background_color[1] = _d_background_color[1];
	background_color[2] = _d_background_color[2];
	
	border_color[0] = _d_border_color[0];
	border_color[1] = _d_border_color[1];
	border_color[2] = _d_border_color[2];
	
	unlit_color[0] = _d_unlit_color[0];
	unlit_color[1] = _d_unlit_color[1];
	unlit_color[2] = _d_unlit_color[2];
	
	lit_color[0] = _d_lit_color[0];
	lit_color[1] = _d_lit_color[1];
	lit_color[2] = _d_lit_color[2];
	
	enable_tray_icon = _d_enable_tray_icon;
	tray_slider_vertical = _d_tray_slider_vertical;
	tray_slider_width = _d_tray_slider_width;
	tray_slider_height = _d_tray_slider_height;
	
	enable_tray_icon_background_color = _d_enable_tray_icon_background_color;
	tray_icon_background_color[0] = _d_tray_icon_background_color[0];
	tray_icon_background_color[1] = _d_tray_icon_background_color[1];
	tray_icon_background_color[2] = _d_tray_icon_background_color[2];
	

}


//apply settings to a slider
void ConfigSettings::apply_to_slider(retro_slider *slider){
	slider->width = slider_width;
	slider->height = slider_height;
	slider->margin = slider_margin;
	slider->seg_thickness = seg_thickness;
	slider->seg_spacing = seg_spacing;
	slider->vertical = vertical;
	
	slider->background_color[0]=background_color[0];
	slider->background_color[1]=background_color[1];
	slider->background_color[2]=background_color[2];
	
	slider->border_color[0]=border_color[0];
	slider->border_color[1]=border_color[1];
	slider->border_color[2]=border_color[2];
	
	slider->unlit_color[0]=unlit_color[0];
	slider->unlit_color[1]=unlit_color[1];
	slider->unlit_color[2]=unlit_color[2];
	
	slider->lit_color[0]=lit_color[0];
	slider->lit_color[1]=lit_color[1];
	slider->lit_color[2]=lit_color[2];
}
//apply settings to a tray_slider
void ConfigSettings::apply_to_tray_slider(retro_slider *slider){
	apply_to_slider(slider);
	slider->width = tray_slider_width;
	slider->height = tray_slider_height;
	slider->vertical = tray_slider_vertical;
}


//copy the settings of another ConfigSettings into this one
void ConfigSettings::copy_settings(ConfigSettings *ptr){
	strcpy(card, ptr->card);
	list_ptr = ptr->list_ptr;
	num_names = ptr->num_names;
	vertical = ptr->vertical;
	window_width = ptr->window_width;
	window_height = ptr->window_height;
	slider_width = ptr->slider_width;
	slider_height = ptr->slider_height;
	slider_margin = ptr->slider_margin;
	seg_thickness = ptr->seg_thickness;
	seg_spacing = ptr->seg_spacing;
	for(int i=0; i<3; i++){
		background_color[i] = ptr->background_color[i];
		border_color[i] = ptr->border_color[i];
		unlit_color[i] = ptr->unlit_color[i];
		lit_color[i] = ptr->lit_color[i];
	}
	enable_tray_icon = ptr->enable_tray_icon;
	tray_slider_vertical = ptr->tray_slider_vertical;
	tray_slider_width = ptr->tray_slider_width;
	tray_slider_height = ptr->tray_slider_height;
	strcpy(tray_control_name, ptr->tray_control_name);
	enable_tray_icon_background_color = ptr->enable_tray_icon_background_color;
	tray_icon_background_color[0] = ptr->tray_icon_background_color[0];
	tray_icon_background_color[1] = ptr->tray_icon_background_color[1];
	tray_icon_background_color[2] = ptr->tray_icon_background_color[2];
	for(int i=0; i<num_names; i++){
		strcpy(name_list[i], ptr->name_list[i]);
	}
}


//parse the config file
void ConfigSettings::parse_config(char *config_file){
	//first set the defaults
	apply_defaults();
	//store the filename
	strcpy(_config_file, config_file);
	
	FILE *cfile = fopen(_config_file, "r");	
	if (!cfile){
		fprintf(stdout, "Cannot read file: %s\nUsing defaults...\n", _config_file);
		return;
	}
	
	char buffer[80];
	char *tmpptr;
	while (fgets(buffer, 80, cfile)){
		//use the # as a comment, and ignore newlines
		if (buffer[0] == '#' || buffer[0] == '\n'){
			continue;
		}
		tmpptr=strtok(buffer, "=\n");
		if (strcmp(tmpptr, "card")==0){
			tmpptr=strtok(NULL, "=\"\n");
			strcpy(card, tmpptr);
		} else if (strcmp(tmpptr, "vertical")==0){
			tmpptr=strtok(NULL, "=\n");
			vertical=(bool)atoi(tmpptr);
		} else if (strcmp(tmpptr, "window_width")==0){
			tmpptr=strtok(NULL, "=\n");
			window_width=atoi(tmpptr);
		} else if (strcmp(tmpptr, "window_height")==0){
			tmpptr=strtok(NULL, "=\n");
			window_height=atoi(tmpptr);
		} else if (strcmp(tmpptr, "slider_width")==0){
			tmpptr=strtok(NULL, "=\n");
			slider_width=atoi(tmpptr);
		} else if (strcmp(tmpptr, "slider_height")==0){
			tmpptr=strtok(NULL, "=\n");
			slider_height=atoi(tmpptr);
		} else if (strcmp(tmpptr, "slider_margin")==0){
			tmpptr=strtok(NULL, "=\n");
			slider_margin=atoi(tmpptr);
		} else if (strcmp(tmpptr, "seg_thickness")==0){
			tmpptr=strtok(NULL, "=\n");
			seg_thickness=atoi(tmpptr);
		} else if (strcmp(tmpptr, "seg_thickness")==0){
			tmpptr=strtok(NULL, "=\n");
			seg_thickness=atoi(tmpptr);
		} else if (strcmp(tmpptr, "background_color")==0){
			tmpptr=strtok(NULL, "=\n");
			htonf(background_color, tmpptr);
		} else if (strcmp(tmpptr, "border_color")==0){
			tmpptr=strtok(NULL, "=\n");
			htonf(border_color, tmpptr);
		} else if (strcmp(tmpptr, "unlit_color")==0){
			tmpptr=strtok(NULL, "=\n");
			htonf(unlit_color, tmpptr);
		} else if (strcmp(tmpptr, "lit_color")==0){
			tmpptr=strtok(NULL, "=\n");
			htonf(lit_color, tmpptr);
		} else if (strcmp(tmpptr, "enable_tray_icon")==0){
			tmpptr=strtok(NULL, "=\n");
			enable_tray_icon=(bool)atoi(tmpptr);
		} else if (strcmp(tmpptr, "tray_icon_background_color")==0){
			tmpptr=strtok(NULL, "=\n");
			htonf(tray_icon_background_color, tmpptr);
			enable_tray_icon_background_color=true;
		} else if (strcmp(tmpptr, "tray_slider_vertical")==0){
			tmpptr=strtok(NULL, "=\n");
			tray_slider_vertical=(bool)atoi(tmpptr);
		} else if (strcmp(tmpptr, "tray_slider_width")==0){
			tmpptr=strtok(NULL, "=\n");
			tray_slider_width=atoi(tmpptr);
		} else if (strcmp(tmpptr, "tray_slider_height")==0){
			tmpptr=strtok(NULL, "=\n");
			tray_slider_height=atoi(tmpptr);
		} else if (strcmp(tmpptr, "tray_control")==0){
			tmpptr=strtok(NULL, "=\"\n");
			strcpy(tray_control_name, tmpptr);
		} else if (strcmp(tmpptr, "sliders:")==0){
			int n;
			for (n=0; fgets(buffer, 80, cfile); n++){
				//ignore blank lines, comments, and erroneous junk
				if(buffer[0] != '\t' || buffer[1] == '#' || buffer[1] == '\n'){
					n--;
					continue;
				}
				//trim off the tab, two quotation marks and terminating newline if it exists
				char *buffer2 = strchr(buffer, '"')+1;
				while (buffer[strlen(buffer)-1] == '\n' || buffer[strlen(buffer)-1] == '"'){
					buffer[strlen(buffer)-1]='\0';
				}
				
				//put it into the array
				strcpy(name_list[n], buffer2);
				
			}
			num_names=n;
		}
	}
	
	fclose(cfile);
	
}


//write the config file
void ConfigSettings::write_config(){
	char tmpstr1[8];
	char tmpstr2[8];
	FILE *cfile = fopen(_config_file, "w");	
	if (!cfile){
		fprintf(stdout, "Cannot open file: %s\n for writing\n", _config_file);
		return;
	}
	
	fputs("# Config file for retrovol\n", cfile);
	fputs("# This file should reside in the user's home directory and be named .retrovolrc\n", cfile);

	fputs("\n", cfile);
	fputs("\n# Which soundcard to use\n", cfile);
	fprintf(cfile, "#card=%s\n", _d_card);
	if (strcmp(card, _d_card) != 0){ fprintf(cfile, "card=%s\n", card); }

	fputs("\n# Set this to 1 to make the sliders vertical, or 0 for horizontal (only applies to the main window)\n", cfile);
	fprintf(cfile, "#vertical=%d\n", _d_vertical);
	if (vertical != _d_vertical){ fprintf(cfile, "vertical=%d\n", vertical); }

	fputs("\n# Window dimensions\n", cfile);
	fprintf(cfile, "#window_width=%d\n", _d_window_width);
	if (window_width != _d_window_width){ fprintf(cfile, "window_width=%d\n", window_width); }
	fprintf(cfile, "#window_height=%d\n", _d_window_height);
	if (window_height != _d_window_height){ fprintf(cfile, "window_height=%d\n", window_height); }

	fputs("\n# Slider dimensions\n", cfile);
	fprintf(cfile, "#slider_width=%d\n", _d_slider_width);
	if (slider_width != _d_slider_width){ fprintf(cfile, "slider_width=%d\n", slider_width); }
	fprintf(cfile, "#slider_height=%d\n", _d_slider_height);
	if (slider_height != _d_slider_height){ fprintf(cfile, "slider_height=%d\n", slider_height); }
	fprintf(cfile, "#slider_margin=%d\n", _d_slider_margin);
	if (slider_margin != _d_slider_margin){ fprintf(cfile, "slider_margin=%d\n", slider_margin); }
	fprintf(cfile, "#seg_thickness=%d\n", _d_seg_thickness);
	if (seg_thickness != _d_seg_thickness){ fprintf(cfile, "seg_thickness=%d\n", seg_thickness); }
	fprintf(cfile, "#seg_spacing=%d\n", _d_seg_spacing);
	if (seg_spacing != _d_seg_spacing){ fprintf(cfile, "seg_spacing=%d\n", seg_spacing); }

	fputs("\n# Slider colorscheme\n", cfile);
	nftoh(_d_background_color, tmpstr1);
	nftoh(background_color, tmpstr2);
	fprintf(cfile, "#background_color=%s\n", tmpstr1);
	if (strcmp(tmpstr1, tmpstr2) != 0){ fprintf(cfile, "background_color=%s\n", tmpstr2); }
	nftoh(_d_border_color, tmpstr1);
	nftoh(border_color, tmpstr2);
	fprintf(cfile, "#border_color=%s\n", tmpstr1);
	if (strcmp(tmpstr1, tmpstr2) != 0){ fprintf(cfile, "border_color=%s\n", tmpstr2); }
	nftoh(_d_unlit_color, tmpstr1);
	nftoh(unlit_color, tmpstr2);
	fprintf(cfile, "#unlit_color=%s\n", tmpstr1);
	if (strcmp(tmpstr1, tmpstr2) != 0){ fprintf(cfile, "unlit_color=%s\n", tmpstr2); }
	nftoh(_d_lit_color, tmpstr1);
	nftoh(lit_color, tmpstr2);
	fprintf(cfile, "#lit_color=%s\n", tmpstr1);
	if (strcmp(tmpstr1, tmpstr2) != 0){ fprintf(cfile, "lit_color=%s\n", tmpstr2); }

	fputs("\n# Enable the tray_icon\n", cfile);
	fprintf(cfile, "#enable_tray_icon=%d\n", _d_enable_tray_icon);
	if (enable_tray_icon != _d_enable_tray_icon){ fprintf(cfile, "enable_tray_icon=%d\n", enable_tray_icon); }

	fputs("\n# Background color of tray_icon (default is default GTK background color).  Note:  the commandline -bg option overrides this\n", cfile);
	nftoh(_d_tray_icon_background_color, tmpstr1);
	nftoh(tray_icon_background_color, tmpstr2);
	fprintf(cfile, "#tray_icon_background_color=%s\n", tmpstr1);
	if (enable_tray_icon_background_color){ fprintf(cfile, "tray_icon_background_color=%s\n", tmpstr2); }

	fputs("\n# Set this to 1 to make the slider on the tray_icon vertical, or 0 for horizontal\n", cfile);
	fprintf(cfile, "#tray_slider_vertical=%d\n", _d_tray_slider_vertical);
	if (tray_slider_vertical != _d_tray_slider_vertical){ fprintf(cfile, "tray_slider_vertical=%d\n", tray_slider_vertical); }

	fputs("\n# Tray slider dimensions\n", cfile);
	fprintf(cfile, "#tray_slider_width=%d\n", _d_tray_slider_width);
	if (tray_slider_width != _d_tray_slider_width){ fprintf(cfile, "tray_slider_width=%d\n", tray_slider_width); }
	fprintf(cfile, "#tray_slider_height=%d\n", _d_tray_slider_height);
	if (tray_slider_height != _d_tray_slider_height){ fprintf(cfile, "tray_slider_height=%d\n", tray_slider_height); }
	
	fputs("\n# Which slider to link with the tray_icon\n", cfile);
	fprintf(cfile, "#tray_control=%s\n", "Master Playback Volume");
	if (strlen(tray_control_name) > 0 && strcmp(tray_control_name, "Master Playback Volume") != 0){ fprintf(cfile, "tray_control=%s\n", tray_control_name); }

	fputs("\n\n", cfile);
	fputs("\n# Which sliders to display, in order.  They MUST have a tab first and be quoted\n# with double-quotes.  To get a list of the slider names, run this command:\n#    amixer controls\n# NOTE:  This section must go at the end of the file!\n", cfile);

	fputs("\n#EXAMPLE:\n", cfile);
	fputs("\n#sliders:\n", cfile);
	fputs("#\t\"Master Playback Volume\"\n", cfile);
	fputs("#\t\"Front Playback Volume\"\n", cfile);
	fputs("#\t\"Surround Playback Volume\"\n", cfile);
	fputs("\nsliders:\n", cfile);
	for (int n=0; n<num_names; n++){
		fprintf(cfile, "\t\"%s\"\n", name_list[n]);
	}

	fclose(cfile);
	
}

//reorder the items in list to match name_list, omitting any that are not in name_list
void ConfigSettings::reorder_list(ElementList *list){
	list_ptr = list;
	//this function is not needed unless an order has been defined somewhere
	if (num_names!=0){
		
		int *order = new int[list_ptr->num_items];

		//find the indexes
		int k = 0;
		for (int n=0; n<num_names; n++){
			for (int i=0; i < list_ptr->num_items; i++){
				if (strcmp(name_list[n], list->items[i]->name) == 0){
					order[k++]=i;
					break;
				}
			}
		}

		num_names = k;
		list_ptr->reorder_items(order, num_names);
		delete order;
		
		//update the name_list to omit nonexistent items
		list_ptr->list_my_names(name_list);
	}
	
}


//look through the list and set the tray_slider_control to the matching element
void ConfigSettings::set_tray_slider(ElementList *list){
	//don't bother searching unless a name has been specified
	if (strlen(tray_control_name) != 0){
		for(int i=0; i<list->num_elems; i++){
			if (strcmp(list->elems[i].name, tray_control_name) == 0){
				tray_control = &(list->elems[i]);
				break;
			}
		}
	}
	//if the name was not specified, or was but was not found, then iterate through
	//the search_list to find one.
	char search_list[5][80] = {"Master Playback Volume", "PCM Playback Volume", "Front Playback Volume", "Playback Volume", "Volume"};
	int attempt = 0;
	while(attempt < 5 && (strlen(tray_control_name)==0 || !tray_control)){
		for(int i=0; i<list->num_elems; i++){
			if (strstr(list->elems[i].name, search_list[attempt])){
				tray_control = &(list->elems[i]);
				strcpy(tray_control_name, list->elems[i].name);
				break;
			}
		}
		attempt++;
	}
	if(strlen(tray_control_name)==0 || !tray_control){
		tray_control = &(list->elems[0]);
		strcpy(tray_control_name, list->elems[0].name);
	}
}


//take a hex string like #AAFF88 and put it into a three item integer array
void ConfigSettings::htoi(int *array, char *string){
	if (strlen(string) == 7 ){
		for (int i=0; i<3; i++){
			if (string[1+2*i] >= '0' && string[1+2*i] <= '9'){
				array[i]=16*(string[1+2*i]-'0');
			} else if (string[1+2*i] >= 'A' && string[1+2*i] <= 'F'){
				array[i]=16*(string[1+2*i]-'A'+10);
			} else if (string[1+2*i] >= 'a' && string[1+2*i] <= 'f'){
				array[i]=16*(string[1+2*i]-'a'+10);
			} else {
				array[i]=0;
			}
			if (string[2+2*i] >= '0' && string[2+2*i] <= '9'){
				array[i]+=(string[2+2*i]-'0');
			} else if (string[2+2*i] >= 'A' && string[2+2*i] <= 'F'){
				array[i]+=(string[2+2*i]-'A'+10);
			} else if (string[2+2*i] >= 'a' && string[2+2*i] <= 'f'){
				array[i]+=(string[2+2*i]-'a'+10);
			} else {
				array[i]+=0;
			}
		}
	} else {
		array[0]=array[1]=array[2]=0;
	}
}

//take a hex string like #AAFF88 and put it into a three item float array, normalized so 255=1.0, 0=0.0
void ConfigSettings::htonf(float *array, char *string){
	int intarray[3];
	htoi(intarray, string);
	for (int i=0; i<3; i++){
		array[i]=((float)intarray[i])/255;
	}
}

//take a 3 item integer array and convert it into a hex string like #AAFF88
void ConfigSettings::itoh(int *array, char *string){
	string[0] = '#';
	for (int i=0; i<3; i++){
		int tmp = array[i]/16;
		if (tmp >= 10 && tmp <= 15){
			string[1+2*i] = 'A' + tmp - 10;
		} else if (tmp >= 0 && tmp <= 9){
			string[1+2*i] = '0' + tmp;
		} else {
			string[1+2*i] = '0';
		}
		tmp = array[i]%16;
		if (tmp >= 10 && tmp <= 15){
			string[2+2*i] = 'A' + tmp - 10;
		} else if (tmp >= 0 && tmp <= 9){
			string[2+2*i] = '0' + tmp;
		} else {
			string[2+2*i] = '0';
		}
	}
	string[7] = '\0';
}

//take a 3 item normalized float array and convert it into a hex string like #AAFF88
void ConfigSettings::nftoh(float *array, char *string){
	int intarray[3];
	for (int i=0; i<3; i++){
		intarray[i]=(int)(array[i]*255);
	}
	itoh(intarray, string);
}

//take a 3 item normalized float array and convert it into a GdkColor
void ConfigSettings::nftog(float *array, GdkColor *color){
	color->red=(guint16)(array[0]*65535);
	color->green=(guint16)(array[1]*65535);
	color->blue=(guint16)(array[2]*65535);
}

//take a GdkColor and convert it into a 3 item normalized float array
void ConfigSettings::gtonf(float *array, GdkColor *color){
	array[0]=((float)color->red)/65535;
	array[1]=((float)color->green)/65535;
	array[2]=((float)color->blue)/65535;
}




