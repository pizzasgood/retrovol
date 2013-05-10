/* alsa_classes.cpp */
/* This code is part of the Public Domain. */
/*
 This file contains classes to load and manipulate the mixer settings through
 alsa using asoundlib.h.  Numbers are automatically scaled to a 0-100 range
 to simplify use with volume sliders.
 
 created 2008-09-30 by pizzasgood
*/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <alsa/asoundlib.h>

#include "alsa_classes.h"

//i18n stuff
#include "gettext.h"
#include <locale.h>
#define _(String) gettext (String)


//Constructor: automatically populates the element with its non-dynamic data
Element::Element(char *_card, int _numid, const char *_name){
	//do the easy stuff first
	card = _card;
	numid = _numid;
	strcpy(name, _name);
	switch_id = -1; //this will be changed later if there is an associated switch
	scaling = LINEAR; //this will be set later
	auto_mute = true; //this can be set later
	associated = false; //this one is also handled later
	
	//set the short name by dropping any trailing "Playback*", "Volume*", or "Switch*"
	strcpy(short_name, name);
	char *space = strstr(short_name, " Playback");
	if (!space){
		space = strstr(short_name, " Volume");
	}
	if (!space){
		space = strstr(short_name, " Switch");
	}
	if (space){
		space[0]='\0';
	}
	
	//if there is a trailing hyphen, get rid of it
	if (short_name[strlen(short_name)-2] == ' ' && short_name[strlen(short_name)-1] == '-'){
		short_name[strlen(short_name)-2]='\0';
	}
	
	//also abbrieviate some things
	strrep(short_name, (char *)"Source", (char *)"So.");
	strrep(short_name, (char *)"Channel Mode", (char *)"Channel");
	strrep(short_name, (char *)"Digital Capture", (char *)"Digital");
	
	
	//some needed pointers
	int err;
	snd_ctl_t *handle;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_value_t *control;
	
	//allocate the items
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_value_alloca(&control);
	

	//set the interface and numid to grab the device which we're interested in
	snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
	snd_ctl_elem_id_set_numid(id, numid);
	//use the id to set the control
	snd_ctl_elem_value_set_id(control, id);
	
	//open a handle to use with the card
	if ((err = snd_ctl_open(&handle, card, 0)) < 0) {
		fprintf(stderr, _("Control %s open error: %s\n"), card, snd_strerror(err));
	}
	
	snd_ctl_elem_read(handle, control);
	
	//use the id to set the info
	snd_ctl_elem_info_set_id(info, id);
	//hook it up with the handle
	snd_ctl_elem_info(handle, info);
	//get the info
	snd_ctl_elem_info_get_id(info, id);
	
	//set the index of the element (generally 0 except stuff like 'capture 0, capture 1, capture 2')
	index = snd_ctl_elem_id_get_index(id);
	//update the names if index > 0
	if (index > 0){
		strcat(short_name, " ");
		short_name[strlen(short_name)-1] = '0' + index;
	}

	//set the number of values the element has (generally will be 2, for left and right)
	values = snd_ctl_elem_info_get_count(info);

	//set what iface the element uses
	snd_ctl_elem_iface_t interface = snd_ctl_elem_id_get_interface(id);
	strncpy(iface, snd_ctl_elem_iface_name(interface), 16);
	iface[15] = '\0';
	
	//set what datatype the element holds
	strcpy(type, snd_ctl_elem_type_name(snd_ctl_elem_info_get_type(info)));
	
	//only use max and min if it's an integer, otherwise set to 0
	if (strcmp(type, "INTEGER") == 0){
		min = snd_ctl_elem_info_get_min(info);
		max = snd_ctl_elem_info_get_max(info);
	} else {
		min = max = 0;
	}
	
	
	//if it's an enumerated, get a list of the types it has
	if (strcmp(type, "ENUMERATED") == 0){
		number_of_enums = snd_ctl_elem_info_get_items(info);
		enums=(char**)malloc(number_of_enums*sizeof(char*));
		for(unsigned int i=0; i<number_of_enums; i++){
			//set which item in the enumerated list we're interested in
			snd_ctl_elem_info_set_item(info, i);
			//hook it up with the handle
			snd_ctl_elem_info(handle, info);
			enums[i]=(char*)malloc((strlen(snd_ctl_elem_info_get_item_name(info))+1)*sizeof(char));
			strcpy(enums[i], snd_ctl_elem_info_get_item_name(info));
		}
	} else {
		number_of_enums=0;
		enums=NULL;
	}
	
	
	
	
	
	
	//don't need the handle open anymore, so close it
	snd_ctl_close(handle);
}


