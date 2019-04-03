
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

//jzu
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
//jzu

#include "lemonmenu.h"
#include "game.h"
#include "options.h"
#include "error.h"

#include <cstring>
#include <confuse.h>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <SDL/SDL_rotozoom.h>

#include <bcm2835.h>
#include "gpio_joystick.h"


#define UPDATE_SNAP_EVENT 1

#include <typeinfo>

using namespace ll;
using namespace std;


uint32_t REMEMBEREDHASH=0x00000000;


/**
 * Timer callback function
 */
Uint32 snap_timer_callback(Uint32 interval, void *param);

// hash string to uint32
uint32_t jenkins_one_at_a_time_hash(char *key, size_t len);

/**
 * Compares the text property of two item pointers and returns true if the left
 * is less than the right.
 */
bool cmp_item(item* left, item* right)
{ return strcmp(left->text(), right->text()) < 0; }

lemon_menu::lemon_menu(SDL_Surface* screen) :
   _screen(screen), _show_hidden(false), _snap_timer(0),
   _snap_delay(g_opts.get_int(KEY_SNAPSHOT_DELAY)),
   _rotate(g_opts.get_int(KEY_ROTATE))
{
   _layout = new layout(
         g_opts.get_string(KEY_SKIN_FILE),
         g_opts.get_int(KEY_SCREEN_WIDTH),
         g_opts.get_int(KEY_SCREEN_HEIGHT),
         _rotate);

   load_menus();
}

lemon_menu::~lemon_menu()
{
   delete _layout;
   delete _top; // delete top menu will propigate to children
}
   
