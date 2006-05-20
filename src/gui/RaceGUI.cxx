//  $Id: RaceGUI.cxx,v 1.19 2005/09/05 17:27:06 joh Exp $
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <plib/pw.h>
#include "RaceGUI.h"
#include "../History.h"
#include "WidgetSet.h"
#include "World.h"
#include "Track.h"

#define TEXT_START_X  (config->width-220)
RaceGUI::RaceGUI(): time_left(0.0),
		    stats_enabled(false),
		    next_string(0)             {
  if(!config->profile) {
    UpdateKeyboardMappings();
  }   // if !config->profile

  fpsCounter = 0;
  pos_string[0] = "?!?";
  pos_string[1] = "1st";
  pos_string[2] = "2nd";
  pos_string[3] = "3rd";
  pos_string[4] = "4th";
  pos_string[5] = "5th";
  pos_string[6] = "6th";
  pos_string[7] = "7th";
  pos_string[8] = "8th";
  pos_string[9] = "9th";
  pos_string[10] = "10th";
  fpsTimer.reset();
  fpsTimer.setMaxDelta(1000);
  memset(tt, 0, sizeof(float) * 6);
  
}   // RaceGUI

// -----------------------------------------------------------------------------
RaceGUI::~RaceGUI() {
	//FIXME: does all that material stuff need freeing somehow?
}   // ~Racegui

// -----------------------------------------------------------------------------
void RaceGUI::UpdateKeyboardMappings() {
  // Defines the mappings for player keys to kart and action	
  // To avoid looping over all players to find out what
  // player control key was pressed, a special data structure 
  // is set up: keysToKArt contains for each (player assigned) 
  // key which kart it applies to (and therefore which player),
  // and typeForKey contains the assigned function of that key.
  for(int i=0; i<MAXKEYS; i++) {
    keysToKart[i]=0;
    typeForKey[i]=0;
  }
  
  for(int i=0; i<world->raceSetup.getNumPlayers(); i++) {
    PlayerKart* kart = world->getPlayerKart(i);
    Player*     p    = kart->getPlayer();
    keysToKart[p->keys[KC_WHEELIE]] = kart;
    keysToKart[p->keys[KC_JUMP]   ] = kart;
    keysToKart[p->keys[KC_RESCUE] ] = kart;
    keysToKart[p->keys[KC_FIRE]   ] = kart;
    typeForKey[p->keys[KC_WHEELIE]] = KC_WHEELIE;
    typeForKey[p->keys[KC_JUMP]   ] = KC_JUMP   ;
    typeForKey[p->keys[KC_RESCUE] ] = KC_RESCUE ;
    typeForKey[p->keys[KC_FIRE]   ] = KC_FIRE   ;
  }
}   // UpdateKeyControl

// -----------------------------------------------------------------------------
void RaceGUI::update(float dt) {
	drawStatusText (world->raceSetup) ;
}   // update

// -----------------------------------------------------------------------------
void RaceGUI::keybd(int key) {
  static int isWireframe = FALSE ;
  // Check if it's a user assigned key
  if(keysToKart[key]) {
    keysToKart[key]->action(typeForKey[key]);
  } else {
    switch ( key ) {
      case 0x12      : if(world->raceSetup.getNumPlayers()==1) {   // ctrl-r
                         Kart* kart = world->getPlayerKart(0);
			 kart->setCollectable((rand()%2)?COLLECT_MISSILE
					                :COLLECT_HOMING_MISSILE,
					      10000);
                        }
                       break;
      case PW_KEY_F12: config->displayFPS = !config->displayFPS ;
	               if(config->displayFPS) {
			 fpsTimer.reset();
			 fpsTimer.setMaxDelta(1000);
			 fpsCounter=0;
		       }
		       return;
      case PW_KEY_F11: glPolygonMode ( GL_FRONT_AND_BACK, 
				       isWireframe ? GL_FILL : GL_LINE);
	               isWireframe = ! isWireframe ;
		       return ;
      case 27:         widgetSet -> tgl_paused();    // ESC
	  	       guiStack.push_back(GUIS_RACEMENU);
		       // The player might have changed the keyboard 
		       // configuration, so we need to redefine the mappings
		       UpdateKeyboardMappings();
		       break;
//JH for debugging only.
    case PW_KEY_F10: history->Save(); return;
#ifdef DEBUG
      case PW_KEY_F10: stToggle () ; return ;
#endif
     default:   break;
    }   // switch
  }   // if(keysToKart[key] else
} // keybd