//this is mainly for debugging; it prints data about the element to the console
void Element::print(){
	printf("numid: %d  name: %s\n", numid, name);
	printf("type: %s, values: %d\n", type, values);
	printf("min: %d, max: %d\n", min, max);
	printf("value: %d", _get(0));
	for (int i=1; i<values; i++){
		printf(", %d", _get(i));
	}
	printf("\n");
}


//this is used internally to scale a number to be from 0-100
int Element::scale_out(int num){
	if(max-min==0){ return(num); }
	switch (scaling){
		case LOGARITHMIC:
			return(round(pow(101.0, (num-min)/(double)(max-min)))-1);
			break;
		case EXPONENTIAL:
			return(round(100.0*log((num-min)/(double)(max-min)+1)/log(2)));
			break;
		case LINEAR:
		default:
			return(ceil(100.0*(num-min)/(max-min)));
			break;
	}
}
//this is the inverse of scale_out; it's used to take a 0-100 number and put it
//into the proper scale for the element to understand
int Element::scale_in(int num){
	if(max-min==0){ return(num); }
	switch (scaling){
		case LOGARITHMIC:
			return(round((log(num+1)/log(101)*(max-min))+min));
			break;
		case EXPONENTIAL:
			return(round((pow(2.0, num/(double)100)-1)*(max-min)+min));
			break;
		case LINEAR:
		default:
			return(floor((num*(max-min)/(100))+min));
			break;
	}
}
//this will grab the highest value in the element
int Element::get(){
	int ret = scale_out(_get(0));
	for (int i=1; i<values; i++){
		if(ret < scale_out(_get(i))){ ret = scale_out(_get(i)); }
	}
	return(ret);
}
//this gets the value of value n
int Element::get(int n){
	return(scale_out(_get(n)));
}

//this is a shorthand for setting values 0 and 1 to l and r
void Element::set_lr(int l, int r){
	set(l, 0);
	set(r, 1);
}
//this sets all values to num
int Element::set(int num){
	int ret=0;
	for (int i=0; i<values; i++){
		ret = set(num, i);
	}
	return(ret);
}
//this sets value n to num
int Element::set(int num, int n){
	return(scale_out(_set(scale_in(num), n)));
}

//these are callback functions for get() and set(), so they can be used with function pointers
float Element::get_callback(void *obj){
	return( (float)( ((Element*)obj)->get() ) );
}
float Element::get_callback(void *obj, int n){
	return( (float)( ((Element*)obj)->get(n) ) );
}
float Element::set_callback(void *obj, float num){
	return( (float)( ((Element*)obj)->set((int)num) ) );
}
float Element::set_callback(void *obj, float num, int n){
	return( (float)( ((Element*)obj)->set((int)num, n) ) );
}
		
//this is used internally to get the unscaled value of value n
int Element::_get(int n){
	//if (n >= values || strcmp(type, "INTEGER") != 0){
	if (n >= values){
		return(0);
	}
	
	int err;
	snd_ctl_t *handle;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_value_t *control;
	
	//allocate the items
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_value_alloca(&control);
	

	//set the interface and numid to grab the device which we're interested in
	snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
	snd_ctl_elem_id_set_numid(id, numid);
	//use the id to set the control
	snd_ctl_elem_value_set_id(control, id);
	
	//open a handle to use with the card
	if ((err = snd_ctl_open(&handle, card, 0)) < 0) {
		fprintf(stderr, _("Control %s open error: %s\n"), card, snd_strerror(err));
	}
	
	snd_ctl_elem_read(handle, control);
	
	//grab the value - note: you could get away with not checking the type and just using the get_integer one
	int ret;
	if (strcmp(type, "INTEGER") == 0){
		ret = (int)snd_ctl_elem_value_get_integer(control, n);
	} else if (strcmp(type, "ENUMERATED") == 0){
		ret = (int)snd_ctl_elem_value_get_enumerated(control, n);
	} else if (strcmp(type, "BYTE") == 0){
		ret = (int)snd_ctl_elem_value_get_byte(control, n);
	} else if (strcmp(type, "BOOLEAN") == 0){
		ret = (int)snd_ctl_elem_value_get_boolean(control, n);
	} else {
		ret = (int)snd_ctl_elem_value_get_integer(control, n);
	}
	
	//don't need the handle open anymore, so close it
	snd_ctl_close(handle);
	
	return(ret);
	
}

