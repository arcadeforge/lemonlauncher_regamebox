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

#include <SDL/SDL_image.h>
#include "game.h"
#include "menu.h"
#include "options.h"
#include "error.h"
#include "log.h"

using namespace ll;
using namespace std;

SDL_Surface* game::snapshot()
{

    string img(g_opts.get_string(KEY_MAME_SNAP_PATH));
    string vid(g_opts.get_string(KEY_MAME_VIDEO_PATH));

    string video_x1(g_opts.get_string(KEY_VIDEO_X1));
    string video_y1(g_opts.get_string(KEY_VIDEO_Y1));
    string video_x2(g_opts.get_string(KEY_VIDEO_X2));
    string video_y2(g_opts.get_string(KEY_VIDEO_Y2));
    string video_or(g_opts.get_string(KEY_VIDEO_OR));

    string video_vol(g_opts.get_string(KEY_MAME_VIDEO_VOL));
   
    size_t pos = img.find("%r");
    if (pos == string::npos) {
        log << warn << "game::snapshot: snap option missing %r specifier" << endl;
        return NULL;
    }

    size_t pos_vid = vid.find("%r");
    if (pos_vid == string::npos) {
        log << warn << "game::snapshot: video option missing %r specifier" << endl;
        //return NULL;
        printf("no video");
    } 
    else
    {
        vid.replace(pos_vid, 2, rom());
    }
    img.replace(pos, 2, rom());
   
    system("pkill -9 omxplayer"); 
    char buf[1024];

    char testbuf[5];
    //char test[] = "0000";
    string test = "";

    // when no video player parameter in lemonlaunch.conf are
    // given the default value 0 will be taken. 
    // catch them and execute no system call for omxplayer
    snprintf(testbuf, sizeof(testbuf), "%s%s%s%s", video_x1.c_str(), video_y1.c_str(), video_x2.c_str(), video_y2.c_str());
    //printf("test#%s#\n", test);
    //printf("testbuf#%s#\n", testbuf);

//    if ( strcmp(testbuf, test) == 0) 
    if ( vid == test ) 
    {
        //printf("no video\n");
    } else {
        //printf("omx player\n");

        snprintf(buf, sizeof(buf), "omxplayer --orientation %s --win \"%s %s %s %s\" --vol %s %s > /dev/null 2>&1 &", video_or.c_str(), video_x1.c_str(), video_y1.c_str(), video_x2.c_str(), video_y2.c_str(), video_vol.c_str(), vid.c_str());
        //snprintf(buf, sizeof(buf), "omxplayer --win \"%s %s %s %s\" %s > /dev/null 2>&1 &", video_x1.c_str(), video_y1.c_str(), video_x2.c_str(), video_y2.c_str(), vid.c_str());
        system(buf);
        //system("omxplayer --win \"100 100 320 240\" /root/w.mp4 > /dev/null 2>&1 &");
        //printf ("%s\n",buf);
    }
         
   
    log << debug << "game::snapshot: " << img << endl;
    printf("snap : %s\n", img.c_str());
    return IMG_Load(img.c_str());
      //   return IMG_Load("/mnt/sulla/background4.png");
      
}

SDL_Surface* game::draw(TTF_Font* font, SDL_Color color, SDL_Color hover_color) const
{
   SDL_Color c = this == ((menu*)parent())->selected()? hover_color : color;
   return TTF_RenderText_Blended(font, text(), c);
}
