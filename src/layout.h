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
#ifndef LAYOUT_H_
#define LAYOUT_H_

#include <confuse.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include "menu.h"
#include <string>

#define DIMENSION_FULL -1

namespace ll {

typedef enum { left_justify, right_justify, center_justify } justify_t;

/**
 * Class for handling layout and rendering of the interface
 */
class layout {
private:
   std::string _theme_dir;
   
   SDL_Surface* _bg;
   SDL_Rect _screen;
   
   TTF_Font* _title_font;
   TTF_Font* _list_font;
   
   SDL_Rect _title_rect;
   Uint32 _title_color;
   int _title_font_height;
   justify_t _title_justify;
   
   SDL_Rect _list_rect;
   SDL_Color _list_color;
   SDL_Color _list_hover_color;
   int _list_font_height;
   int _list_item_spacing;
   justify_t _list_justify;
   int _page_size;
   
   SDL_Rect _snap_rect;
   Uint8 _snap_alpha;
   
   SDL_Surface* _snap;
   SDL_Surface* _buffer;
   
   /** Render menu item at the given verticle offset */
   void render_item(SDL_Surface* buffer, item* i, int yoff);
   
   /**
    * Parses the dimensions option from the conf section and fills in the
    * w,h props of the rect.  The screen rect (member var) and x,y props
    * of the rect are used to calculate w,h when one of the dimensions
    * has the 'full' keyword. 
    */
   void parse_dimensions(SDL_Rect* rect, cfg_t* sec);
   
   /**
    * When path is relative (no leading forward slash) return the path appended
    * to the theme directory path.  Otherwise, return path unchanged.
    */
   void normalize(const char* path, string& new_path);
   
public:
   /**
    * Creates the layout from the given skin file.  The layout uses
    * the screen dimentions and rotation to calculate the dimensions of
    * the rendering buffer.
    */
   layout(const char* skin, Uint16 width, Uint16 height, int rotate);
   
   /**
    * Free resources (fonts, surfaces)
    */
   ~layout();

   /** Returns number of list items that fit in one page */
   const int page_size() const
   { return _page_size; }
   
   /**
    * Sets the current snapshot image
    */
   void snap(SDL_Surface* snap);
   
   /**
    * Returns a pointer to the rendering surface
    */
   SDL_Surface* buffer() const
   { return _buffer; }
   
   /**
    * Render the layout for the current menu
    */
   void render(menu* current);
};

} // end namespace

#endif
