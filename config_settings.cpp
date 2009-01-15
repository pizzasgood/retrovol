/* config_settings.cpp */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h> //this is needed due to retro
#include "retro_slider.h"
#include "alsa_classes.h"
#include "config_settings.h"


ConfigSetttings::ConfigSetttings(){
	num_names = 0;
	//defaults
	strcpy(card, "hw:0");
	//vertical = true;
	vertical = false;
	//window_width=480;
	//window_height=180;
	window_width=256;
	window_height=256;
	//slider_width=20;
	//slider_height=102;
	slider_width=102;
	slider_height=20;
	slider_margin = 2;
	seg_thickness = 2;
	seg_spacing = 1;
	
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


//apply settings to a slider
void ConfigSetttings::apply_to_slider(retro_slider *slider){
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


//parse the config file
void ConfigSetttings::parse_config(char *config_file){
	FILE *cfile = fopen(config_file, "r");	
	if (!cfile){
		fprintf(stdout, "Cannot read file: %s\nUsing defaults...\n", config_file);
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
		if (strcmp(tmpptr, "vertical")==0){
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


//reorder the items in list to match name_list, omitting any that are not in name_list
void ConfigSetttings::reorder_list(ElementList *list){
	
	//this function is not needed unless an order has been defined somewhere
	if (num_names!=0){
		
		int *order = new int[list->num_items];

		//find the indexes
		for (int n=0; n<num_names; n++){
			for (int i=0; i<list->num_items; i++){
				if (strcmp(name_list[n], list->items[i]->name) == 0){
					order[n]=i;
					break;
				}
			}
		}

		list->reorder_items(order, num_names);
		delete order;
		
	}
	
}


//take a hex string like #AAFF88 and put it into a three item integer array
void ConfigSetttings::htoi(int *array, char *string){
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
void ConfigSetttings::htonf(float *array, char *string){
	int intarray[3];
	htoi(intarray, string);
	for (int i=0; i<3; i++){
		array[i]=((float)intarray[i])/255;
	}
}

