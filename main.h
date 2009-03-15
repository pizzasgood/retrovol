/* main.h */
/* This code is part of the Public Domain. */

#ifndef __MAIN__
#define __MAIN__


//callback that handles changing an enumerated control
void change_combo_box(GtkWidget *combo_box, Element *elem);

//callback that handles muting/unmuting a control
void toggle_checkbox(GtkWidget *chkbx, Element *elem);
//callback that handles updating a checkbox
void refresh_checkbox(GtkWidget *chkbx, GdkEventExpose *event, Element *elem);

//callback that handles clicking the tray icon
gboolean tray_button_press_event_callback (GtkWidget *widget, GdkEventButton *event, GtkWidget *slider_window);

//update the tray-icon and refresh the window
gboolean update(gpointer data);

//replace the second space (or the first if preceeding a '(' or '-') with '\n'
//else append a '\n' to keep things lined up
void word_wrap(char *wrapped, char *orig);

#endif
