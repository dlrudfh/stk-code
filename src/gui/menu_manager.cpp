//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Patrick Ammann <pammann@aro.ch>
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

#include <cassert>

//This is needed in various platforms, but not all
# include <algorithm>

#include "menu_manager.hpp"

#include "main_menu.hpp"
#include "char_sel.hpp"
#include "difficulty.hpp"
#include "game_mode.hpp"
#include "options.hpp"
#include "track_sel.hpp"
#include "num_laps.hpp"
#include "num_players.hpp"
#include "config_controls.hpp"
#include "config_display.hpp"
#include "config_sound.hpp"
#include "player_controls.hpp"
#include "race_gui.hpp"
#include "race_results_gui.hpp"
#include "grand_prix_ending.hpp"
#include "race_manager.hpp"
#include "game_manager.hpp"
#include "race_menu.hpp"
#include "help_menu.hpp"
#include "credits_menu.hpp"
#include "grand_prix_select.hpp"
#include "sound_manager.hpp"
#include "sdldrv.hpp"
#include "user_config.hpp"
#include "widget_manager.hpp"

using namespace std;

MenuManager* menu_manager= new MenuManager();

MenuManager::MenuManager()
{
    m_current_menu = NULL;
    m_RaceGUI      = NULL;
    m_handled_size = 0;
}

//-----------------------------------------------------------------------------
MenuManager::~MenuManager()
{
    delete m_current_menu;
}

//-----------------------------------------------------------------------------
/** Puts the given menu into the menu stack and saves the widgetToken of
  * the last selected widget for later reactivation.
 */
void MenuManager::pushMenu(MenuManagerIDs id)
{
	// If there is already an element then this is the one for the menu
	// which is still visible. We store its currently selected widget
	// therein.
	if (m_menu_stack.size())
		m_menu_stack.back().second = widget_manager->get_selected_wgt();

	// used to suppress select-sound on startup
    static bool is_startup = true;

    if( MENUID_EXITGAME == id )
    {
        sound_manager->playSfx(SOUND_BACK_MENU);
    }
    else
    {
        if( !is_startup ) sound_manager->playSfx(SOUND_SELECT_MENU);
        else is_startup = false;
    }

	// Creates a new entry for the to be displayed menu.
	pair <MenuManagerIDs, int> element;
	element.first = id;
	element.second = WidgetManager::WGT_NONE;
	
    m_menu_stack.push_back(element);
}

//-----------------------------------------------------------------------------
void MenuManager::popMenu()
{
    sound_manager->playSfx(SOUND_BACK_MENU);
	
	m_menu_stack.pop_back();
}