//this is used internally to get the string value from an enumerated list
void Element::sget(char *ret){
	
	int err;
	snd_ctl_t *handle;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_value_t *control;
	
	//allocate the items
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_value_alloca(&control);
	

	//set the interface and numid to grab the device which we're interested in
	snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
	snd_ctl_elem_id_set_numid(id, numid);
	//use the id to set the control
	snd_ctl_elem_value_set_id(control, id);
	
	//open a handle to use with the card
	if ((err = snd_ctl_open(&handle, card, 0)) < 0) {
		fprintf(stderr, _("Control %s open error: %s\n"), card, snd_strerror(err));
	}
	
	snd_ctl_elem_read(handle, control);
	
	//grab the value
	if (strcmp(type, "ENUMERATED") == 0){
		//use the id to set the info
		snd_ctl_elem_info_set_id(info, id);
		//set which item in the enumerated list we're interested in
		snd_ctl_elem_info_set_item(info, snd_ctl_elem_value_get_enumerated(control, 0));
		//hook it up with the handle
		snd_ctl_elem_info(handle, info);
		strcpy(ret,snd_ctl_elem_info_get_item_name(info));
	} else {
		sprintf(ret, "%ld", snd_ctl_elem_value_get_integer(control, 0));
	}
	
	//don't need the handle open anymore, so close it
	snd_ctl_close(handle);
	
}

//this is used internally to set value n to num (unscaled)
int Element::_set(int num, int n){
	
	int err;
	snd_ctl_t *handle;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_value_t *control;
	
	//allocate the items
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_value_alloca(&control);
	

	//set the interface and numid to grab the device which we're interested in
	snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
	snd_ctl_elem_id_set_numid(id, numid);
	//use the id to set the control
	snd_ctl_elem_value_set_id(control, id);
	
	//open a handle to use with the card
	if ((err = snd_ctl_open(&handle, card, 0)) < 0) {
		fprintf(stderr, _("Control %s open error: %s\n"), card, snd_strerror(err));
	}
	
	
	//set the values
	for (int i=0; i<values; i++){
		if(i==n){
			if (strcmp(type, "INTEGER") == 0){
				snd_ctl_elem_value_set_integer(control, i, num);
			} else if (strcmp(type, "ENUMERATED") == 0){
				snd_ctl_elem_value_set_enumerated(control, i, num);
			} else if (strcmp(type, "BYTE") == 0){
				snd_ctl_elem_value_set_byte(control, i, num);
			} else if (strcmp(type, "BOOLEAN") == 0){
				snd_ctl_elem_value_set_boolean(control, i, num);
			} else {
				snd_ctl_elem_value_set_integer(control, i, num);
			}
		} else {
			//when we set 'n', it resets the other values, so let's be sure to preserve them
			if (strcmp(type, "INTEGER") == 0){
				snd_ctl_elem_value_set_integer(control, i, _get(i));
			} else if (strcmp(type, "ENUMERATED") == 0){
				snd_ctl_elem_value_set_enumerated(control, i, _get(i));
			} else if (strcmp(type, "BYTE") == 0){
				snd_ctl_elem_value_set_byte(control, i, _get(i));
			} else if (strcmp(type, "BOOLEAN") == 0){
				snd_ctl_elem_value_set_boolean(control, i, _get(i));
			} else {
				snd_ctl_elem_value_set_integer(control, i, _get(i));
			}
		}
	}

	
	//make the change
	snd_ctl_elem_write(handle, control);
	
	//don't need the handle open anymore, so close it
	snd_ctl_close(handle);


	//now, if auto_mute is set and this has a switch, handle that
	if (auto_mute && strcmp(type, "INTEGER") == 0 && switch_ptr){
		if (num == 0){
			switch_ptr->set(0);
		} else {
			switch_ptr->set(1);
		}
	}
	
	return(num);
	
}







