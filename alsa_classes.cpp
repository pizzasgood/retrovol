/* alsa_classes.cpp */
/*
 This file contains classes to load and manipulate the mixer settings through
 alsa using asoundlib.h.  Numbers are automatically scaled to a 0-100 range
 to simplify use with volume sliders.
 
 created 2008-09-30 by pizzasgood
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <alsa/asoundlib.h>

#include "alsa_classes.h"


//Constructor: automatically populates the element with its non-dynamic data
Element::Element(char *_card, int _numid, const char *_name){
	//do the easy stuff first
	card = _card;
	numid = _numid;
	strcpy(name, _name);
	switch_id = -1; //this will be changed later if there is an associated switch
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
		fprintf(stderr, "Control %s open error: %s\n", card, snd_strerror(err));
	}
	
	snd_ctl_elem_read(handle, control);
	
	//use the id to set the info
	snd_ctl_elem_info_set_id(info, id);
	//hook it up with the handle
	snd_ctl_elem_info(handle, info);
	//get the info
	snd_ctl_elem_info_get_id(info, id);
	
	//set the number of values the element has (generally will be 2, for left and right)
	values = snd_ctl_elem_info_get_count(info);
	
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
	return(ceil(100.0*(num-min)/(max-min)));
}
//this is the inverse of scale_out; it's used to take a 0-100 number and put it
//into the proper scale for the element to understand
int Element::scale_in(int num){
	if(max-min==0){ return(num); }
	return(floor((num*(max-min)/(100))+min));
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
		fprintf(stderr, "Control %s open error: %s\n", card, snd_strerror(err));
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
		fprintf(stderr, "Control %s open error: %s\n", card, snd_strerror(err));
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
		fprintf(stderr, "Control %s open error: %s\n", card, snd_strerror(err));
	}
	
	
	//set the values
	for (int i=0; i<values; i++){
		if(i==n){
			snd_ctl_elem_value_set_integer(control, i, num);
		} else {
			//when we set 'n', it resets the other values, so let's be sure to preserve them
			snd_ctl_elem_value_set_integer(control, i, _get(i));
		}
	}
	
	//make the change
	snd_ctl_elem_write(handle, control);
	
	//don't need the handle open anymore, so close it
	snd_ctl_close(handle);
	
	return(num);
	
}







//Constructor: automatically creates a list of the elments in _card, and also
//calls each element's constructor, causing the list to be populated as it is
//built.
ElementList::ElementList(char *_card){
	strcpy(card, _card);
	
	
	//open a pointer to use
	snd_hctl_t *hctl;
	snd_hctl_open(&hctl, card, 0);
	snd_hctl_load(hctl);
	
	//get the total number and allocate the array
	num_elems = snd_hctl_get_count(hctl);
	elems = (Element*)malloc(num_elems*sizeof(Element));
	
	//loop through and store the numid and name for each element in list->elems
	snd_hctl_elem_t *one_elem = snd_hctl_first_elem(hctl);
	for (int i=0; i++<num_elems; one_elem=snd_hctl_elem_next(one_elem)){
		//want to use the constructor
		Element *tmpptr = new Element(card, snd_hctl_elem_get_numid(one_elem), snd_hctl_elem_get_name(one_elem));
		elems[i-1] = *tmpptr;
		delete(tmpptr);
		
	}

	//want to track down all "Switch" elements and look to see if they have an associated value elsewhere
	for (int i=0; i<num_elems; i++){
		if (strstr(elems[i].name, "Switch")){
			//okay, we need to grab the name, minus the "Switch", and loop through to
			//find any elements with that as the first portion of their name
			char buffer[80];
			strncpy(buffer, elems[i].name, strlen(elems[i].name) - 6);
			buffer[strlen(elems[i].name) - 6] = '\0';
			for (int j=0; j<num_elems; j++){
				if (strstr(elems[j].name, buffer) && !strstr(elems[j].name, "Switch")){
					elems[j].switch_id = i;
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
		if (elems[i].switch_id >= 0 && !strstr(elems[i].name, "Playback Volume") && !strstr(elems[i].name, "Capture Volume")){
			items[num_items++] = &elems[i];
		}
	}
	for (int i=0; i<num_elems; i++){
		if (elems[i].switch_id >= 0 && !strstr(elems[i].name, "Playback Volume") && strstr(elems[i].name, "Capture Volume")){
			items[num_items++] = &elems[i];
		}
	}
	for (int i=0; i<num_elems; i++){
		if (!elems[i].associated && !(strstr(elems[i].name, "Switch") || strstr(elems[i].name, "Playback Volume"))){
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


//this is just here for testing; used to be main()
int test_alsa_stuff(){
	char card[] = "hw:0";
	ElementList list(card);
	
	for (int i=0; i<list.num_elems; i++){
		if (!list.elems[i].associated && (strstr(list.elems[i].name, "Switch") || strstr(list.elems[i].name, "Playback Volume"))){
			printf("%d\t%d\t%s\n", list.elems[i].numid, list.elems[i].switch_id,list.elems[i].name);
		}
	}
	printf("\n");
	for (int i=0; i<list.num_elems; i++){
		if (list.elems[i].switch_id > 0 && strstr(list.elems[i].name, "Playback Volume")){
			printf("%d\t%d\t%s\n", list.elems[i].numid, list.elems[i].switch_id,list.elems[i].name);
		}
	}
	printf("\n");
	for (int i=0; i<list.num_elems; i++){
		if (list.elems[i].switch_id > 0 && !strstr(list.elems[i].name, "Playback Volume") && !strstr(list.elems[i].name, "Capture Volume")){
			printf("%d\t%d\t%s\n", list.elems[i].numid, list.elems[i].switch_id,list.elems[i].name);
		}
	}
	printf("\n");
	for (int i=0; i<list.num_elems; i++){
		if (list.elems[i].switch_id > 0 && !strstr(list.elems[i].name, "Playback Volume") && strstr(list.elems[i].name, "Capture Volume")){
			printf("%d\t%d\t%s\n", list.elems[i].numid, list.elems[i].switch_id,list.elems[i].name);
		}
	}
	printf("\n");
	for (int i=0; i<list.num_elems; i++){
		if (!list.elems[i].associated && !(strstr(list.elems[i].name, "Switch") || strstr(list.elems[i].name, "Playback Volume"))){
			printf("%d\t%d\t%s\n", list.elems[i].numid, list.elems[i].switch_id,list.elems[i].name);
		}
	}
	
	
	
	return(0);
}