//-----------------------------------------------------------------------------
void MenuManager::update()
{

    if (m_handled_size != m_menu_stack.size())
    {
        if (m_RaceGUI
            && m_current_menu == m_RaceGUI)
        {
          m_RaceGUI = 0;
          drv_setMode(MENU);
        }

        delete m_current_menu;
        m_current_menu= NULL;

        m_handled_size= m_menu_stack.size();
        if (m_handled_size > 0)
        {
			pair<MenuManagerIDs, int> saved = m_menu_stack.back();
            MenuManagerIDs id = saved.first;
			int saved_widget = saved.second;
			
            switch (id)
            {
            case MENUID_MAINMENU:
                m_current_menu= new MainMenu();
                break;
            case MENUID_CHARSEL_P1:
            case MENUID_CHARSEL_P2:
            case MENUID_CHARSEL_P3:
            case MENUID_CHARSEL_P4:
                m_current_menu= new CharSel(id - MENUID_CHARSEL_P1);
                break;
            case MENUID_DIFFICULTY:
                m_current_menu= new Difficulty();
                break;
            case MENUID_GAMEMODE:
                m_current_menu= new GameMode();
                break;
            case MENUID_OPTIONS:
                m_current_menu= new Options();
                break;
            case MENUID_TRACKSEL:
                m_current_menu= new TrackSel();
                break;
            case MENUID_NUMLAPS:
                m_current_menu= new NumLaps();
                break;
            case MENUID_NUMPLAYERS:
                m_current_menu= new NumPlayers();
                break;
            case MENUID_RACE:
                drv_setMode(INGAME);
                m_current_menu = new RaceGUI();
                m_RaceGUI      = m_current_menu;
                break;
            case MENUID_RACERESULT:
                m_current_menu= new RaceResultsGUI();
                break;
            case MENUID_GRANDPRIXEND:
                m_current_menu= new GrandPrixEnd();
                break;
            case MENUID_GRANDPRIXSELECT:
                m_current_menu= new GrandPrixSelect();
                break;
#if 0
            case MENUID_NEXTRACE:
                race_manager->next();
                break;
#endif
            case MENUID_RACEMENU:
                m_current_menu= new RaceMenu();
                break;
            case MENUID_EXITGAME:
                m_menu_stack.clear();
                game_manager->abort();
                break;

            case MENUID_CONFIG_CONTROLS:
                m_current_menu= new ConfigControls();
                break;
            case MENUID_CONFIG_P1:
            case MENUID_CONFIG_P2:
            case MENUID_CONFIG_P3:
            case MENUID_CONFIG_P4:
                m_current_menu= new PlayerControls(id - MENUID_CONFIG_P1);
                break;
            case MENUID_CONFIG_DISPLAY:
                m_current_menu= new ConfigDisplay();
                break;
            case MENUID_CONFIG_SOUND:
                m_current_menu= new ConfigSound();
                break;
            case MENUID_HELP:
                m_current_menu = new HelpMenu();
                break;
            case MENUID_CREDITS:
                m_current_menu = new CreditsMenu();
                break;
            default:
                break;
            }   // switch

			// Restores the previously selected widget if there was one.
			if (saved_widget != WidgetManager::WGT_NONE)
			{
				widget_manager->lighten_wgt_color( saved_widget );
				widget_manager->pulse_wgt( saved_widget );
				widget_manager->set_selected_wgt(saved_widget);
			} else if( widget_manager->get_selected_wgt() != WidgetManager::WGT_NONE )
            {
                widget_manager->lighten_wgt_color(widget_manager->get_selected_wgt());
            }
        }
    }

    static ulClock now  = ulClock();
    now.update();

    if (m_current_menu != NULL)
    {
        m_current_menu->update(now.getDeltaTime());
    }
}   // update

//-----------------------------------------------------------------------------
//Used to create a new instance of the present menu, which updates the widgets
//text and/or location, if they have been changed by selecting an entry in the menu. 
//eg: after a change of screen resolution

void MenuManager::refreshMenu()
{
        widget_manager->layout();
}

//-----------------------------------------------------------------------------
void MenuManager::switchToGrandPrixEnding()
{
    if (m_current_menu != NULL)
    {
        if(m_current_menu==m_RaceGUI) m_RaceGUI=0;
        delete m_current_menu;
        m_current_menu= NULL;
    }
    m_handled_size= 0;

    m_menu_stack.clear();
    pushMenu(MENUID_GRANDPRIXEND);
}

//-----------------------------------------------------------------------------
void MenuManager::switchToRace()
{
    m_menu_stack.clear();
    pushMenu(MENUID_RACE);
}

//-----------------------------------------------------------------------------
// Returns true if the id is somewhere on the stack. This can be used to detect
// if the config_display menu was called from the race_gui, or main_menu
bool MenuManager::isSomewhereOnStack(MenuManagerIDs id)
{
	for(vector<pair<MenuManagerIDs,int> >::iterator i = m_menu_stack.begin(); i != m_menu_stack.end(); i++)
	{
		if ((*i).first == id)
			return true;
	}
	
	return false;
}   // isSomewhereOnStack

// -----------------------------------------------------------------------------
void MenuManager::switchToMainMenu()
{
    if (m_current_menu != NULL)
    {
        if(m_current_menu==m_RaceGUI) m_RaceGUI=0;
        delete m_current_menu;
        m_current_menu= NULL;
    }
    m_handled_size= 0;

    m_menu_stack.clear();
    pushMenu(MENUID_MAINMENU);
}
