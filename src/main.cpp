/* main.cpp */
/* This code is part of the Public Domain. */

/*
	retrovol - a retro-styled volume mixer, by Pizzasgood
*/

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include "retro_slider.h"
#include "alsa_classes.h"
#include "config_settings.h"
#include "config_window.h"
#include "main.h"
#if ! GTK_CHECK_VERSION(2,16,0)
	#include "eggtrayicon.h"
#endif

//i18n stuff
#include "gettext.h"
#include <locale.h>
#define _(String) gettext (String)


static ConfigSettings settings;
//add the leading slash here, so that it can simply be concatenated with the results of getenv("HOME") later.
const char config_file[] = "/.retrovolrc";
const char pid_filename[] = "/tmp/retrovol.pid";
static ElementList *list_ptr;
bool cmdline_enable_bg_color = false;
char cmdline_bg_color[8];
bool start_hidden = false;
bool tray_at_bottom = true; //this will be updated on the fly and is used to detect when the tray moves



//signal handler to make the main window appear (if not already up)
void popup_handler(int signum){
	gtk_widget_show_all(settings.main_window);
}

//signal handler to exit cleanly on SIGINT and  SIGTERM
void exit_handler(int signum){
	settings.restart = false;
	gtk_main_quit();
}

//callback that handles changing an enumerated control
void change_combo_box(GtkWidget *combo_box, Element *elem){
	int state = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_box));
	elem->set((int)state);
}

//callback that handles muting/unmuting a control
void toggle_checkbox(GtkWidget *chkbx, Element *elem){
	bool state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(chkbx));
	elem->set((int)state);
}

void refresh_checkbox(GtkWidget *chkbx, GdkEventExpose *event, Element *elem){
	bool state = (bool)elem->get();
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chkbx), state);
}


//callback that handles clicking the tray icon
gboolean tray_button_press_event_callback (GObject *widget, GdkEventButton *event, GtkWidget *slider_window){
	
	switch(event->button){
		case 1:		//left mouse button - toggle slider_window
			if (GTK_WIDGET_VISIBLE(slider_window)){
				gtk_widget_hide_all(slider_window);
			} else {
				int slider_width, slider_height, icon_width, icon_height, x_pos, y_pos;
				int tray_offset; //this may be needed if the tray border hides the bottom of the slider
				//get some dimensions
				gtk_window_get_size(GTK_WINDOW(slider_window), &slider_width, &slider_height);
				GdkScreen *screen = gdk_screen_get_default();
#if GTK_CHECK_VERSION(2,16,0)
				GdkScreen *screen_from_tray;
				GdkRectangle area;
				GtkOrientation orientation;
				if (gtk_status_icon_get_geometry(settings.tray_icon, &screen_from_tray, &area, &orientation)){
					icon_width = area.width;
					icon_height = area.height;
				} else {
					//just outright guess
					icon_width = 24;
					icon_height = 24;
				}
#else
				gtk_window_get_size(GTK_WINDOW(widget), &icon_width, &icon_height);
#endif
				//compute the x position
				//x_pos = event->x_root - event->x - slider_width/2 + widget->allocation.width/2;
				x_pos = event->x_root - event->x - slider_width/2 + icon_width/2;
				//if the user has supplied an offset, use that, otherwise guess
				if (settings.tray_slider_offset >= 0){
					tray_offset = settings.tray_slider_offset;
				} else {
					if (event->y_root > gdk_screen_get_height(screen)/2){
						//tray at bottom
						tray_offset = gdk_screen_get_height(screen) - icon_height - event->y_root + event->y;
					} else {
						//tray at top
						tray_offset = event->y_root - event->y;
					}
				}
				//compute the y position
				if (event->y_root > gdk_screen_get_height(screen)/2){
					//tray at bottom
					y_pos = event->y_root-event->y-slider_height-tray_offset;
				} else {
					//tray at top
					y_pos = icon_height+event->y_root-event->y+tray_offset;
				}
				//do the deed
				gtk_window_move(GTK_WINDOW(slider_window), x_pos, y_pos);
				gtk_widget_show_all(slider_window);
			}
			break;
		case 3:		//right mouse button - display tray menu or main window, depending on settings.enable_tray_menu
			if (settings.enable_tray_menu){
				gtk_menu_popup(GTK_MENU(settings.tray_icon_menu), NULL, NULL, NULL, NULL, event->button, event->time);
			} else {
				if (GTK_WIDGET_VISIBLE(settings.main_window)){
					gtk_widget_hide(settings.main_window);
				} else {
					gtk_widget_show_all(settings.main_window);
				}
			}
			break;
		case 2:		//middle mouse button - mute
			if (settings.tray_control->switch_id >= 0){
				bool state = !(bool)list_ptr->elems[settings.tray_control->switch_id].get();
				list_ptr->elems[settings.tray_control->switch_id].set((int)(state));
			}
			break;
		default:
			break;
	}
	
	update(NULL);
	return(true);
}