void lemon_menu::load_menus()
{
   cfg_opt_t game_opts[] = {
      CFG_STR("rom", 0, CFGF_NODEFAULT),
      CFG_STR("title", 0, CFGF_NODEFAULT),
      CFG_STR("params", "", CFGF_NONE),
      CFG_STR("orientation", "h", CFGF_NONE),      //CM: default orientation is h
      CFG_END()
   };

   cfg_opt_t menu_opts[] = {
      CFG_BOOL("sorted", cfg_true, CFGF_NONE),
      CFG_SEC("game", game_opts, CFGF_MULTI),
      CFG_END()
   };

   cfg_opt_t root_opts[] = {
      CFG_SEC("menu", menu_opts, CFGF_TITLE | CFGF_MULTI),
      CFG_END()
   };

   //cfg_opt_t opts[] = {
   //   CFG_SEC("root", root_opts, CFGF_TITLE),
   //   CFG_END()
   //};

   cfg_t* cfg = cfg_init(root_opts, CFGF_NONE);
   
   game* startgame;
   menu* startmenu;
   
   string cfg_file("games.conf");
   g_opts.resolve(cfg_file);
   
   int result = cfg_parse(cfg, cfg_file.c_str());

   if (result == CFG_FILE_ERROR) {
      // file error usually means file not found, warn and load empty menu
      log << warn << "load_menus: file error, using defaults" << endl;
      cfg_parse_buf(cfg, "");
   } else if (result == CFG_PARSE_ERROR) {
      throw bad_lemon("load_menus: parse error");
   }

   // only one root supported for now, should be straight forward to support more
   //cfg_t* root = cfg_getsec(cfg, "root");

   //_top = new menu(cfg_title(root));
   _top = new menu("PI2JAMMA");

   int startgamecount,gamecount;
   // iterate over menu sections
   int menu_cnt = cfg_size(cfg, "menu");
   for (int i=0; i<menu_cnt; i++) {
      cfg_t* m = cfg_getnsec(cfg, "menu", i);

      const char* mtitle = cfg_title(m);
      bool sorted = cfg_getbool(m, "sorted") == cfg_true;

      // "should" be safe to assume title is at least one charcter long
      if (mtitle[0] != '.' || _show_hidden) {

         // create menu and add to root menu
         menu* pmenu = new menu(mtitle);
         gamecount=0;
         // iterate over the game sections
         int game_cnt = cfg_size(m, "game");
         if (game_cnt > 0)
            _top->add_child(pmenu);
         
         for (int j=0; j<game_cnt; j++) {
            cfg_t* g = cfg_getnsec(m, "game", j);

            char* rom = cfg_getstr(g, "rom");
            char* title = cfg_getstr(g, "title");
            char* params = cfg_getstr(g, "params");
            char* orientation = cfg_getstr(g, "orientation");

            if (title[0] != '.' || _show_hidden)
          {
        if( (strlen(g_opts.get_string(KEY_ORIENTATIONFILTER)) == 0 ) || (strcmp(orientation, g_opts.get_string(KEY_ORIENTATIONFILTER) ) == 0) || (strcmp(orientation, "a" ) == 0) )
          {
            gamecount++;
            game* newgame;

            newgame=new game(rom, title, params, orientation);
            pmenu->add_child(newgame);
            // pmenu->add_child(new game(rom, title, params, orientation));
            
          }
          }        
         }

         // sort the menu alphabeticly using game/item name
         if (sorted)
            sort(pmenu->first(), pmenu->last(), cmp_item);
       }
    }

   // always sort top menu
   //   sort(_top->first(), _top->last(), cmp_item);

   _current = _top;

   cfg_free(cfg);

   // restart at last selection?
   if(strcmp("y", g_opts.get_string(KEY_REMEMBERLAST) ) == 0)
     {    
       FILE *fp;
       fp = fopen( g_opts.get_string(KEY_REMEMBERLASTFILE)  , "r+");
       if(fp)
     {
       fread(&REMEMBEREDHASH, sizeof(REMEMBEREDHASH) ,1,fp);
       fclose(fp);
     }
     }       


    // printf ("rhash %04X\n",REMEMBEREDHASH );
    
    if(REMEMBEREDHASH != 0x00000000 ) {
       

        // loop over all menus and select entry with corresponding hash
        // entries are mixed up, because of sort!
        // so we have to loop over all of them to select a special one

        bool stop=false;
        vector<menu*>::iterator j;

        for ( auto j = _top->first(); (j != _top->last()) && !stop  ; j++ ) {
           game* g;
           menu* m;

            m = static_cast<menu*>(*j);
            _current = m;
            
            //cout << "typ: " <<typeid(j).name() << '\n' ;
            //cout << "typ: " <<typeid(*j).name() << '\n' ;
            //cout << "typ: " <<typeid(m).name() << '\n' ;
                     
            if( m->has_children() ) {
               g= static_cast<game*>( *(m->first()) );

                for(; (m->select_next()) && !stop ;) {
                    uint32_t hash=0L;
                    char *str;

                    game* ga = (game*)m->selected();
                    if (typeid(game) == typeid(*ga))
                    {
                        asprintf(&str, "%s%s%s", (char *)ga->text(), (char *)ga->params() , (char *)ga->rom() ) ;

                        hash=jenkins_one_at_a_time_hash(str, strlen(str));
                        free(str);
              
                        if(hash==REMEMBEREDHASH) {
                        //printf("Gefunden\n");
                            stop=true;
                            m->select_previous();  // einen zurueck
                        }
                    }
                }
                if(!stop)
                    m->select_reset();       
            }
            //eintrag nicht gefunden, zurueck auf startmenu und ersten Eintrag
            if(!stop) {
                _top->select_reset();
                _current = _top;
            }
        }
    }
}

void lemon_menu::render()
{
    // render ui
    _layout->render(_current);

    if (_rotate != 0) {
     
        SDL_Surface* tmp = rotozoomSurface(_layout->buffer(), _rotate, 1, 0);

        //CM: hack, sonst Stoerung in snaps (warum tritt das nur bei Drehung auf?)
        SDL_FillRect(_screen, NULL, 0 );

        SDL_BlitSurface(tmp, NULL, _screen, NULL);
        SDL_UpdateRect(_screen, 0, 0, 0, 0);
        SDL_FreeSurface(tmp);

    } else {
        SDL_BlitSurface(_layout->buffer(), NULL, _screen, NULL);
        SDL_UpdateRect(_screen, 0, 0, 0, 0);
    }
}


#define WAIT1 for(waitcount=20;waitcount--;waitcount>1 );
volatile unsigned long waitcount;