// -----------------------------------------------------------------------------
void RaceGUI::stick(const int &whichAxis, const float &value){
  KartControl controls;
  controls.data[whichAxis] = value;
  world -> getPlayerKart(0) -> incomingJoystick ( controls );
}   // stick

// -----------------------------------------------------------------------------
void RaceGUI::joybuttons( int whichJoy, int hold, int presses, int releases ) {
  KartControl controls;
  controls.buttons = hold;
  controls.presses = presses;
  controls.releases = releases;
  world -> getPlayerKart(whichJoy) -> incomingJoystick ( controls );
}   // joybuttons

// -----------------------------------------------------------------------------
void RaceGUI::drawFPS () {
  if (++fpsCounter>=10) {
    fpsTimer.update();
    sprintf(fpsString, "%d",(int)(fpsCounter/fpsTimer.getDeltaTime()));
    fpsCounter = 0;
    fpsTimer.setMaxDelta(1000);
  }    
  widgetSet->drawText (fpsString, 36, 0, config->height-36, 255, 255, 255 ) ;
}   // drawFPS

// -----------------------------------------------------------------------------
void RaceGUI::stToggle () {
  if ( stats_enabled )
    stats_enabled = FALSE ;
  else
  {
    stats_enabled = TRUE ;

    for ( int i = 0 ; i < MAX_STRING ; i++ )
       debug_strings [ i ][ 0 ] = '\0' ;

    next_string = 0 ;
  }
}   // stToggle

// -----------------------------------------------------------------------------
void RaceGUI::stPrintf ( char *fmt, ... ) {
  char *p = debug_strings [ next_string++ ] ;

  if ( next_string >= MAX_STRING )
    next_string = 0 ;

  va_list ap ;
  va_start ( ap, fmt ) ;
/*
  Ideally, we should use this:

     vsnprintf ( p, MAX_STRING_LENGTH, fmt, ap ) ;

  ...but it's only in Linux   :-(
*/

  vsprintf ( p, fmt, ap ) ;

  va_end ( ap ) ;
}   // stPrintf

// -----------------------------------------------------------------------------
void RaceGUI::drawInverseDropShadowText ( const char *str, int sz, 
					  int x, int y              ) {
  widgetSet->drawText ( str, sz, x, y, 255, 255, 255 ) ;
  widgetSet->drawText ( str, sz, x+1, y+1, 0, 0, 0 ) ;
}   // drawInverseDropShadowText

// -----------------------------------------------------------------------------
void RaceGUI::drawDropShadowText (const char *str, int sz, int x, int y ) {
  widgetSet->drawText ( str, sz, x, y, 0, 0, 0 ) ;
  widgetSet->drawText ( str, sz, x+1, y+1, 255, 255, 255 ) ;
}  // drawDropShadowText

// -----------------------------------------------------------------------------
void RaceGUI::drawTexture(const GLuint texture, int w, int h, 
			  int red, int green, int blue, int x, int y) {
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture);

  glColor3ub ( red, green, blue ) ;
  glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(x, (float)h+y);

    glTexCoord2f(1, 0);
    glVertex2f((float)w+x, (float)h+y);

    glTexCoord2f(1, 1);
    glVertex2f((float)w+x, y);

    glTexCoord2f(0, 1);
    glVertex2f(x, y);
  glEnd();

  glDisable(GL_TEXTURE_2D);
}   // drawTexture

// -----------------------------------------------------------------------------
void RaceGUI::drawTimer () {
  if(world->getPhase()!=World::RACE_PHASE) return;
  char str [ 256 ] ;

  time_left = world->clock;

  int min     = (int) floor ( time_left / 60.0 ) ;
  int sec     = (int) floor ( time_left - (double) ( 60 * min ) ) ;
  int tenths  = (int) floor ( 10.0f * (time_left - (double)(sec + 60*min)));

  sprintf ( str, "%d:%02d\"%d", min,  sec,  tenths ) ;
  drawDropShadowText ( str, 36, TEXT_START_X, config->height-80) ;
}   // drawTimer

