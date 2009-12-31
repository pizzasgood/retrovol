/* config_window.h */
/* This code is part of the Public Domain. */

#ifndef __CONFIG_WINDOW__
#define __CONFIG_WINDOW__

class SwapStruc {
	public:
		bool *control;
		int *iA;
		int *iB;
		GtkAdjustment *adj_A;
		GtkAdjustment *adj_B;

		void swap();
		void toggle();
		void set(GtkToggleButton *button);
};

class OrderWidget {
	private:
		GtkWidget *hbox;
		GtkListStore *a_store;
		GtkWidget *a_list;
		GtkListStore *i_store;
		GtkWidget *i_list;
		GtkListStore *build_list_from_names(int num_names, const char name_list[][80]);

		//move the selected item up in the list
		static void move_selected_up(GtkWidget *widget, gpointer data);
		//move the selected item down in the list
		static void move_selected_down(GtkWidget *widget, gpointer data);
		//add the selected item to the list
		static void add_selected(GtkWidget *widget, gpointer data);
		//remove the selected item from the list
		static void remove_selected(GtkWidget *widget, gpointer data);
	public:
		//build the widgets
		void build(GtkContainer *parent_container, int *num_names, char name_list[][80], ElementList *list_ptr);
		//update the name_list to match the GtkListStore
		int update_names_from_list(char name_list[][80]);
};

//load the current settings into a temporary tmp_settings variable
void load_settings(ConfigSettings *settings);

//save the current settings back to the rc file and apply them
void save_settings();

//close the window without saving anything
static void cancel_config_window(GtkWidget *widget, gpointer data);

//close the window and save the settings
static void apply_config_window(GtkWidget *widget, gpointer data);

//return a pointer to a viewport in a scrolled window in a notebook tab
GtkWidget *tab_init(GtkWidget *notebook, const char *label_text);

//update the value pointed to by the data pointer with the value contained by the widget
static void update_int(GtkWidget *widget, gpointer data);

//create an entry to edit an int value w/ spinbutton
GtkAdjustment *add_entry_int(GtkWidget *vbox, const char *label_text, int *item);

//update the value pointed to by the data pointer with the value contained by the widget
static void update_bool(GtkWidget *widget, gpointer data);

//update the bool and swap the ints pointed to by the data pointer with the value contained by the widget
static void update_bool_s(GtkWidget *widget, gpointer data);

//create an entry to edit a bool value w/ checkbox
void add_entry_bool_c(GtkWidget *vbox, const char *label_text, bool *item, SwapStruc *swap_struc = NULL);

//create an entry to edit a bool value w/ radiobutton
void add_entry_bool_r(GtkWidget *vbox, const char *label_text, const char *true_label, const char *false_label, bool *item, SwapStruc *swap_struc = NULL);

//update the value pointed to by the data pointer with the value contained by the widget
static void update_color(GtkWidget *widget, gpointer data);

//create an entry to edit a color value
void add_entry_color(GtkWidget *vbox, const char *label_text, float *item);

//create a preferences window
void build_config_window(ConfigSettings *settings);

#endif
