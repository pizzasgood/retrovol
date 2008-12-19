/* main.cpp */

/*
    retrovol - a retro-styled volume mixer, by Pizzasgood
*/

#include <stdlib.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <string.h>
#include "retro_slider.h"
#include "alsa_classes.h"
#include "main.h"



static ConfigSetttings settings;
char config_file[] = "/root/.retrovolrc"; //CHANGE this to use the home-dir!


//callback that handles muting/unmuting a control
void toggle_it(GtkWidget *chkbx, Element *elem){
	bool state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkbx));
	elem->set((int)state);
}


//replace the second space (or the first if preceeding a '(' or '-') with '\n'
//else append a '\n' to keep things lined up
void word_wrap(char *wrapped, char *orig){
	strcpy(wrapped, orig);
	unsigned int i,n;
	for (i=0, n=0; i<strlen(wrapped); i++){
		if (wrapped[i] == ' '){
			n++;
			if (n==2 || (n==1 && (wrapped[i+1] == '(' || wrapped[i+1] == '-'))){
				n=2;
				wrapped[i]='\n';
				while(wrapped[i+1] == '-' || wrapped[i+1] == ' '){
					strcpy(&wrapped[i+1], &wrapped[i+2]);
				}
				break;
			}
		}
	}
	if (n<2){
		wrapped[i]='\n';
		wrapped[i+1]='\0';
	}
}




ConfigSetttings::ConfigSetttings(){
	num_names = 0;
	//defaults
	strcpy(card, "hw:0");
	vertical = true;
	window_width=480;
	window_height=180;
	slider_width=20;
	slider_height=102;
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









int main(int argc, char** argv) {
	//parse the config file
	settings.parse_config(config_file);
	//load the controls into list
	ElementList list(settings.card);
	//reorder the controls to the order specified in the config file
	settings.reorder_list(&list);
	
	//set up the window
	GtkWidget *window;
	gtk_init(&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), settings.window_width, settings.window_height);
	gtk_window_set_title(GTK_WINDOW(window), "Retrovol");
	
	//use a scrolled window
	GtkWidget *scrolled_window;
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy((GtkScrolledWindow*)scrolled_window, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(window), scrolled_window);
	
	//put the stuff into a viewport manually, so we can specify that it should have no shadow
	GtkWidget *viewport;
	viewport = gtk_viewport_new(NULL, NULL);
	gtk_viewport_set_shadow_type(GTK_VIEWPORT(viewport), GTK_SHADOW_NONE);
	gtk_container_add(GTK_CONTAINER(scrolled_window), viewport);
	
			
	//and create an Hbox to hold all the stuff
	GtkWidget *hbox;
	if (settings.vertical){
		hbox = gtk_hbox_new(TRUE, 2);
		gtk_container_add(GTK_CONTAINER(viewport), hbox);
	} else {
		hbox = gtk_vbox_new(TRUE, 2);
		gtk_container_add(GTK_CONTAINER(viewport), hbox);
	}
			
	//add the sliders
	retro_slider *sliders = new retro_slider[list.num_items];
	
	for(int i=0; i<list.num_items; i++){
		//use a vbox w/ slider on top and label on bottom
		GtkWidget *vbox;
		if (settings.vertical){
			vbox = gtk_vbox_new(FALSE, 2);
		} else {
			vbox = gtk_hbox_new(FALSE, 2);
		}
		gtk_box_pack_start(GTK_BOX(hbox), vbox, false, false, 0);
		
		if (strcmp(list.items[i]->type, "INTEGER") == 0){
			//integers need sliders
			//the rslider pseudo-widget likes to be inside a container, lets use a GtkAlignment
			GtkWidget *frame;
			if (settings.vertical){
				frame = gtk_alignment_new(0.5,0.0,0,0);
				gtk_box_pack_start(GTK_BOX(vbox), frame, false, false, 0);
			} else {
				frame = gtk_alignment_new(0.0,0.5,0,0);
				gtk_box_pack_end(GTK_BOX(vbox), frame, false, false, 0);
			}
			//make the slider and associate with a control
			settings.apply_to_slider(&sliders[i]);
			sliders[i].init(frame, (void*)list.items[i], &Element::get_callback, &Element::set_callback);
		
		} else if (strcmp(list.items[i]->type, "BOOLEAN") == 0){
			//booleans need checkboxes
			GtkWidget *alignment;
			if (settings.vertical){
				alignment = gtk_alignment_new(0.5,1.0,0,0);
				gtk_box_pack_start(GTK_BOX(vbox), alignment, true, true, 0);
			} else {
				alignment = gtk_alignment_new(1.0,0.5,0,0);
				gtk_box_pack_end(GTK_BOX(vbox), alignment, true, true, 0);
			}
			GtkWidget *chkbx;
			chkbx = gtk_check_button_new();
			//set it to the current state
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkbx), (bool)list.items[i]->get());
			//bind to the toggle_it function
			g_signal_connect (GTK_TOGGLE_BUTTON(chkbx), "toggled", G_CALLBACK (toggle_it), list.items[i]);
			gtk_container_add(GTK_CONTAINER(alignment), chkbx);
		} else if (strcmp(list.items[i]->type, "ENUMERATED") == 0){
			//tempory stuff - put label for enumerated
			GtkWidget *alignment;
			if (settings.vertical){
				alignment = gtk_alignment_new(0.5,0.5,0,0);
				gtk_box_pack_start(GTK_BOX(vbox), alignment, true, true, 0);
			} else {
				alignment = gtk_alignment_new(0.5,0.5,0,0);
				gtk_box_pack_end(GTK_BOX(vbox), alignment, true, true, 0);
			}
			GtkWidget *label;
			char text[16];
			list.items[i]->sget(text);
			label = gtk_label_new(text);
			gtk_container_add(GTK_CONTAINER(alignment), label);
		}
		
		//add a checkbox for sliders that are muteable
		if (list.items[i]->switch_id >= 0){
			GtkWidget *alignment;
			if (settings.vertical){
				alignment = gtk_alignment_new(0.5,1.0,0,0);
				gtk_box_pack_start(GTK_BOX(vbox), alignment, true, true, 0);
			} else {
				alignment = gtk_alignment_new(1.0,0.5,0,0);
				gtk_box_pack_end(GTK_BOX(vbox), alignment, true, true, 0);
			}
			GtkWidget *chkbx;
			chkbx = gtk_check_button_new();
			//set it to the current state
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkbx), (bool)list.elems[list.items[i]->switch_id].get());
			//bind to the toggle_it function
			g_signal_connect (GTK_TOGGLE_BUTTON(chkbx), "toggled", G_CALLBACK (toggle_it), &(list.elems[list.items[i]->switch_id]));
			gtk_container_add(GTK_CONTAINER(alignment), chkbx);
		}
		
		//display the name of the control
		GtkWidget *alignment;
		char wrapped[80];
		if (settings.vertical){
			alignment = gtk_alignment_new(0.5,1.0,0,0);
			gtk_box_pack_end(GTK_BOX(vbox), alignment, false, false, 0);
			word_wrap(wrapped, list.items[i]->short_name);
		} else {
			alignment = gtk_alignment_new(1.0,0.5,0,0);
			gtk_box_pack_start(GTK_BOX(vbox), alignment, false, false, 0);
			strcpy(wrapped, list.items[i]->short_name);
		}
		GtkWidget *label;
		label = gtk_label_new(wrapped);
		gtk_container_add(GTK_CONTAINER(alignment), label);
	}
	
	
	//finish the gtk stuff
	gtk_widget_show_all(window);
	g_signal_connect(window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
	
	gtk_main();
	
	return(0);
}