// -----------------------------------------------------------------------------
void RaceGUI::drawScore (const RaceSetup& raceSetup, Kart* player_kart,
			 int offset_x, int offset_y, float ratio_x, 
			 float ratio_y                                 ) {
  char str [ 256 ] ;

  /* Show velocity */
  if ( player_kart->getVelocity()->xyz[1] < 0 )
    sprintf ( str, "Reverse" ) ;
  else {
    if(config->useKPH) {
      sprintf(str,"%d kph",
	      (int)(player_kart->getVelocity()->xyz[1]/KILOMETERS_PER_HOUR));
    } else {
      sprintf(str,"%d mph",
	      (int)(player_kart->getVelocity()->xyz[1]/MILES_PER_HOUR));
    }   // use KPH
  }   // velocity<0

  drawDropShadowText ( str, (int)(36*ratio_y), 
		       (int)(offset_x+TEXT_START_X        *ratio_x),
		       (int)(offset_y+(config->height-200)*ratio_y) );

  /* Show lap number */
  if ( player_kart->getLap() < 0 ) {
    //JH sprintf ( str, "Not Started Yet!" ) ;  Don't like this text; besides
    // it's messed up because of the wrong lap count anyway.
    sprintf ( str, " " ) ;  // One space to avoid warning about empty string
  }  else if ( player_kart->getLap() < raceSetup.numLaps - 1 ) {
    sprintf ( str, "Lap:%d/%d",
	      player_kart->getLap() + 1, raceSetup.numLaps ) ;
  } else {
    sprintf ( str, "Last Lap!" );
  }
  drawDropShadowText ( str, (int)(38*ratio_y), 
		       (int)(offset_x+TEXT_START_X        *ratio_x),
		       (int)(offset_y+(config->height-250)*ratio_y) );

  /* Show player's position */
  sprintf ( str, "%s", pos_string [ player_kart->getPosition() ] ) ;
  drawDropShadowText ( str, (int)(38*ratio_y), 
  		       (int)(offset_x+TEXT_START_X        *ratio_x), 
  		       (int)(offset_y+(config->height-300)*ratio_y) );
}   // drawScore

// -----------------------------------------------------------------------------
#define TRACKVIEW_SIZE 100

void RaceGUI::drawMap () {
  glDisable ( GL_TEXTURE_2D ) ;
  glColor3f ( 0,0,1 ) ;
  world -> track -> draw2Dview ( 430+TRACKVIEW_SIZE  , TRACKVIEW_SIZE   ) ;
  glColor3f ( 1,1,0 ) ;
  world -> track -> draw2Dview ( 430+TRACKVIEW_SIZE+1, TRACKVIEW_SIZE+1 ) ;

  glBegin ( GL_QUADS ) ;

  for ( int i = 0 ; i < world->getNumKarts() ; i++ ) {
    sgCoord *c ;

    Kart* kart = world->getKart(i);
    glColor3fv ( *kart->getColour());
    c          = kart->getCoord () ;

    world -> track->glVtx ( c->xyz, 430+TRACKVIEW_SIZE+3, TRACKVIEW_SIZE+3 ) ;
    world -> track->glVtx ( c->xyz, 430+TRACKVIEW_SIZE+0, TRACKVIEW_SIZE+3 ) ;
    world -> track->glVtx ( c->xyz, 430+TRACKVIEW_SIZE+0, TRACKVIEW_SIZE+0 ) ;
    world -> track->glVtx ( c->xyz, 430+TRACKVIEW_SIZE+3, TRACKVIEW_SIZE+0 ) ;
  }

  glEnd () ;
}   // drawMap

