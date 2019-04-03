/*
 * Copyright 2007 Josh Kropf
 * 
 * This file is part of Lemon Launcher.
 * 
 * Lemon Launcher is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Lemon Launcher is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Lemon Launcher; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef OPTIONS_H_
#define OPTIONS_H_

#include <confuse.h>
#include <string>

namespace ll {

/* log level: 0 = off, 1 = error, 2 = info, 3 = warning, 4 = debug */
#define KEY_LOGLEVEL "loglevel"

/* Screen settings */
#define KEY_SCREEN_WIDTH   "width"      /* width of video mode  (int) */
#define KEY_SCREEN_HEIGHT  "height"     /* height of video mode (int) */
#define KEY_SCREEN_BPP     "bitdepth"   /* pits per pixel */
#define KEY_FULLSCREEN     "fullscreen" /* full screen mode (true/false) */

#define KEY_VIDEO_X1     "video_x1" /* video x1 */
#define KEY_VIDEO_X2     "video_x2" /* video x2 */
#define KEY_VIDEO_Y1     "video_y1" /* video y1 */
#define KEY_VIDEO_Y2     "video_y2" /* video y2 */
#define KEY_VIDEO_OR     "video_orientation" /* video or */


#define KEY_ORIENTATIONFILTER     "orientationfilter" /* "" "h" "v" */
#define KEY_PI2JAMMAJOYSTICK      "pi2jammajoystick" /* "" "y" "n" */
#define KEY_PI2JAMMAJOYSTICK_DELAY      "pi2jammajoystickdelay" /* msec  */

#define KEY_REMEMBERLAST       "lastselection"   /* y,n */
#define KEY_REMEMBERLASTFILE   "lastselection_tmp_file" /* filename */
  
#define KEY_ROTATE         "rotate"     /* rotate (0, 90, 180, 270) */

/* Ui settings */
#define KEY_SKIN_FILE       "theme"
#define KEY_SNAPSHOT_DELAY  "snapshot_delay"

/* MAME settings */
#define KEY_MAME_PATH       "mame"
#define KEY_MAME_SNAP_PATH  "snap"
#define KEY_MAME_VIDEO_PATH  "video"
#define KEY_MAME_VIDEO_VOL  "video_volume"

/* Key mapping */
#define KEY_KEYCODE_EXIT      "exit"
#define KEY_KEYCODE_UP        "up"
#define KEY_KEYCODE_DOWN      "down"
#define KEY_KEYCODE_PGUP      "pgup"
#define KEY_KEYCODE_PGDOWN    "pgdown"
#define KEY_KEYCODE_RELOAD    "reload"
#define KEY_KEYCODE_TOGGLE    "showhide"
#define KEY_KEYCODE_SELECT    "select"
#define KEY_KEYCODE_BACK      "back"
#define KEY_KEYCODE_ALPHAMOD  "alphamod"
#define KEY_KEYCODE_FAV       "fav"

/* Joystick mapping */
#define JOY_UP        "joy_up"
#define JOY_DOWN      "joy_down"
#define JOY_PGUP      "joy_pgup"
#define JOY_PGDOWN    "joy_pgdown"
#define JOY_SELECT    "joy_select"
#define JOY_BACK      "joy_back"
#define JOY_BUTTON_UP        "joy_button_up"
#define JOY_BUTTON_DOWN      "joy_button_down"
#define JOY_BUTTON_PGUP      "joy_button_pgup"
#define JOY_BUTTON_PGDOWN    "joy_button_pgdown"
#define JOY_ALPHAMOD_UP      "joy_alphamod_up"
#define JOY_ALPHAMOD_DOWN      "joy_alphamod_down"
#define JOY_FAV    "joy_fav"

/**
 * Class for reading configuration file.  Settings are accessed by passing
 * a key to one of the get_* methods.
 */
class options
{
private:
   char* _conf_dir;
   cfg_t *_cfg;
   
public:
   /**
    * Creates options class.  The load method must be called before calling
    * any of the get_ methods.
    */
   options();
   
   /** Cleanup */
   ~options();
   
   /** Parses conf conf files from the conf file directory */
   void load(const char* conf_dir);
   
   /** Returns an option as a boolean */
   bool get_bool(const char* key) const;
   
   /** Returns an option as an integer */
   int get_int(const char* key) const;
   
   /** Returns an option as a string */
   const char* get_string(const char* key) const;
   
   /**
    * Resolves the path to the file relative to the config dir (set at
    * compile time).
    */
   void resolve(std::string& file) const;
};

extern options g_opts;

} // end namespace declaration 

#endif /*OPTIONS_H_*/