//update the tray-icon and refresh the window
gboolean update(gpointer data){
	bool state = true;
	if (settings.enable_tray_icon){
		int val = settings.tray_control->get();
		char tooltiptext[32];
		if (settings.tray_control->switch_id >= 0){
			state = (bool)list_ptr->elems[settings.tray_control->switch_id].get();
		}
		if (state){
			int image = 2+3*val/100;
			if (image > 4){
				image=4;
			} else if (image < 0){
				image=0;
			} else if (val == 0){
				image=1;
			}
			sprintf(tooltiptext, _("Volume: %d%%"), val);
#if GTK_CHECK_VERSION(2,16,0)
			gtk_status_icon_set_from_file(settings.tray_icon, settings.icon_file_names[image]);
#else
			gtk_image_set_from_file(GTK_IMAGE(settings.tray_icon_image), settings.icon_file_names[image]);
#endif
		} else {
			sprintf(tooltiptext, _("Volume: Muted"));
#if GTK_CHECK_VERSION(2,16,0)
			gtk_status_icon_set_from_file(settings.tray_icon, settings.icon_file_names[0]);
#else
			gtk_image_set_from_file(GTK_IMAGE(settings.tray_icon_image), settings.icon_file_names[0]);
#endif
		}
#if GTK_CHECK_VERSION(2,16,0)
			gtk_status_icon_set_tooltip_text(settings.tray_icon, tooltiptext);
#elif GTK_CHECK_VERSION(2,12,0)
			gtk_widget_set_tooltip_text(settings.tray_icon_image, tooltiptext);
#else
			static GtkTooltips *tooltips = gtk_tooltips_new();
			gtk_tooltips_set_tip(tooltips, settings.tray_icon_image, tooltiptext, NULL);
#endif
		//if the tray was moved, update the menu
		GdkScreen *screen = gdk_screen_get_default();
		int icon_x,icon_y;
#if GTK_CHECK_VERSION(2,16,0)
		GdkScreen *screen_from_tray;
		GdkRectangle area;
		GtkOrientation orientation;
		if (gtk_status_icon_get_geometry(settings.tray_icon, &screen_from_tray, &area, &orientation)){
			icon_x = area.x;
			icon_y = area.y;
		} else {
			icon_x = gdk_screen_get_width(screen);
			icon_y = gdk_screen_get_height(screen);
		}
#else
		gtk_window_get_position(GTK_WINDOW(settings.tray_icon), &icon_x, &icon_y);
#endif
		if ((bool)(icon_y > gdk_screen_get_height(screen)/2) ^ tray_at_bottom){
			//the current status does not match the previous status, so update the menu
			set_menu();
		}
		//in case the icon was hidden due to the tray exiting, try reshowing it again
#if GTK_CHECK_VERSION(2,16,0)
		gtk_status_icon_set_visible(settings.tray_icon, true);
#else
		gtk_widget_show_all(settings.tray_icon);
#endif
	}
	if (GTK_WIDGET_VISIBLE(settings.main_window)){
		gtk_widget_queue_draw(settings.main_window);
	}
	if (GTK_WIDGET_VISIBLE(settings.slider_window)){
		gtk_widget_queue_draw(settings.slider_window);
	}
	
	return(true);
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


//callback for the configure window
void configure( GtkWidget *w, gpointer data){
	build_config_window(&settings);
}

//callback that opens the main window
void open_window(GtkWidget *w, gpointer data){
	restore_posdim();
	gtk_widget_show_all(settings.main_window);
}

//callback that closes the main window
void close_window( GtkWidget *w, gpointer data){
	if (settings.enable_tray_icon){
		gtk_widget_hide(settings.main_window);
	} else {
		gtk_widget_destroy(settings.main_window);
	}
}

//Returns a menubar widget made from the passed menu_items
GtkWidget *get_menubar_menu( GtkWidget  *window, GtkItemFactoryEntry *menu_items, gint nmenu_items, const char *menu_name )
{
	GtkItemFactory *item_factory;
	GtkAccelGroup *accel_group;

	/* Make an accelerator group (shortcut keys) */
	accel_group = gtk_accel_group_new ();

	/* Make an ItemFactory (that makes a menubar) */
	item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, menu_name, accel_group);

	/* This function generates the menu items. Pass the item factory,
	   the number of items in the array, the array itself, and any
	   callback data for the the menu items. */
	gtk_item_factory_create_items (item_factory, nmenu_items, menu_items, NULL);

	/* Attach the new accelerator group to the window. */
	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);

	/* Finally, return the actual menu bar created by the item factory. */
	return gtk_item_factory_get_widget (item_factory, menu_name);
}


