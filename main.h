/* main.h */

#ifndef __MAIN__
#define __MAIN__


//callback that handles muting/unmuting a control
void toggle_it(GtkWidget *chkbx, Element *elem);

//replace the second space (or the first if preceeding a '(' or '-') with '\n'
//else append a '\n' to keep things lined up
void word_wrap(char *wrapped, char *orig);

#endif