#define NRHC165BITS 24


long unsigned int Read_Port_HC165()
{
    long unsigned int val=0xffffffff;
    int i;

    bcm2835_gpio_write(CLK, HIGH);
    WAIT1
    bcm2835_gpio_write(PL, LOW);
    WAIT1
    bcm2835_gpio_write(PL, HIGH);

    for(i=0;i<NRHC165BITS;i++) {
        val= val<<1 ;
        WAIT1
        uint8_t value = bcm2835_gpio_lev(DIN);
        val=val|value;
        bcm2835_gpio_write(CLK, LOW);
        WAIT1
        bcm2835_gpio_write(CLK, HIGH);
    }

    return val;
}

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))


long unsigned int oldgpiokeyval=0;

// hash string to uint32
uint32_t jenkins_one_at_a_time_hash(char *key, size_t len)
  {
    uint32_t hash, i;
    for(hash = i = 0; i < len; ++i)
      {
    hash += key[i];
    hash += (hash << 10);
    hash ^= (hash >> 6);
      }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
  }

#include <unistd.h>

void lemon_menu::main_loop()
{
    log << info << "main_loop: starting render loop" << endl;

    render();
    reset_snap_timer();

    long unsigned int val;
     
    const int exit_key = g_opts.get_int(KEY_KEYCODE_EXIT);
    const int up_key = g_opts.get_int(KEY_KEYCODE_UP);
    const int down_key = g_opts.get_int(KEY_KEYCODE_DOWN);
    const int pgup_key = g_opts.get_int(KEY_KEYCODE_PGUP);
    const int pgdown_key = g_opts.get_int(KEY_KEYCODE_PGDOWN);
    const int reload_key = g_opts.get_int(KEY_KEYCODE_RELOAD);
    const int toggle_key = g_opts.get_int(KEY_KEYCODE_TOGGLE);
    const int select_key = g_opts.get_int(KEY_KEYCODE_SELECT);
    const int back_key = g_opts.get_int(KEY_KEYCODE_BACK);
    const int alphamod = g_opts.get_int(KEY_KEYCODE_ALPHAMOD);
    const int fav = g_opts.get_int(KEY_KEYCODE_FAV);


    const int up_joy = g_opts.get_int(JOY_UP);
    const int down_joy = g_opts.get_int(JOY_DOWN);
    const int pgup_joy = g_opts.get_int(JOY_PGUP);
    const int pgdown_joy = g_opts.get_int(JOY_PGDOWN);
    const int up_button_joy = g_opts.get_int(JOY_BUTTON_UP);
    const int down_button_joy = g_opts.get_int(JOY_BUTTON_DOWN);
    const int pgup_button_joy = g_opts.get_int(JOY_BUTTON_PGUP);
    const int pgdown_button_joy = g_opts.get_int(JOY_BUTTON_PGDOWN);
    const int select_joy = g_opts.get_int(JOY_SELECT);
    const int back_joy = g_opts.get_int(JOY_BACK);
    const int alphamod_down_joy = g_opts.get_int(JOY_ALPHAMOD_UP);
    const int alphamod_up_joy = g_opts.get_int(JOY_ALPHAMOD_DOWN);
    const int fav_joy = g_opts.get_int(JOY_FAV);


    //CM delay fuer hardware joystick
    int sleep_delay;
    int alphamodflag=0;
   
    sleep_delay=g_opts.get_int(KEY_PI2JAMMAJOYSTICK_DELAY); 
    if(sleep_delay<50) sleep_delay=50;
    if(sleep_delay>5000) sleep_delay=5000;
   
   
    //printf("sleep_day: %i\n",sleep_delay);
   
    //jzu : USB Joystick
    SDL_Joystick *joy;
    SDL_JoystickEventState(SDL_ENABLE);

    // Open joystick
    joy=SDL_JoystickOpen(0);
  
//    if(joy)
//    {
         //printf("Opened Joystick 0\n");
         //printf("Name: %s\n", SDL_JoystickName(0));
         //printf("Number of Axes: %d\n", SDL_JoystickNumAxes(joy));
         //printf("Number of Buttons: %d\n", SDL_JoystickNumButtons(joy));
         //printf("Number of Balls: %d\n", SDL_JoystickNumBalls(joy));
//    }
//    else
         //printf("Couldn't open Joystick 0\n");
  


    _running = true;
    while (_running) {
        SDL_Event event;
        while(SDL_PollEvent(&event) != 0) {
      
            //SDL_WaitEvent(&event);
            SDLKey key = event.key.keysym.sym;
            SDLMod mod = event.key.keysym.mod;
            
            switch (event.type) {
                case SDL_QUIT:
                    _running = false;
                    break;
                case SDL_KEYUP:
                    if (key == exit_key) {
                        _running = false;
                    } else if (key == select_key) {
                        handle_activate();
                    } else if (key == fav) {
                        handle_fav();
                    } else if (key == back_key) {
                        handle_up_menu();
                    } else if (key == reload_key) {
                        load_menus();
                        render();
                    } else if (key == toggle_key) {
                        handle_show_hide();
                    //CM
                    } else if (key == alphamod) {
                       alphamodflag=0;
                    }

                    break;
                case SDL_KEYDOWN:
                    if (key == up_key) {
                        handle_up();
                    } else if (key == down_key) {
                        handle_down();
                    //} else if (key == pgup_key && mod & alphamod) {
                    } else if (key == pgup_key & alphamodflag) {
                        handle_alphadown();
                    //} else if (key == pgdown_key && mod & alphamod) {
                    } else if (key == pgdown_key & alphamodflag) {
                        handle_alphaup();
                    } else if (key == pgup_key) {
                        handle_pgup();
                    } else if (key == pgdown_key) {
                        handle_pgdown();
                    //CM
                    } else if (key == alphamod) {
                        alphamodflag=1;
                    }
                    break;

//JZU

            case SDL_JOYAXISMOTION:
                if( event.jaxis.axis == pgup_joy ) {
                    if( event.jaxis.value < -32000 ) { 
                        //printf("left\n");
                        handle_pgup();
                    } 
                    else if ( event.jaxis.value > 32000 ) { 
                        //printf("right\n");
                        handle_pgdown(); 
                    } 
                    else { 
                        //printf("else axis 0\n"); 
                    }
                }
                if( event.jaxis.axis == up_joy ) {
                    if( event.jaxis.value < -32000 ) { 
                        //printf("up\n");
                        handle_up();
                    }  
                    else if( event.jaxis.value > 32000 ) { 
                         //printf("down\n");
                         handle_down(); 
                     } 
                     else { 
                         //printf("else axis 1\n"); 
                     }
                }
                break;
            case SDL_JOYBUTTONDOWN:  /* Handle Joystick Button Presses */
                if ( event.jbutton.button == back_joy ) {
                    //printf("button 0  = X\n");
                    handle_up_menu();
                }
                if ( event.jbutton.button == select_joy ) {
                    //printf("button 2 = B\n");
                    handle_activate();
                }
                if ( event.jbutton.button == alphamod_down_joy ) {
                    //printf("button 4 = L\n");
                    handle_alphadown();
                }
                if ( event.jbutton.button == alphamod_up_joy ) {
                    //printf("button 6 = R\n");
                    handle_alphaup();
                }
                if ( event.jbutton.button == up_button_joy ) {
                    handle_up(); 
                }
                if ( event.jbutton.button == down_button_joy ) {
                    handle_down(); 
                }
                if ( event.jbutton.button == pgup_button_joy ) {
                    handle_pgup(); 
                }
                if ( event.jbutton.button == pgdown_button_joy ) {
                    handle_pgdown(); 
                }
                if ( event.jbutton.button == fav_joy ) {
                    handle_fav(); 
                }
                break;

            case SDL_JOYHATMOTION:
                if (event.jhat.value & SDL_HAT_UP)
                    handle_up(); 

                if (event.jhat.value & SDL_HAT_RIGHT)
                    handle_pgdown(); 

                if (event.jhat.value & SDL_HAT_DOWN)
                    handle_down(); 

                if (event.jhat.value & SDL_HAT_LEFT)
                    handle_pgup(); 
                break;
//JZU
            case SDL_USEREVENT:
                if (event.user.code == UPDATE_SNAP_EVENT)
                    update_snap();
                break;
        }
    }
        

    //printf("joystring: %s\n",g_opts.get_string(KEY_PI2JAMMAJOYSTICK));
      
    if(strcmp("y", g_opts.get_string(KEY_PI2JAMMAJOYSTICK) ) == 0) {    
        val=Read_Port_HC165();
        if ( !CHECK_BIT(val,18) )
            handle_up();
        if ( !CHECK_BIT(val,19) )
            handle_down();
      //      if ( !CHECK_BIT(val,8) )
      //    handle_pgup();
      //      if ( !CHECK_BIT(val,9) )
      //    handle_pgdown();


        if ( !CHECK_BIT(val,9) ) {
            if ( !CHECK_BIT(val,0) ) {
                handle_alphaup();         // alphab blaetern mit p1b3 pressed
            }
            else {
                handle_pgdown();
            }
        }


        if ( !CHECK_BIT(val,8) ) {
            if ( !CHECK_BIT(val,0) ) {
                handle_alphadown();       // alphab blaetern mit p1b3 pressed
            }
            else {
                handle_pgup();
            }
        }

      
        if ( !CHECK_BIT(val,10) )
            handle_activate();
        if ( !CHECK_BIT(val,11) )
            handle_up_menu();

        if( !CHECK_BIT(val,11) && !CHECK_BIT(val,17) && !CHECK_BIT(val,19) ) //p1b2&p1start&p1down -> escape
            _running = false;
      
    }
    
    //         _current->last ();
      // calculate hash and remember it
      uint32_t hash=0L;      
      char *str;
      game* ga = (game*)_current->selected();
      if (typeid(game) == typeid(*ga))
    {
      //      printf ("x %s\n ",ga->text() );
      //      printf ("x %s\n ",ga->params() );
      //      printf ("x %s\n ",ga->rom() );      
      asprintf(&str, "%s%s%s", (char *)ga->text(), (char *)ga->params() , (char *)ga->rom() ) ;
            //printf ("%s\n ",str );
      hash=jenkins_one_at_a_time_hash(str, strlen(str));
      //printf ("hash %04X\n",hash );
      REMEMBEREDHASH=hash;
      free(str);
    }
      
      
    usleep(sleep_delay*1000);        
}


reset_snap_timer();
}