//Constructor: automatically creates a list of the elments in _card, and also
//calls each element's constructor, causing the list to be populated as it is
//built.
ElementList::ElementList(char *_card){
	strcpy(card, _card);
	
	num_elems = 0;
	elems = NULL;
	num_items = 0;
	items = NULL;
	
	//open a pointer to use
	int error = 0;
	snd_hctl_t *hctl;
	error = snd_hctl_open(&hctl, card, SND_CTL_NONBLOCK | SND_CTL_ASYNC);
	if (error){
		fprintf(stderr, "Unable to open card %s, snd_hctl_open returned %d\n", card, error);
		return;
	}
	error = snd_hctl_load(hctl);
	if (error){
		fprintf(stderr, "Unable to load control for card %s, snd_hctl_load returned %d\n", card, error);
		return;
	}
	
	//get the total number and allocate the array
	num_elems = snd_hctl_get_count(hctl);
	elems = (Element*)malloc(num_elems*sizeof(Element));
	
	//loop through and store the numid and name for each element in list->elems
	snd_hctl_elem_t *one_elem = snd_hctl_first_elem(hctl);
	int k=0;
	for (int i=0; i++<num_elems; one_elem=snd_hctl_elem_next(one_elem)){
		//want to use the constructor
		Element *tmpptr = new Element(card, snd_hctl_elem_get_numid(one_elem), snd_hctl_elem_get_name(one_elem));
		//let's only bother with elements that have the MIXER iface
		if(strstr(tmpptr->iface, "MIXER")){
			elems[k] = *tmpptr;
			k++;
		}
		delete(tmpptr);
		
	}
	num_elems = k;

	//want to track down all "Switch" elements and look to see if they have an associated value elsewhere
	for (int i=0; i<num_elems; i++){
		if (strstr(elems[i].name, "Switch")){
			//okay, we need to grab the name, replace the 'Switch' with a 'Volume', and loop through to
			//find any elements with that as the first portion of their name
			char buffer[256];
			strncpy(buffer, elems[i].name, strlen(elems[i].name) - 6);
			buffer[strlen(elems[i].name) - 6] = '\0';
			strcat(buffer, "Volume");
			for (int j=0; j<num_elems; j++){
				if (strstr(elems[j].name, buffer) && !strstr(elems[j].name, "Switch") && elems[j].index == elems[i].index){
					elems[j].switch_id = i;
					elems[j].switch_ptr = &(elems[i]);
					elems[j].associated = elems[i].associated = true;
				}
			}
			
		}
	}
	
	//clean up
	snd_hctl_free(hctl);
	snd_hctl_close(hctl);
	
	populate_items();
	
}

int ElementList::list_my_names(char list[][256]){
	for(int i=0; i<num_items; i++){
		strcpy(list[i], items[i]->name);
	}
	return(num_items);
}

int ElementList::list_my_numids(int list[]){
	for(int i=0; i<num_items; i++){
		list[i]=items[i]->numid;
	}
	return(num_items);
}

int ElementList::list_all_names(char list[][256]){
	int k=0;
	for(int n=0; n<num_elems; n++){
		if (!strstr(elems[n].name, "Switch") || (!elems[n].associated && strstr(elems[n].name, "Switch"))){
			strcpy(list[k++], elems[n].name);
		}
	}
	return(k);
}

int ElementList::list_all_numids(int list[]){
	int k=0;
	for(int n=0; n<num_elems; n++){
		if (!strstr(elems[n].name, "Switch") || (!elems[n].associated && strstr(elems[n].name, "Switch"))){
			list[k++]=elems[n].numid;
		}
	}
	return(k);
}

int ElementList::list_other_names(char list[][256]){
	int k=0;
	for(int n=0; n<num_elems; n++){
		bool other = true;
		for(int i=0; i<num_items; i++){
			if(items[i]->numid == elems[n].numid){
				other = false;
				break;
			}
		}
		if (other && (!strstr(elems[n].name, "Switch") || (!elems[n].associated && strstr(elems[n].name, "Switch")))){
			strcpy(list[k++], elems[n].name);
		}
	}
	return(k);
}