//set up the popup menu, if enabled
void set_menu(){
	//TODO: add stock icons
	if (settings.enable_tray_menu){
		GtkWidget *exit_entry = gtk_menu_item_new_with_mnemonic(_("_Exit"));
		GtkWidget *config_entry = gtk_menu_item_new_with_mnemonic(_("_Config Window"));
		GtkWidget *show_entry = gtk_menu_item_new_with_mnemonic(_("_Full Window"));
		g_signal_connect(G_OBJECT(exit_entry), "activate", G_CALLBACK(gtk_main_quit), NULL);
		g_signal_connect(G_OBJECT(config_entry), "activate", G_CALLBACK(configure), NULL);
		g_signal_connect(G_OBJECT(show_entry), "activate", G_CALLBACK(open_window), NULL);
		if (G_IS_OBJECT(settings.tray_icon_menu)){
			gtk_widget_destroy(settings.tray_icon_menu);
		}
		settings.tray_icon_menu = gtk_menu_new();

		//Decide which order to stack the menu, so that the entry under the mouse
		//initially will hopefully be show_entry, which is what the user is most
		//likely trying to use, as opposed to exit_entry, which they probably do
		//not want to accidentally hit...
		GdkScreen *screen = gdk_screen_get_default();
		int icon_x,icon_y;
#if GTK_CHECK_VERSION(2,16,0)
		GdkScreen *screen_from_tray;
		GdkRectangle area;
		GtkOrientation orientation;
		if (gtk_status_icon_get_geometry(settings.tray_icon, &screen_from_tray, &area, &orientation)){
			icon_x = area.x;
			icon_y = area.y;
		} else {
			icon_x = gdk_screen_get_width(screen);
			icon_y = gdk_screen_get_height(screen);
		}
#else
		gtk_window_get_position(GTK_WINDOW(settings.tray_icon), &icon_x, &icon_y);
#endif
		if (icon_y > gdk_screen_get_height(screen)/2){
			//taskbar is on bottom, so put show_entry on bottom for ergonomics
			tray_at_bottom = true;
			gtk_menu_shell_append(GTK_MENU_SHELL(settings.tray_icon_menu), exit_entry);
			gtk_menu_shell_append(GTK_MENU_SHELL(settings.tray_icon_menu), config_entry);
			gtk_menu_shell_append(GTK_MENU_SHELL(settings.tray_icon_menu), show_entry);
		} else {
			//taskbar is on top, so put show_entry on top for ergonomics
			tray_at_bottom = false;
			gtk_menu_shell_append(GTK_MENU_SHELL(settings.tray_icon_menu), show_entry);
			gtk_menu_shell_append(GTK_MENU_SHELL(settings.tray_icon_menu), config_entry);
			gtk_menu_shell_append(GTK_MENU_SHELL(settings.tray_icon_menu), exit_entry);
		}
		
		//make menu potentially visible
		gtk_widget_show_all(settings.tray_icon_menu);
	}
}