void lemon_menu::handle_fav()
{
    if (!_current->has_children()) return;

    item* item = _current->selected();

    if (typeid(game) == typeid(*item)) {

        system("pkill -9 omxplayer"); 

        game* g = (game*)_current->selected();
        string cmd(g_opts.get_string(KEY_MAME_PATH));

        // this is required when lemon launcher is full screen for some reason
        // otherwise mame freezes and all the processes have to be kill manually
        bool full = g_opts.get_bool(KEY_FULLSCREEN);
        if (full) SDL_WM_ToggleFullScreen(_screen);

        size_t pos = cmd.find("%r");
        if (pos == string::npos)
            throw bad_lemon("mame path missing %r specifier");

        cmd.replace(pos, 2, g->rom());
        cmd.append(" ");
        cmd.append(g->params());

        std::cout << "Lemon_fav: " << cmd << std::endl;

        // CM stop Menu and exit 
        _running = false;

        // letzte selction merken
        if(strcmp("y", g_opts.get_string(KEY_REMEMBERLAST) ) == 0)
        {
            FILE *fp;
            fp = fopen( g_opts.get_string(KEY_REMEMBERLASTFILE)  , "w+");
            if(fp) 
            {
                fwrite(&REMEMBEREDHASH, sizeof(REMEMBEREDHASH) ,1,fp);
                fclose(fp);
            }
        }
            
        if (full) SDL_WM_ToggleFullScreen(_screen);

        // clear the event queue
        SDL_Event event;
        while (SDL_PollEvent(&event));

        render();

    }
}

