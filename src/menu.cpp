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
#include "menu.h"
#include "options.h"
#include <cctype>
#include "log.h"

using namespace ll;

menu::~menu()
{
   for (vector<item*>::iterator i = _children.begin(); i != _children.end(); i++)
      delete *i;
}

const bool menu::select_next(int step)
{
   int last = _children.size()-1;
   if (_selected < last) {
      _selected = _selected + step <= last? _selected + step : last;
      return true;
   }
   
   return false;
}

const bool menu::select_previous(int step)
{
   if (_selected > 0) {
      _selected = _selected - step >= 0? _selected - step : 0;
      return true;
   }
   
   return false;
}

const bool menu::select_next_alpha()
{
   // first character of selected child in lowercase
   int sel_ch = tolower(_children[_selected]->text()[0]);

   // iterate over children to find next in alphabetic order
   for (int i=_selected, last=_children.size()-1; i <= last; i++) {
      if (tolower(_children[i]->text()[0]) > sel_ch) {
         _selected = i;
         return true;
      }
   }
   
   return false;
}

const bool menu::select_previous_alpha()
{
   // first character of selected child in lowercase
   int sel_ch = tolower(_children[_selected]->text()[0]);

   // iterate over children to find privious in alphabetic order
   for (int i=_selected; i >= 0; i--) {
      if (tolower(_children[i]->text()[0]) < sel_ch) {
         _selected = i;
         return true;
      }
   }
   
   return false;
}

SDL_Surface* menu::draw(TTF_Font* font, SDL_Color color, SDL_Color hover_color) const
{
   SDL_Color c = parent() && this == ((menu*)parent())->selected()? hover_color : color;
   return TTF_RenderText_Blended(font, text(), c);
}

SDL_Surface* menu::snapshot()
{

    string img(g_opts.get_string(KEY_MAME_SNAP_PATH));
    size_t pos = img.find("%r");
    if (pos == string::npos) {
        log << warn << "menu::snapshot: snap option missing %r specifier" << endl;
        return NULL;
    }
    img.replace(pos, 2, text());
    log << debug << "menu::snapshot: " << img << endl;
    printf("menu %s\n" , img.c_str());
    return IMG_Load(img.c_str());
      //   return IMG_Load("/mnt/sulla/background4.png");


}
