/* config_window.h */
/* This code is part of the Public Domain. */

#ifndef __CONFIG_WINDOW__
#define __CONFIG_WINDOW__

//load the current settings into a temporary tmp_settings variable
void load_settings(ConfigSettings *settings);

//save the current settings back to the rc file and apply them
void save_settings();

//create a preferences window
void build_config_window(ConfigSettings *settings);

#endif
