/* alsa_classes.h */

//This stores data about an element (e.g. Front Volume, Front Switch, etc.)
//and provides get() and set() functions to deal with its values.  Numbers are
//This stores data about an element (e.g. Front Volume, Front Switch, etc.)
//and provides get() and set() functions to deal with its values.  Numbers are
//scaled to be in a 0-100 range.
class Element {
	public:
		Element(char *_card, int _numid, const char *_name);
		char *card;
		int numid;
		int index;
		char name[80];
		char type[16];
		int values;
		int switch_id;
		bool associated;
		
		int get();
		int get(int n);
		
		int set(int num);
		int set(int num, int n);
		void set_lr(int lvol, int rvol);
		
		static float get_callback(void *obj);
		static float get_callback(void *obj, int n);
		static float set_callback(void *obj, float num);
		static float set_callback(void *obj, float num, int n);
		
		void print();
		
		
	private:
		int min;
		int max;
		int scale_out(int num);
		int scale_in(int num);
		int _get(int n);
		int _set(int num, int n);
};


//This stores a list of all Elements that are availible on a card
class ElementList {
	public:
		ElementList(char *_card);
		
		char card[8];
		
		int num_elems;
		Element *elems;
};




//this is just here for testing; used to be main()
int test_alsa_stuff();

