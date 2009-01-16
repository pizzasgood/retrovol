/* main.h */

#ifndef __MAIN__
#define __MAIN__


//callback that handles muting/unmuting a control
void toggle_it(GtkWidget *chkbx, Element *elem);

//callback that handles clicking the tray icon
static gboolean tray_button_press_event_callback (GtkWidget *widget, GdkEventButton *event, GtkWidget *slider_window);

//replace the second space (or the first if preceeding a '(' or '-') with '\n'
//else append a '\n' to keep things lined up
void word_wrap(char *wrapped, char *orig);

#endif