//save the position and dimensions of the window
gboolean save_posdim(GtkWidget *widget, GdkEventConfigure *event, gpointer data){
	settings.window_x = event->x;
	settings.window_y = event->y;
	settings.window_width = event->width;
	settings.window_height = event->height;
	settings.write_config();
}

//restore the position and dimensions of the window
void restore_posdim(){
	if (settings.window_x >= 0 || settings.window_y >= 0){
		GdkScreen *screen = gdk_screen_get_default();
		int window_w, window_h;
		gtk_window_get_size(GTK_WINDOW(settings.main_window), &window_w, &window_h);
		if (settings.window_x + window_w > gdk_screen_get_width(screen)){
			settings.window_x = gdk_screen_get_width(screen) - window_w;
		} else if (settings.window_x < 0){
			settings.window_x = 0;
		}
		if (settings.window_y + window_h > gdk_screen_get_height(screen)){
			settings.window_y = gdk_screen_get_height(screen) - window_h;
		} else if (settings.window_y < 0){
			settings.window_y = 0;
		}
		gtk_window_move(GTK_WINDOW(settings.main_window), settings.window_x, settings.window_y);
	}
}

bool loop(int argc, char** argv) {

	char home[256]; //NEED TO MAKE THIS DYNAMIC
	strcpy(home, getenv("HOME"));

	//parse the config file
	settings.parse_config(strcat(home, config_file));
	//load the controls into list
	ElementList list(settings.card);
	list_ptr = &list;
	//reorder the controls to the order specified in the config file
	settings.reorder_list(&list);
	
	//initialize gtk
	gtk_init(&argc, &argv);

	//set up the tray_slider that goes in the tray
	if (settings.enable_tray_icon){
		GtkWidget *tray_frame;
		tray_frame = gtk_alignment_new(0.5,0.0,0,0);
		settings.tray_slider = new retro_slider;
		settings.set_tray_slider(&list);
		settings.apply_to_tray_slider(settings.tray_slider);
		settings.tray_slider->init(tray_frame, (void*)settings.tray_control, &Element::get_callback, &Element::set_callback, (settings.tray_control->values > 1));

		//set up the small window that holds the tray_slider
		settings.slider_window = gtk_window_new (GTK_WINDOW_POPUP);
		gtk_window_set_resizable(GTK_WINDOW(settings.slider_window), false);
		gtk_window_set_decorated(GTK_WINDOW(settings.slider_window), false);
		gtk_window_set_skip_taskbar_hint(GTK_WINDOW(settings.slider_window), true);
		gtk_window_set_skip_pager_hint(GTK_WINDOW(settings.slider_window), true);
		gtk_widget_set_usize(settings.slider_window, settings.tray_slider->width, settings.tray_slider->height);
		//don't want accidental closure of the slider window to destroy the window
		g_signal_connect(settings.slider_window, "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);
		//want the widow to go away when it loses focus
		g_signal_connect(settings.slider_window, "focus-out-event", G_CALLBACK (gtk_widget_hide), NULL);
		gtk_container_add( GTK_CONTAINER(settings.slider_window), tray_frame );
		//we want it hidden by default, but it must be shown at least once or else scrolling over the icon will cause a hang
		gtk_widget_show_all(settings.slider_window);
		gtk_widget_hide_all(settings.slider_window);
		
			
		//set up tray icon
#if GTK_CHECK_VERSION(2,16,0)
		settings.tray_icon = gtk_status_icon_new();
		gtk_status_icon_set_from_file(settings.tray_icon, VOL_MEDIUM_IMAGE);
#else
		settings.tray_icon = GTK_WIDGET(egg_tray_icon_new("Retrovol Tray Icon"));
		//set the background color
		bool enable_tray_icon_background_color = settings.enable_tray_icon_background_color; 
		GdkColor bg_color;
		char bg_color_str[8];
		if (cmdline_enable_bg_color){
			enable_tray_icon_background_color = true;
			strcpy(bg_color_str, cmdline_bg_color);
		} else if (settings.enable_tray_icon_background_color){
			settings.nftoh(settings.tray_icon_background_color, bg_color_str);
		}
		if (enable_tray_icon_background_color){
			if (gdk_color_parse(bg_color_str, &bg_color)){
				GtkStyle *style = gtk_style_copy(gtk_widget_get_style(settings.tray_icon));
				style->bg[GTK_STATE_NORMAL] = bg_color;
				gtk_widget_set_style(settings.tray_icon, style);
			} else {
				fprintf(stderr, _("Error:  Failed to set background color to %s\n"), bg_color_str);
			}
		}
		//set up the images
		settings.tray_icon_image = gtk_image_new();
		gtk_container_add( GTK_CONTAINER(settings.tray_icon), settings.tray_icon_image );
		gtk_image_set_from_file(GTK_IMAGE(settings.tray_icon_image), VOL_MEDIUM_IMAGE);
		//set the event mask
		gtk_widget_set_events (settings.tray_icon, GDK_BUTTON_PRESS_MASK | GDK_SCROLL_MASK);
#endif

		//signals
		g_signal_connect(G_OBJECT(settings.tray_icon), "button_press_event", G_CALLBACK (&tray_button_press_event_callback), settings.slider_window);
		g_signal_connect(G_OBJECT(settings.tray_icon), "scroll_event", G_CALLBACK (&retro_slider::scroll_event_callback), settings.tray_slider);

#if GTK_CHECK_VERSION(2,16,0)
		//make icon visible
		gtk_status_icon_set_visible(settings.tray_icon, true);
#else
		//handle situations where the icon's window dies, such as due to the tray itself exiting
		g_signal_connect(G_OBJECT(settings.tray_icon), "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);

		//make icon visible
		gtk_widget_show_all(settings.tray_icon);
#endif

		//set up the popup menu (the function checks if it should actually do anything)
		set_menu();

	}
	


	//set up the window
	settings.main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	//gtk_window_set_position(GTK_WINDOW(settings.main_window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(settings.main_window), settings.window_width, settings.window_height);
	gtk_window_set_title(GTK_WINDOW(settings.main_window), "Retrovol");
	restore_posdim();
	g_signal_connect(settings.main_window, "configure-event", G_CALLBACK (save_posdim), NULL);
	
	//if the tray icon is enabled, we want the window to hide rather than closing
	if (settings.enable_tray_icon){
		g_signal_connect(settings.main_window, "delete-event", G_CALLBACK (gtk_widget_hide_on_delete), NULL);
	}

	//make the over_box, which will hold stuff like the menu, status bar, and the actual content in the middle
	GtkWidget *over_box;
	over_box = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(settings.main_window), over_box);

	//define the menu
	GtkItemFactoryEntry menu_items_1[] = {
		{ (gchar*)_("/_File"),           NULL,              NULL,                      0, (gchar*)"<Branch>" },
		{ (gchar*)_("/File/_Configure"), (gchar*)"<CTRL>C", G_CALLBACK(configure),     0, (gchar*)"<StockItem>", GTK_STOCK_NEW },
		{ (gchar*)_("/File/_Quit"),      (gchar*)"<CTRL>Q", G_CALLBACK(close_window),  0, (gchar*)"<StockItem>", GTK_STOCK_QUIT },
	};
	gint nmenu_items_1 = sizeof (menu_items_1) / sizeof (menu_items_1[0]);
	
	GtkItemFactoryEntry menu_items_2[] = {
		{ (gchar*)_("/_File"),           NULL,              NULL,                      0, (gchar*)"<Branch>" },
		{ (gchar*)_("/File/_Configure"), (gchar*)"<CTRL>C", G_CALLBACK(configure),     0, (gchar*)"<StockItem>", GTK_STOCK_NEW },
		{ (gchar*)_("/File/_Exit completely"),      (gchar*)"<CTRL>E", G_CALLBACK(gtk_main_quit),  0, (gchar*)"<StockItem>", GTK_STOCK_QUIT },
		{ (gchar*)_("/File/_Quit"),      (gchar*)"<CTRL>Q", G_CALLBACK(close_window),  0, (gchar*)"<StockItem>", GTK_STOCK_QUIT },
	};
	gint nmenu_items_2 = sizeof (menu_items_2) / sizeof (menu_items_2[0]);

	GtkItemFactoryEntry *menu_items;
	gint nmenu_items;
	//if the tray menu is enabled, don't have the "Exit" entry in the main menu
	if (settings.enable_tray_menu){
		menu_items = menu_items_1;
		nmenu_items = nmenu_items_1;
	} else {
		menu_items = menu_items_2;
		nmenu_items = nmenu_items_2;
	}

	//build the menu
	GtkWidget *menubar;
	menubar = get_menubar_menu(settings.main_window, menu_items, nmenu_items, "<RetrovolMain>");
	gtk_box_pack_start(GTK_BOX(over_box), menubar, FALSE, TRUE, 0);


	//use a scrolled window
	GtkWidget *scrolled_window;
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy((GtkScrolledWindow*)scrolled_window, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(over_box), scrolled_window);
	
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
			sliders[i].init(frame, (void*)list.items[i], &Element::get_callback, &Element::set_callback, (list.items[i]->values > 1));
		
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
			//bind to the toggle_checkbox function
			Element* ptr = list.items[i];
			g_signal_connect(GTK_TOGGLE_BUTTON(chkbx), "toggled", G_CALLBACK (toggle_checkbox), ptr);
			g_signal_connect_after(GTK_TOGGLE_BUTTON(chkbx), "expose-event", G_CALLBACK (refresh_checkbox), ptr);
			gtk_container_add(GTK_CONTAINER(alignment), chkbx);
		} else if (strcmp(list.items[i]->type, "ENUMERATED") == 0){
			GtkWidget *alignment;
			if (settings.vertical){
				alignment = gtk_alignment_new(0.5,0.5,0,0);
				gtk_box_pack_start(GTK_BOX(vbox), alignment, true, true, 0);
			} else {
				alignment = gtk_alignment_new(1.0,0.5,0,0);
				gtk_box_pack_end(GTK_BOX(vbox), alignment, true, true, 0);
			}
			//insert a combobox with the different options
			GtkWidget *combo_box;
			combo_box=gtk_combo_box_new_text();
			for(unsigned int n=0; n<list.items[i]->number_of_enums; n++){
				gtk_combo_box_append_text(GTK_COMBO_BOX(combo_box), list.items[i]->enums[n]);
			}
			gtk_combo_box_set_active(GTK_COMBO_BOX(combo_box), list.items[i]->get());
			//bind to the change_combo_box function
			g_signal_connect(GTK_COMBO_BOX(combo_box), "changed", G_CALLBACK (change_combo_box), list.items[i]);
			gtk_container_add(GTK_CONTAINER(alignment), combo_box);
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
			//bind to the toggle_checkbox function
			g_signal_connect(GTK_TOGGLE_BUTTON(chkbx), "toggled", G_CALLBACK (toggle_checkbox), &(list.elems[list.items[i]->switch_id]));
			g_signal_connect_after(GTK_TOGGLE_BUTTON(chkbx), "expose-event", G_CALLBACK (refresh_checkbox), &(list.elems[list.items[i]->switch_id]));
			
			gtk_container_add(GTK_CONTAINER(alignment), chkbx);
		}
		
		//display the name of the control
		GtkWidget *alignment;
		char wrapped[256];
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
	
	//finish the window stuff
	if (!start_hidden){ gtk_widget_show_all(settings.main_window); }
	g_signal_connect(settings.main_window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
	

	//add some periodic refreshment to keep the icon and window up-to-date
	#if GTK_CHECK_VERSION(2,14,0)
		g_timeout_add_seconds(1, update, NULL);
	#else
		//this is less efficient than g_timeout_add_seconds()
		g_timeout_add(1000, update, NULL);
	#endif
	
	//finished with gtk setup
	gtk_main();
	
	//have the window shown again if it was open before we restarted
	if (settings.resume_main){
		settings.resume_main = false;
		start_hidden = false;
	} else {
		start_hidden = true;
	}

	return(settings.restart);
}


int main(int argc, char** argv) {
	//initialize locale jazz
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	for (int i=1; i<argc; i++){
		if (strcmp(argv[i], "-bg") == 0){
			if (i+1 < argc && strlen(argv[i+1]) == 7){
				cmdline_enable_bg_color = true;
				strcpy(cmdline_bg_color, argv[i+1]);
				i++;
			} else {
				fprintf(stderr, _("ERROR:  The -bg option requires a color to be supplied in the format #rrggbb\n"));
			}
		} else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0){
			printf("%s %s\n", argv[0], VERSION);
			exit(0);
		} else if (strcmp(argv[i], "-hide") == 0 || strcmp(argv[i], "--hide") == 0){
			start_hidden = true;
		} else {
			fprintf(stderr, _("Usage: %s [-bg #rrggbb] [-hide]\n"), argv[0]);
			fprintf(stderr, _("Volume mixer with bargraph style sliders.\n"));
			fprintf(stderr, _("Reads ~/.retrovolrc for configuration options.  Additionally, the following\noptions may be given on the commandline:\n"));
			fprintf(stderr, _("\t-v              print the version number and exit\n"));
			fprintf(stderr, _("\t-bg #rrggbb     specify the background color of the tray icon\n"));
			fprintf(stderr, _("\t-hide           hide the main window initially\n"));
			exit(1);
		}
	}

	//create a PID file if not already running, else pop up a window in the running instance and then exit
	FILE *pidfile = fopen(pid_filename, "r");
	bool created_file = false;
	if (pidfile != NULL){
		char pid_string[16];
		fgets(pid_string, 16, pidfile);
		fclose(pidfile);
		//check if it is a zombie process
		char command[256];
		sprintf(command, "ps --no-header -o state %d | grep -q Z", atoi(pid_string)); //TODO:  Use better method (/proc/<pid_string>/state?)
		if (system(command)){
			if (kill(atoi(pid_string), SIGUSR1) == 0){
				exit(0);
			}
		}
	}
	//if we reached this point, we need to create the file
	pidfile = fopen(pid_filename, "w");
	if (pidfile != NULL){
		fprintf(pidfile, "%d\n", getpid());
		fclose(pidfile);
		created_file = true;
	} else {
		fprintf(stderr, _("Error: could not create %s\n"), pid_filename);
	}

	//when SIGUSR1 is recieved, pop up the window
	signal(SIGUSR1, popup_handler);

	//when SIGINT or SIGTERM is recieved, exit cleanly
	signal(SIGINT, exit_handler);
	signal(SIGTERM, exit_handler);

	//main program - in a while loop so that it can easily restart to refresh itself
	while (loop(argc, argv));

	//clean up after ourselves
	if (created_file){ remove(pid_filename); }
	return(0);
}

