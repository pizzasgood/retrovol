/*
 alsa_classes.cpp

 This file contains classes to load and manipulate the mixer settings through
 alsa using asoundlib.h.  Numbers are automatically scaled to a 0-100 range
 to simplify use with volume sliders.
 
 created 2008-09-30 by pizzasgood
*/

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <math.h>

#include "alsa_classes.h"


//Constructor: automatically populates the element with its non-dynamic data
Element::Element(char *_card, int _numid, const char *_name){
	//do the easy stuff first
	card = _card;
	numid = _numid;
	strcpy(name, _name);
	
	int err;
	snd_ctl_t *handle;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_info_t *info;
	
	//allocate the items
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_info_alloca(&info);
	

	//set the interface and numid to grab the device which we're interested in
	snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
	snd_ctl_elem_id_set_numid(id, numid);
	
	//open a handle to use with the card
	if ((err = snd_ctl_open(&handle, card, 0)) < 0) {
		fprintf(stderr, "Control %s open error: %s\n", card, snd_strerror(err));
	}
	
	snd_ctl_elem_info_set_id(info, id);
	snd_ctl_elem_info(handle, info);
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
	
	//don't need the handle open anymore, so close it
	snd_ctl_close(handle);
}


//this is mainly for debugging; it prints data about the element to the console
void Element::print(){
	printf("numid: %d  name: %s\n", numid, name);
	printf("type: %s, values: %d\n", type, values);
	printf("min: %d, max: %d\n", min, max);
	printf("values: %d", get(0));
	for (int i=1; i<values; i++){
		printf(",%d", get(i));
	}
	printf("\n");
}


//this is used internally to scale a number to be from 0-100
int Element::scale_out(int num){
	return(ceil(100.0*(num-min)/(max-min)));
}
//this is the inverse of scale_out; it's used to take a 0-100 number and put it
//into the proper scale for the element to understand
int Element::scale_in(int num){
	return(floor((num*(max-min)/(100))+min));
}

//this is used internally to get the unscaled value of value n
int Element::_get(int n){
	if (n >= values || strcmp(type, "INTEGER") != 0){
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
	
	//grab the value
	int ret = snd_ctl_elem_value_get_integer(control, n);
	
	//don't need the handle open anymore, so close it
	snd_ctl_close(handle);
	
	return(ret);
	
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
void Element::set(int num){
	for (int i=0; i<values; i++){
		set(num, i);
	}
}
//this sets value n to num
void Element::set(int num, int n){
	_set(scale_in(num), n);
}

//this is used internally to set value n to num (unscaled)
void Element::_set(int num, int n){
	
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
		if(i!=n){
			snd_ctl_elem_value_set_integer(control, i, _get(i));
		} else {	
			snd_ctl_elem_value_set_integer(control, i, num);
		}
	}
	
	//make the change
	snd_ctl_elem_write(handle, control);
	
	//don't need the handle open anymore, so close it
	snd_ctl_close(handle);
	
	
	
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
	
	//clean up
	snd_hctl_free(hctl);
	snd_hctl_close(hctl);
	
}



//this is just here for testing; used to be main()
int test_alsa_stuff(int argc, char *argv[]){
	char card[] = "hw:0";
	ElementList list(card);
	list.elems[1].print();
	list.elems[1].set(66);
	list.elems[1].print();

	return(0);
}