int ElementList::list_other_numids(int list[]){
	int k=0;
	for(int n=0; n<num_elems; n++){
		bool other = true;
		for(int i=0; i<num_items; i++){
			if(items[i]->numid == elems[n].numid){
				other = false;
				break;
			}
		}
		if (other && (!strstr(elems[n].name, "Switch") || (!elems[n].associated && strstr(elems[n].name, "Switch")))){
			list[k++]=elems[n].numid;
		}
	}
	return(k);
}

int ElementList::list_all_int_names(char list[][256]){
	int k=0;
	for(int n=0; n<num_elems; n++){
		if (strcmp("INTEGER", elems[n].type) == 0){
			strcpy(list[k++], elems[n].name);
		}
	}
	return(k);
}

int ElementList::list_all_int_numids(int list[]){
	int k=0;
	for(int n=0; n<num_elems; n++){
		if (strcmp("INTEGER", elems[n].type) == 0){
			list[k++]=elems[n].numid;
		}
	}
	return(k);
}

void ElementList::populate_items(){
	num_items=0;
	//Since there aren't that many elements, I figure it's more efficient to just
	//allocate enough space for every element now, and then later realloc it to
	//shrink down to the needed size.  As opposed to constantly reallocing as we
	//go, or looping through here twice to find out in advance how large the array
	//will be.
	items = (Element**)malloc(num_elems * sizeof(Element*));
	
	for (int i=0; i<num_elems; i++){
		if (!elems[i].associated && strstr(elems[i].name, "Switch")){
			items[num_items++] = &elems[i];
		}
	}
	for (int i=0; i<num_elems; i++){
		if (elems[i].switch_id >= 0 && strstr(elems[i].name, "Master Playback Volume")){
			items[num_items++] = &elems[i];
		}
	}
	for (int i=0; i<num_elems; i++){
		if (!elems[i].associated && strstr(elems[i].name, "Playback Volume")){
			items[num_items++] = &elems[i];
		}
	}
	for (int i=0; i<num_elems; i++){
		if (elems[i].switch_id >= 0 && strstr(elems[i].name, "Playback Volume") && !strstr(elems[i].name, "Master Playback Volume")){
			items[num_items++] = &elems[i];
		}
	}
	for (int i=0; i<num_elems; i++){
		if (elems[i].switch_id >= 0 && !strstr(elems[i].name, "Playback Volume") && !strstr(elems[i].name, "Capture Volume") && !strstr(elems[i].name, "IEC958")){
			items[num_items++] = &elems[i];
		}
	}
	for (int i=0; i<num_elems; i++){
		if (elems[i].switch_id >= 0 && !strstr(elems[i].name, "Playback Volume") && strstr(elems[i].name, "Capture Volume")){
			items[num_items++] = &elems[i];
		}
	}
	for (int i=0; i<num_elems; i++){
		if (!elems[i].associated && !(strstr(elems[i].name, "Switch") || strstr(elems[i].name, "Playback Volume") || strstr(elems[i].name, "IEC958"))){
			items[num_items++] = &elems[i];
		}
	}
	
	items = (Element**)realloc(items, num_items * sizeof(Element*));
}


//rearranges the items array so the current indexes are reordered to match 'order'
void ElementList::reorder_items(int *order, int n){
	num_items=n;
	Element **tmp_items = (Element**)malloc(num_items * sizeof(Element*));
	for (int i=0; i<num_items; i++){
		tmp_items[i]=items[order[i]];
	}
	free(items);
	items=tmp_items;
	tmp_items=NULL;
}


//updates the scale for all elements
void ElementList::set_scale(Element::scale_t s){
	for (int i=0; i<num_elems; i++){
		elems[i].scaling = s;
	}
}


//updates the auto_mute for all elements
void ElementList::set_auto_mute(bool a){
	for (int i=0; i<num_elems; i++){
		elems[i].auto_mute = a;
	}
}


//replaces any instance of 'oldstr' found in dest with newstr, and returns true if dest was modified
bool strrep(char *dest, char *oldstr, char *newstr){
	if (char *start = strstr(dest, oldstr)){
		if (start[strlen(oldstr)]=='\0'){
			strcpy(start, newstr);
		} else {
			strncpy(start, newstr, strlen(newstr));
			strcpy(start+strlen(newstr), start+strlen(oldstr));
			strrep(dest, oldstr, newstr); //handle multiple instances of oldstr
		}
		return true;
	} else {
		return false;
	}
}
