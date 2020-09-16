/* main.h */
/* This code is part of the Public Domain. */

#ifndef __MAIN__
#define __MAIN__

//signal handler to make the main window appear (if not already up)
void popup_handler(int signum);

//signal handler to exit cleanly on SIGINT and  SIGTERM
void exit_handler(int signum);

//callback that handles changing an enumerated control
void change_combo_box(GtkWidget *combo_box, Element *elem);

//callback that handles muting/unmuting a control
void toggle_checkbox(GtkWidget *chkbx, Element *elem);
//callback that handles updating a checkbox
void refresh_checkbox(GtkWidget *chkbx, GdkEventExpose *event, Element *elem);

//callback that handles clicking the tray icon
gboolean tray_button_press_event_callback (GObject *widget, GdkEventButton *event, GtkWidget *slider_window);

//update the tray-icon and refresh the window
gboolean update(gpointer data);

//replace the second space (or the first if preceeding a '(' or '-') with '\n'
//else append a '\n' to keep things lined up
void word_wrap(char *wrapped, char *orig);

//callback for the configure window
void configure( GtkWidget *w, gpointer data);

//callback that opens the main window
void open_window(GtkWidget *w, gpointer data);

//callback that closes the main window
void close_window( GtkWidget *w, gpointer data);

//Returns a menubar widget made from the passed menu_items
GtkWidget *get_menubar_menu( GtkWidget  *window, GtkItemFactoryEntry *menu_items, gint nmenu_items, const char *menu_name );

//set up the popup menu, if enabled
void set_menu();

//save the position and dimensions of the window
void save_posdim(GtkWidget *widget, GdkEventConfigure *event, gpointer data);

//restore the position and dimensions of the window
void restore_posdim();

bool loop(int argc, char** argv);
int main(int argc, char** argv);

#endif