// -----------------------------------------------------------------------------
void RaceGUI::drawGameOverText () {
  static int timer = 0 ;

  /* Calculate a color. This will result in an animation effect. */
  int red   = (int)(255 * sin ( (float)timer/5.1f ) / 2.0f + 0.5f);
  int green = (int)(255 * (sin ( (float)timer/6.3f ) / 2.0f + 0.5f));
  int blue  = (int)(255 * sin ( (float)timer/7.2f ) / 2.0f + 0.5f);

  int finishing_position = world->getPlayerKart(0)->getFinishPosition();
  if ( finishing_position < 0 )
    finishing_position = world->getPlayerKart(0)->getPosition() ;

  if ( finishing_position > 1 ) {
    widgetSet->drawText ( "YOU FINISHED"    , 90, 130, 210, red, green, blue ) ;
    widgetSet->drawText ( pos_string [ finishing_position ], 90, 130, 210, 
			  red, green, blue ) ;
  } else {
    widgetSet->drawText ( "CONGRATULATIONS"  , 90, 130, 210, red, green, blue ) ;
    widgetSet->drawText ( "YOU WON THE RACE!", 90, 130, 210, red, green, blue ) ;
  }
}   // drawGameOverText

// -----------------------------------------------------------------------------
void RaceGUI::oldDrawPlayerIcons () {
  int   x =  0 ;
  int   y = 10 ;
  float w = 640.0f - 64.0f ;

    Material *last_players_gst=0;
    for(int i=0; i<world->getNumKarts(); i++) {
      x = (int) ( w * world->getKart(i) -> getDistanceDownTrack () /
		  world -> track -> getTrackLength () ) ;
      Material* players_gst =
	        world->getKart(i)->getKartProperties()->getIconMaterial();
      // Hmm - if the same icon is displayed more than once in a row,
      // plib does only do the first setTexture, therefore nothing is
      // displayed for the remaining icons. So we have to call force() if
      // the same icon is displayed more than once in a row.
      if(last_players_gst==players_gst) {
	players_gst->getState()->force();
      }
      players_gst -> apply ();
      last_players_gst=players_gst;
      glBegin ( GL_QUADS ) ;
        glColor4f    ( 1, 1, 1, 1 ) ;
	glTexCoord2f (  0, 0 ) ; glVertex2i ( x   , y    ) ;
	glTexCoord2f (  1, 0 ) ; glVertex2i ( x+64, y    ) ;
	glTexCoord2f (  1, 1 ) ; glVertex2i ( x+64, y+64 ) ;
	glTexCoord2f (  0, 1 ) ; glVertex2i ( x   , y+64 ) ;
      glEnd () ;

    }   // for i

}   // oldDrawPlayerIcons

// -----------------------------------------------------------------------------
void RaceGUI::drawPlayerIcons () {
  /** Draw players position on the race */

  int x = 10;
  int y;

  glEnable(GL_TEXTURE_2D);
  Material *last_players_gst = 0;
  for(int i = 0; i < world->getNumKarts() ; i++)
    {
      int position = world->getKart(i)->getPosition();
      if(position > 4)  // only draw the first four karts
        continue;

      y = config->width/2-20 - ((position-1)*(55+5));

      // draw text
      drawDropShadowText ( pos_string[position], 28, 55+x, y+10 ) ;

      // draw icon
      Material* players_gst =
	        world->getKart(i)->getKartProperties()->getIconMaterial();
      // Hmm - if the same icon is displayed more than once in a row,
      // plib does only do the first setTexture, therefore nothing is
      // displayed for the remaining icons. So we have to call force() if
      // the same icon is displayed more than once in a row.
      if(last_players_gst==players_gst) {
	players_gst->getState()->force();
      }
      //After calling apply the text appears aliased, since it seems the
      //icon material isn't appropiated for text, so all text output should
      //be done before this call.
      players_gst -> apply ();
      last_players_gst = players_gst;
      glBegin ( GL_QUADS ) ;
        glColor4f    ( 1, 1, 1, 1 ) ;

        glTexCoord2f ( 0, 0 ) ; glVertex2i ( x   , y    ) ;
        glTexCoord2f ( 1, 0 ) ; glVertex2i ( x+55, y    ) ;
        glTexCoord2f ( 1, 1 ) ; glVertex2i ( x+55, y+55 ) ;
        glTexCoord2f ( 0, 1 ) ; glVertex2i ( x   , y+55 ) ;
      glEnd () ;
    }
}   // drawPlayerIcons