void lemon_menu::handle_up()
{
   // ignore event if already at the top of menu
   if (_current->select_previous()) {
      reset_snap_timer();
      render();
   }
}

void lemon_menu::handle_down()
{
   // ignore event if already at the bottom of menu
   if (_current->select_next()) {
      reset_snap_timer();
      render();
   }
}

void lemon_menu::handle_pgup()
{
   // ignore event if already at the top of menu
   if (_current->select_previous(_layout->page_size())) {
      reset_snap_timer();
      render();
   }
}

void lemon_menu::handle_pgdown()
{
   // ignore event if already at the bottom of menu
   if (_current->select_next(_layout->page_size())) {
      reset_snap_timer();
      render();
   }
}

void lemon_menu::handle_alphaup()
{
   if (_current->select_next_alpha()) {
      reset_snap_timer();
      render();
   }
}

void lemon_menu::handle_alphadown()
{
   if (_current->select_previous_alpha()) {
      reset_snap_timer();
      render();
   }
}

void lemon_menu::handle_activate()
{
   if (!_current->has_children()) return;

   item* item = _current->selected();
   if (typeid(menu) == typeid(*item)) {
      handle_down_menu();
   } else if (typeid(game) == typeid(*item)) {
      handle_run();
   }
}

void lemon_menu::handle_run()
{
   system("pkill -9 omxplayer"); 

   game* g = (game*)_current->selected();
   string cmd(g_opts.get_string(KEY_MAME_PATH));

   log << info << "handle_run: launching game " << g->text() << endl;

   // this is required when lemon launcher is full screen for some reason
   // otherwise mame freezes and all the processes have to be kill manually
   bool full = g_opts.get_bool(KEY_FULLSCREEN);
   if (full) SDL_WM_ToggleFullScreen(_screen);

   size_t pos = cmd.find("%r");
   if (pos == string::npos)
      throw bad_lemon("mame path missing %r specifier");

   cmd.replace(pos, 2, g->rom());
   cmd.append(" ");
   cmd.append(g->params());

   log << debug << "handle_run: " << cmd << endl;

   std::cout << "Lemonresult: " << cmd << std::endl;


   //do not start
   //system(cmd.c_str());

   // CM stop Menu and exit 
   _running = false;

   // letzte selction merken
   if(strcmp("y", g_opts.get_string(KEY_REMEMBERLAST) ) == 0)
     {
       FILE *fp;
       fp = fopen( g_opts.get_string(KEY_REMEMBERLASTFILE)  , "w+");
       if(fp)
     {
       fwrite(&REMEMBEREDHASH, sizeof(REMEMBEREDHASH) ,1,fp);
       fclose(fp);
     }
     }
        
   if (full) SDL_WM_ToggleFullScreen(_screen);

   // clear the event queue
   SDL_Event event;
   while (SDL_PollEvent(&event));

   render();
}