// -----------------------------------------------------------------------------
void RaceGUI::drawEmergencyText (Kart* player_kart, int offset_x, 
				 int offset_y, float ratio_x, float ratio_y ) {
  static float wrong_timer = 0.0f ;
  static float last_dist = -1000000.0f ;
  static int last_lap = -1 ;

  float d = player_kart->getDistanceDownTrack () ;
  int   l = player_kart->getLap () ;

  if ( ( l < last_lap || ( l == last_lap && d < last_dist ) ) &&
       player_kart -> getVelocity () -> xyz [ 1 ] > 0.0f         ) {
    wrong_timer += 0.05f; // JH FIXME: was world->clock -> getDeltaTime () ;

    if ( wrong_timer > 2.0f ) {
      static int i = FALSE ;

      int red, green, blue;
      if ( i ) {
        red = blue = 255;
        green = 0;
      } else {
        red = blue = 0;
        green = 255;
      }

      widgetSet->drawText ( "WRONG WAY!", (int)(90*ratio_y), 
			    (int)(130*ratio_x)+offset_x,
			    (int)(210*ratio_y)+offset_y, red, green, blue ) ;
      if ( ! i ) {
        red = blue = 255;
        green = 0;
      } else {
        red = blue = 0;
        green = 255;
      }

      widgetSet->drawText ( "WRONG WAY!", (int)(90*ratio_y), 
			    (int)((130+2)*ratio_x)+offset_x,
			    (int)((210+2)*ratio_y)+offset_y, red, green, blue ) ;

      i = ! i ;
    }   // if wrong_timer>2.0
  } else
    wrong_timer = 0.0f ;

  last_dist = d ;
  last_lap  = l ;
}   //drawEmergencyText

// -----------------------------------------------------------------------------
void RaceGUI::drawCollectableIcons ( Kart* player_kart, int offset_x, 
				     int offset_y, float ratio_x, 
				     float ratio_y                    ) {
  int zz = FALSE ;
  // Originally the hardcoded sizes were 320-32 and 400
  int x1 = (int)((config->width/2-32) * ratio_x) + offset_x ;
  int y1 = (int)(config->height*5/6 * ratio_y)      + offset_y;

  // If player doesn't have anything, just let the transparent black square
  Collectable* collectable=player_kart->getCollectable();
  if(collectable->getType() == COLLECT_NOTHING) {
    glDisable(GL_TEXTURE_2D);
    glBegin ( GL_QUADS ) ;
      glColor4f ( 0.0, 0.0, 0.0, 0.16 ) ;
      glVertex2i ( x1                  , y1    ) ;
      glVertex2i ( x1+(int)(64*ratio_x), y1    ) ;
      glVertex2i ( x1+(int)(64*ratio_x), y1+(int)(64*ratio_y) ) ;
      glVertex2i ( x1                  , y1+(int)(64*ratio_y) ) ;
    glEnd();
    return;
  }
  collectable->getIcon()->apply();

  int n  = player_kart->getNumCollectables() ;

  if ( n > 5 ) n = 5 ;
  if ( n < 1 ) n = 1 ;

  glEnable(GL_TEXTURE_2D);

  glBegin ( GL_QUADS ) ;
    glColor4f    ( 1, 1, 1, 1 ) ;

    for ( int i = 0 ; i < n ; i++ ) {
      if ( zz ) {
	glTexCoord2f ( 0, 2 ) ; glVertex2i ( i*40 + x1                  , y1    ) ;
	glTexCoord2f ( 0, 0 ) ; glVertex2i ( i*40 + x1+(int)(32*ratio_x), y1    ) ;
	glTexCoord2f ( 2, 0 ) ; glVertex2i ( i*40 + x1+(int)(64*ratio_x), y1+(int)(32*ratio_y) ) ;
	glTexCoord2f ( 2, 2 ) ; glVertex2i ( i*40 + x1+(int)(32*ratio_x), y1+(int)(32*ratio_y) ) ;

	glTexCoord2f ( 0, 2 ) ; glVertex2i ( i*40 + x1+(int)(32*ratio_x), y1+(int)(32*ratio_y) ) ;
	glTexCoord2f ( 0, 0 ) ; glVertex2i ( i*40 + x1+(int)(64*ratio_x), y1+(int)(32*ratio_y) ) ;
	glTexCoord2f ( 2, 0 ) ; glVertex2i ( i*40 + x1+(int)(32*ratio_x), y1+(int)(64*ratio_y) ) ;
	glTexCoord2f ( 2, 2 ) ; glVertex2i ( i*40 + x1                  , y1+(int)(64*ratio_y) ) ;
      } else {
	glTexCoord2f ( 0, 0 ) ; glVertex2i ( i*30 + x1                  , y1    ) ;
	glTexCoord2f ( 1, 0 ) ; glVertex2i ( i*30 + x1+(int)(64*ratio_x), y1    ) ;
	glTexCoord2f ( 1, 1 ) ; glVertex2i ( i*30 + x1+(int)(64*ratio_x), y1+(int)(64*ratio_y) ) ;
	glTexCoord2f ( 0, 1 ) ; glVertex2i ( i*30 + x1                  , y1+(int)(64*ratio_y) ) ;
      }
    }   // for i
  glEnd () ;

  glDisable(GL_TEXTURE_2D);
}   // drawCollectableIcons

// -----------------------------------------------------------------------------
/* Energy meter that gets filled with coins */

// Meter fluid color (0 - 255)
#define METER_TOP_COLOR    230, 0, 0, 210
#define METER_BOTTOM_COLOR 240, 110, 110, 210 
// Meter border color (0.0 - 1.0)
#define METER_BORDER_COLOR 0.0, 0.0, 0.0

// -----------------------------------------------------------------------------
void RaceGUI::drawEnergyMeter ( Kart *player_kart, int offset_x, int offset_y, 
				float ratio_x, float ratio_y             ) {
  float state = (float)(player_kart->getNumHerring()) /
                        MAX_HERRING_EATEN;
  int x = (int)(config->width-50 * ratio_x) + offset_x;
  int y = (int)(config->height/4 * ratio_y) + offset_y;
  int w = (int)(24 * ratio_x);
  int h = (int)(config->height/2 * ratio_y);
  int wl = (int)(1 * ratio_x);
  if(wl < 1)
    wl = 1;

  // Draw a Meter border
  // left side
  glBegin ( GL_QUADS ) ;
  glColor3f ( METER_BORDER_COLOR ) ;
    glVertex2i ( x-wl, y-wl ) ;
    glVertex2i ( x,    y-wl ) ;
    glVertex2i ( x,    y + h) ;
    glVertex2i ( x-wl, y + h ) ;
  glEnd () ;

  // right side
  glBegin ( GL_QUADS ) ;
  glColor3f ( METER_BORDER_COLOR ) ;
    glVertex2i ( x+w,    y-wl ) ;
    glVertex2i ( x+w+wl, y-wl ) ;
    glVertex2i ( x+w+wl, y + h) ;
    glVertex2i ( x+w,    y + h ) ;
  glEnd () ;

  // down side
  glBegin ( GL_QUADS ) ;
  glColor3f ( METER_BORDER_COLOR ) ;
    glVertex2i ( x,   y-wl ) ;
    glVertex2i ( x+w, y-wl ) ;
    glVertex2i ( x+w, y ) ;
    glVertex2i ( x,   y ) ;
  glEnd () ;

  // up side
  glBegin ( GL_QUADS ) ;
  glColor3f ( METER_BORDER_COLOR ) ;
    glVertex2i ( x,   y+h ) ;
    glVertex2i ( x+w, y+h ) ;
    glVertex2i ( x+w, y+h+wl ) ;
    glVertex2i ( x,   y+h+wl ) ;
  glEnd () ;

  // Draw the Meter fluid
  glBegin ( GL_QUADS ) ;
  glColor4ub ( METER_TOP_COLOR ) ;
    glVertex2i ( x,   y ) ;
    glVertex2i ( x+w, y ) ;

  glColor4ub ( METER_BOTTOM_COLOR ) ;
    glVertex2i ( x+w, y + (int)(state * h));
    glVertex2i ( x,   y + (int)(state * h) ) ;
  glEnd () ;
}   // drawEnergyMeter