void lemon_menu::handle_up_menu()
{
   system("pkill -9 omxplayer"); 

   if (_current != _top) {
      _current = (menu*)_current->parent();
      reset_snap_timer();
      render();
   }
}

void lemon_menu::handle_down_menu()
{
   _current = (menu*)_current->selected();
   reset_snap_timer();
   render();
}

void lemon_menu::handle_show_hide()
{
   log << info << "handle_show_hide: changing hidden status" << endl;

   _show_hidden = !_show_hidden;
   load_menus();
   render();
}

void lemon_menu::update_snap()
{
   if (_current->has_children()) {
      item* item = _current->selected();
      _layout->snap(item->snapshot());
      render();
      //system("pkill -9 omxplayer"); 
      //system("omxplayer --win \"100 100 320 240\" /root/w.mp4 &"); 

   }

}

void lemon_menu::reset_snap_timer()
{
   if (_snap_timer)
      SDL_RemoveTimer(_snap_timer);

   // schedule timer to run in 500 milliseconds
   _snap_timer = SDL_AddTimer(_snap_delay, snap_timer_callback, NULL);
}

Uint32 snap_timer_callback(Uint32 interval, void *param)
{
   SDL_Event evt;
   evt.type = SDL_USEREVENT;
   evt.user.code = UPDATE_SNAP_EVENT;

   SDL_PushEvent(&evt);

   return 0;
}