// -----------------------------------------------------------------------------
void RaceGUI::drawStatusText (const RaceSetup& raceSetup) {
  glMatrixMode   ( GL_MODELVIEW ) ;
  glPushMatrix   () ;
  glLoadIdentity () ;

  glMatrixMode   ( GL_PROJECTION ) ;
  glPushMatrix   () ;
  glLoadIdentity () ;

  glPushAttrib   ( GL_ENABLE_BIT | GL_LIGHTING_BIT ) ;
  glDisable      ( GL_DEPTH_TEST ) ;
  glDisable      ( GL_LIGHTING   ) ;
  glDisable      ( GL_FOG        ) ;
  glDisable      ( GL_CULL_FACE  ) ;
  glEnable       ( GL_ALPHA_TEST ) ;
  glAlphaFunc    ( GL_GREATER, 0.1 ) ;
  glEnable       ( GL_BLEND      ) ;

  glOrtho        ( 0, config->width, 0, config->height, 0, 100 ) ;
  switch (world->ready_set_go) {
    case 2: widgetSet->drawText ( "Ready!", 80, SCREEN_CENTERED_TEXT, 
				  SCREEN_CENTERED_TEXT, 230, 170, 160 ) ;
            break;
    case 1: widgetSet->drawText ( "Set!", 80, SCREEN_CENTERED_TEXT, 
				  SCREEN_CENTERED_TEXT, 230, 230, 160 ) ;
            break;
    case 0: widgetSet->drawText ( "Go!", 80, SCREEN_CENTERED_TEXT, 
				  SCREEN_CENTERED_TEXT, 100, 210, 100 ) ;
            break;
  }   // switch

  for(int i = 0; i < 10; ++i) {
    if(world->debugtext[i] != "")
      widgetSet->drawText(world->debugtext[i].c_str(), 20, 20, 200 - i*20, 
			  100, 210, 100);
  }
  if(world->getPhase()==World::START_PHASE) {
    for(int i=0; i<raceSetup.getNumPlayers(); i++) {
      if(world->getPlayerKart(i)->earlyStartPenalty()) {
	widgetSet->drawText("Penalty time!!",80, SCREEN_CENTERED_TEXT,
			    200, 200, 10, 10);
      }   // if penalty
    }  // for i < getNumPlayers
  }  // if not RACE_PHASE

  float split_screen_ratio_x, split_screen_ratio_y;
  split_screen_ratio_x = split_screen_ratio_y = 1.0;
  if(raceSetup.getNumPlayers() >= 2)
    split_screen_ratio_y = 0.5;
  if(raceSetup.getNumPlayers() >= 3)
    split_screen_ratio_x = 0.5;

  if ( world->getPhase() == World::FINISH_PHASE ) {
    drawGameOverText     () ;
  } else {
    for(int pla = 0; pla < raceSetup.getNumPlayers(); pla++) {
      int offset_x, offset_y;
      offset_x = offset_y = 0;
      if((pla == 0 && raceSetup.getNumPlayers() > 1) ||
          pla == 2)
	    offset_y = config->height/2;
      if((pla == 2 || pla == 3) && raceSetup.getNumPlayers() > 2)
	    offset_x = config->width/2;

      Kart* player_kart=world->getPlayerKart(pla);
      if(world->getPhase()==World::RACE_PHASE) {
        drawCollectableIcons( player_kart, offset_x, offset_y,
		  split_screen_ratio_x, split_screen_ratio_y ) ;
	    drawEnergyMeter( player_kart, offset_x, offset_y,
          split_screen_ratio_x, split_screen_ratio_y ) ;
        drawScore( raceSetup, player_kart, offset_x, offset_y,
          split_screen_ratio_x, split_screen_ratio_y ) ;
      }
      drawEmergencyText(player_kart, offset_x, offset_y,
			split_screen_ratio_x, split_screen_ratio_y ) ;
    }   // for pla

    if(world->getPhase()==World::RACE_PHASE) {

        drawTimer ();
      drawMap   ();
      if ( config->displayFPS ) drawFPS ();

      //      if(raceSetup.getNumPlayers() == 1) {
	  if(config->oldStatusDisplay) {
	    oldDrawPlayerIcons();
	  } else {
	    drawPlayerIcons() ;
	  }
	//      }   // if getNumPlayers==1

    }   // if RACE_PHASE
  }   // not game over

  glPopAttrib  () ;
  glPopMatrix  () ;
  glMatrixMode ( GL_MODELVIEW ) ;
  glPopMatrix  () ;
}   // drawStatusText

/* EOF */
