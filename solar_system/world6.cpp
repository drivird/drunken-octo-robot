/*
 * world6.cpp
 *
 *  Created on: 2012-07-05
 *      Author: dri
 */

#include "texturePool.h"
#include "cIntervalManager.h"
#include "world6.h"

// We start this tutorial with the standard class. However, the class is a
// subclass of an object called DirectObject. This gives the class the ability
// to listen for and respond to events. From now on the main class in every
// tutorial will be a subclass of DirectObject

// Macro-like function used to reduce the amount to code needed to create the
// on screen instructions
COnscreenText World6::gen_label_text(const string& text, int i) const
   {
   COnscreenText label("label");
   label.set_text(text);
   label.set_pos(LVecBase2f(-1.3, 0.95-0.05*i));
   label.set_fg(Colorf(1, 1, 1, 1));
   label.set_align(TextNode::A_left);
   label.set_scale(0.05);
   label.reparent_to(m_windowFramework->get_aspect_2d());
   return label;
   }

// The initialization method caused when a world object is created
World6::World6(WindowFramework* windowFrameworkPtr)
   : m_windowFramework(windowFrameworkPtr),
     m_title("title", COnscreenText::TS_plain),
     m_sizescale(1),
     m_sky(),
     m_skyTex(NULL),
     m_sun(),
     m_sunTex(NULL),
     m_orbitscale(1),
     m_orbitRootMercury(),
     m_orbitRootVenus(),
     m_orbitRootMars(),
     m_orbitRootEarth(),
     m_orbitRootMoon(),
     m_mercury(),
     m_mercuryTex(NULL),
     m_venus(),
     m_venusTex(NULL),
     m_mars(),
     m_marsTex(NULL),
     m_earth(),
     m_earthTex(NULL),
     m_moon(),
     m_moonTex(NULL),
     m_yearscale(1),
     m_dayscale(1),
     m_dayPeriodSun(NULL),
     m_orbitPeriodMercury(NULL),
     m_dayPeriodMercury(NULL),
     m_orbitPeriodVenus(NULL),
     m_dayPeriodVenus(NULL),
     m_orbitPeriodEarth(NULL),
     m_dayPeriodEarth(NULL),
     m_orbitPeriodMoon(NULL),
     m_dayPeriodMoon(NULL),
     m_orbitPeriodMars(NULL),
     m_dayPeriodMars(NULL),
     m_mouse1EventText("label"),
     m_skeyEventText("label"),
     m_ykeyEventText("label"),
     m_vkeyEventText("label"),
     m_ekeyEventText("label"),
     m_mkeyEventText("label"),
     m_yearCounterText("label"),
     m_yearCounter(0),
     m_simRunning(true)
   {
   // The standard camera position and background initialization
   m_windowFramework->get_graphics_window()->get_active_display_region(0)->
      set_clear_color(Colorf(0, 0, 0, 0));
   // base.disableMouse() // Note: mouse ain't enable by default in C++
   NodePath camera = m_windowFramework->get_camera_group();
   camera.set_pos(0, 0, 45);
   camera.set_hpr(0, -90, 0);

   // The global variables we used to control the speed and size of objects
   m_yearscale = 60;
   m_dayscale = m_yearscale / 365.0 * 5;
   m_orbitscale = 10;
   m_sizescale = 0.6;

   load_planets();            // Load, texture, and position the planets
   rotate_planets();          // Set up the motion to start them moving

   // The standard title text that's in every tutorial
   // Things to note:
   // -fg represents the forground color of the text in (r,g,b,a) format
   // -pos  represents the position of the text on the screen.
   //       The coordinate system is a x-y based wih 0,0 as the center of the
   //       screen
   // -align sets the alingment of the text relative to the pos argument.
   //       Default is center align.
   // -scale set the scale of the text
   // -mayChange argument lets us change the text later in the program.
   //        By default mayChange is set to 0. Trying to change text when
   //        mayChange is set to 0 will cause the program to crash.
   m_title.set_text("Panda3D: Tutorial 1 - Solar System");
   m_title.set_fg(Colorf(1, 1, 1, 1));
   m_title.set_pos(LVecBase2f(0.8, -0.95));
   m_title.set_scale(0.07);
   m_title.reparent_to(m_windowFramework->get_aspect_2d());
   m_mouse1EventText = gen_label_text(
     "Mouse Button 1: Toggle entire Solar System [RUNNING]", 0);
   m_skeyEventText = gen_label_text("[S]: Toggle Sun [RUNNING]", 1);
   m_ykeyEventText = gen_label_text("[Y]: Toggle Mercury [RUNNING]", 2);
   m_vkeyEventText = gen_label_text("[V]: Toggle Venus [RUNNING]", 3);
   m_ekeyEventText = gen_label_text("[E]: Toggle Earth [RUNNING]", 4);
   m_mkeyEventText = gen_label_text("[M]: Toggle Mars [RUNNING]", 5);
   m_yearCounterText = gen_label_text("0 Earth years completed", 6);

   m_yearCounter = 0;            // year counter for earth years
   m_simRunning = true;          // boolean to keep track of the
                                    // state of the global simulation

   // Events
   // Each self.accept statement creates an event handler object that will call
   // the specified function when that event occurs.
   // Certain events like "mouse1", "a", "b", "c" ... "z", "1", "2", "3"..."0"
   // are references to keyboard keys and mouse buttons. You can also define
   // your own events to be used within your program. In this tutorial, the
   // event "newYear" is not tied to a physical input device, but rather
   // is sent by the function that rotates the Earth whenever a revolution
   // completes to tell the counter to update
   // Note: need to listen to input events
   m_windowFramework->enable_keyboard();
   PandaFramework* pfw = m_windowFramework->get_panda_framework();
   // Exit the program when escape is pressed
   pfw->define_key("escape", "sysExit", sys_exit, NULL);
   pfw->define_key("mouse1", "handleMouseClick", call_handle_mouse_click, this);
   pfw->define_key("e", "handleEarth", call_handle_earth, this);
   pfw->define_key("s",                   // message name
         "togglePlanetSun",           // Note: event description
         call_toggle_planet<P_sun>,   // function to call
         this);                       // arguments to be passed to togglePlanet
                                      // See togglePlanet's definition below for
                                      // an explanation of what they are
   // Repeat the structure above for the other planets
   pfw->define_key("y", "togglePlanetMercury",
      call_toggle_planet<P_mercury>, this);
   pfw->define_key("v", "togglePlanetVenus",
      call_toggle_planet<P_venus>, this);
   pfw->define_key("m", "togglePlanetMars",
      call_toggle_planet<P_mars>, this);
   EventHandler::get_global_event_handler()->add_hook("newYear",
                                                       call_inc_year,
                                                       this);
   }

void World6::call_handle_mouse_click(const Event* eventPtr, void* dataPtr)
   {
   // Precondition
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<World6*>(dataPtr)->handle_mouse_click();
   }

void World6::handle_mouse_click()
   {
   // When the mouse is clicked, if the simulation is running pause all the
   // planets and sun, otherwise resume it
   if(m_simRunning)
      {
      nout << "Pausing Simulation" << endl;
      // changing the text to reflect the change from "RUNNING" to "PAUSED"
      m_mouse1EventText.set_text(
         "Mouse Button 1: Toggle entire Solar System [PAUSED]");
      // For each planet, check if it is moving and if so, pause it
      // Sun
      if(m_dayPeriodSun->is_playing())
         {
         toggle_planet(P_sun);
         }
      // Mercury
      if(m_dayPeriodMercury->is_playing())
         {
         toggle_planet(P_mercury);
         }
      // Venus
      if(m_dayPeriodVenus->is_playing())
         {
         toggle_planet(P_venus);
         }
      // Earth and moon
      if(m_dayPeriodEarth->is_playing())
         {
         toggle_planet(P_earth);
         toggle_planet(P_moon);
         }
      // Mars
      if(m_dayPeriodMars->is_playing())
         {
         toggle_planet(P_mars);
         }
      }
   else
      {
      // "The simulation is paused, so resume it
      nout << "Resuming Simulation" << endl;
      m_mouse1EventText.set_text(
         "Mouse Button 1: Toggle entire Solar System [RUNNING]");
      // the not operator does the reverse of the previous code
      if(!m_dayPeriodSun->is_playing())
         {
         toggle_planet(P_sun);
         }
      if(!m_dayPeriodMercury->is_playing())
         {
         toggle_planet(P_mercury);
         }
      if(!m_dayPeriodVenus->is_playing())
         {
         toggle_planet(P_venus);
         }
      if(!m_dayPeriodEarth->is_playing())
         {
         toggle_planet(P_earth);
         toggle_planet(P_moon);
         }
      if(!m_dayPeriodMars->is_playing())
         {
         toggle_planet(P_mars);
         }
      }
   // toggle m_simRunning
   m_simRunning = !m_simRunning;
   }

// Note: instead of passing a struct containing *this and the data in parameter
//       dataPtr, we pass only *this and make a family of function based on
//       an enum to identify the planet of interest.
template<int planet>
void World6::call_toggle_planet(const Event* eventPtr, void* dataPtr)
   {
   // Precondition
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<World6*>(dataPtr)->toggle_planet((Planet)planet);
   }

// The togglePlanet function will toggle the intervals that are given to it
// between paused and playing.
// Planet is the name to print
// Day is the interval that spins the planet
// Orbit is the interval that moves around the orbit
// Text is the OnscreenText object that needs to be updated
void World6::toggle_planet(Planet planetId)
   {
   // Note: retrieve information about the specified planet
   string planet;
   PT(CLerpNodePathInterval) day = NULL;
   PT(CLerpNodePathInterval) orbit = NULL;
   COnscreenText* text = NULL;

   switch(planetId)
      {
      case P_sun:
         planet = "Sun";
         day = m_dayPeriodSun;
         orbit = NULL;
         text = &m_skeyEventText;
         break;
      case P_mercury:
         planet = "Mercury";
         day = m_dayPeriodMercury;
         orbit = m_orbitPeriodMercury;
         text = &m_ykeyEventText;
         break;
      case P_venus:
         planet = "Venus";
         day = m_dayPeriodVenus;
         orbit = m_orbitPeriodVenus;
         text = &m_vkeyEventText;
         break;
      case P_earth:
         planet = "Earth";
         day = m_dayPeriodEarth;
         orbit = m_orbitPeriodEarth;
         text = &m_ekeyEventText;
         break;
      case P_moon:
         planet = "Moon";
         day = m_dayPeriodMoon;
         orbit = m_orbitPeriodMoon;
         text = NULL;
         break;
      case P_mars:
         planet = "Mars";
         day = m_dayPeriodMars;
         orbit = m_orbitPeriodMars;
         text = &m_mkeyEventText;
         break;
      default:
         nout << "ERROR: missing a Planet." << endl;
         return;
      }

   // Note: then continue as in the python tutorial
   string state;
   if(day->is_playing())
      {
      nout << "Pausing " << planet << endl;
      state = " [PAUSED]";
      }
   else
      {
      nout << "Resuming " << planet << endl;
      state = " [RUNNING]";
      }

   // Update the onscreen text if it is given as an argument
   if(text != NULL)
      {
      string old = text->get_text();
      // strip out the last segment of text after the last white space
      // and append the string stored in 'state'
      text->set_text(old.substr(0, old.rfind(' ')) + state);
      }

   // toggle the day interval
   toggle_interval(day);
   // if there is an orbit interval, toggle it
   if(orbit != NULL)
      {
      toggle_interval(orbit);
      }
   }

// toggleInterval does exactly as its name implies
// It takes an interval as an argument. Then it checks to see if it is playing.
// If it is, it pauses it, otherwise it resumes it.
void World6::toggle_interval(CLerpNodePathInterval* interval)
   {
   if(interval->is_playing()) { interval->pause (); }
   else                       { interval->resume(); }
   }

void World6::call_handle_earth(const Event* eventPtr, void* dataPtr)
   {
   // Precondition
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<World6*>(dataPtr)->handle_earth();
   }

// Earth needs a special buffer function because the moon is tied to it
// When the "e" key is pressed, togglePlanet is called on both the earth and
// the moon.
void World6::handle_earth()
   {
   toggle_planet(P_earth);
   toggle_planet(P_moon);
   }

void World6::call_inc_year(const Event* eventPtr, void* dataPtr)
   {
   // Precondition
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<World6*>(dataPtr)->inc_year();
   }

// the function incYear increments the variable yearCounter and then updates
// the OnscreenText 'yearCounterText' every time the message "newYear" is sent
void World6::inc_year()
   {
   ++m_yearCounter;
   ostringstream text;
   text << m_yearCounter << " Earth years completed";
   m_yearCounterText.set_text(text.str());
   }

///////////////////////////////////////////////////////////////////////////
// Except for the one commented line below, this is all as it was before //
// Scroll down to the next comment to see an example of sending messages //
///////////////////////////////////////////////////////////////////////////

void World6::load_planets()
   {
   NodePath render = m_windowFramework->get_render();
   m_orbitRootMercury = render.attach_new_node("orbit_root_mercury");
   m_orbitRootVenus = render.attach_new_node("orbit_root_venus");
   m_orbitRootMars = render.attach_new_node("orbit_root_mars");
   m_orbitRootEarth = render.attach_new_node("orbit_root_earth");

   m_orbitRootMoon = m_orbitRootEarth.attach_new_node("orbit_root_moon");

   NodePath models = m_windowFramework->get_panda_framework()->get_models();
   m_sky = m_windowFramework->load_model(models,
                                            "../models/solar_sky_sphere");

   m_skyTex = TexturePool::load_texture("../models/stars_1k_tex.jpg");
   m_sky.set_texture(m_skyTex);
   m_sky.reparent_to(render);
   m_sky.set_scale(40);

   m_sun = m_windowFramework->load_model(models, "../models/planet_sphere");
   m_sunTex = TexturePool::load_texture("../models/sun_1k_tex.jpg");
   m_sun.set_texture(m_sunTex);
   m_sun.reparent_to(render);
   m_sun.set_scale(2 * m_sizescale);

   m_mercury = m_windowFramework->load_model(models,
                                                "../models/planet_sphere");
   m_mercuryTex = TexturePool::load_texture("../models/mercury_1k_tex.jpg");
   m_mercury.set_texture(m_mercuryTex);
   m_mercury.reparent_to(m_orbitRootMercury);
   m_mercury.set_pos( 0.38 * m_orbitscale, 0, 0);
   m_mercury.set_scale(0.385 * m_sizescale);

   m_venus = m_windowFramework->load_model(models,
                                              "../models/planet_sphere");
   m_venusTex = TexturePool::load_texture("../models/venus_1k_tex.jpg");
   m_venus.set_texture(m_venusTex);
   m_venus.reparent_to(m_orbitRootVenus);
   m_venus.set_pos( 0.72 * m_orbitscale, 0, 0);
   m_venus.set_scale(0.923 * m_sizescale);

   m_mars = m_windowFramework->load_model(models, "../models/planet_sphere");
   m_marsTex = TexturePool::load_texture("../models/mars_1k_tex.jpg");
   m_mars.set_texture(m_marsTex);
   m_mars.reparent_to(m_orbitRootMars);
   m_mars.set_pos(1.52 * m_orbitscale, 0, 0);
   m_mars.set_scale(0.515 * m_sizescale);

   m_earth = m_windowFramework->load_model(models,
                                              "../models/planet_sphere");
   m_earthTex = TexturePool::load_texture("../models/earth_1k_tex.jpg");
   m_earth.set_texture(m_earthTex);
   m_earth.reparent_to(m_orbitRootEarth);
   m_earth.set_scale(m_sizescale);
   m_earth.set_pos(m_orbitscale, 0, 0);

   m_orbitRootMoon.set_pos(m_orbitscale, 0, 0);

   m_moon = m_windowFramework->load_model(models, "../models/planet_sphere");
   m_moonTex = TexturePool::load_texture("../models/moon_1k_tex.jpg");
   m_moon.set_texture(m_moonTex);
   m_moon.reparent_to(m_orbitRootMoon);
   m_moon.set_scale(0.1 * m_sizescale);
   m_moon.set_pos(0.1 * m_orbitscale, 0, 0);
   }

void World6::rotate_planets()
   {
   m_dayPeriodSun = new CLerpNodePathInterval("dayPeriodSunInterval",
                                              20,
                                              CLerpInterval::BT_no_blend,
                                              true,
                                              false,
                                              m_sun,
                                              NodePath());
   m_dayPeriodSun->set_start_hpr(LVecBase3f(  0, 0, 0));
   m_dayPeriodSun->set_end_hpr  (LVecBase3f(360, 0, 0));

   m_orbitPeriodMercury =
      new CLerpNodePathInterval("orbitPeriodMercuryInterval",
                                0.241 * m_yearscale,
                                CLerpInterval::BT_no_blend,
                                true,
                                false,
                                m_orbitRootMercury,
                                NodePath());
   m_orbitPeriodMercury->set_start_hpr(LVecBase3f(  0, 0, 0));
   m_orbitPeriodMercury->set_end_hpr  (LVecBase3f(360, 0, 0));

   m_dayPeriodMercury = new CLerpNodePathInterval("dayPeriodMercuryInterval",
                                                  59 * m_dayscale,
                                                  CLerpInterval::BT_no_blend,
                                                  true,
                                                  false,
                                                  m_mercury,
                                                  NodePath());
   m_dayPeriodMercury->set_start_hpr(LVecBase3f(  0, 0, 0));
   m_dayPeriodMercury->set_end_hpr  (LVecBase3f(360, 0, 0));

   m_orbitPeriodVenus = new CLerpNodePathInterval("orbitPeriodVenusInterval",
                                                  0.615 * m_yearscale,
                                                  CLerpInterval::BT_no_blend,
                                                  true,
                                                  false,
                                                  m_orbitRootVenus,
                                                  NodePath());
   m_orbitPeriodVenus->set_start_hpr(LVecBase3f(  0, 0, 0));
   m_orbitPeriodVenus->set_end_hpr  (LVecBase3f(360, 0, 0));

   m_dayPeriodVenus = new CLerpNodePathInterval("dayPeriodVenusInterval",
                                                  243 * m_dayscale,
                                                  CLerpInterval::BT_no_blend,
                                                  true,
                                                  false,
                                                  m_venus,
                                                  NodePath());
   m_dayPeriodVenus->set_start_hpr(LVecBase3f(  0, 0, 0));
   m_dayPeriodVenus->set_end_hpr  (LVecBase3f(360, 0, 0));

   // Here the earth interval has been changed to rotate like the rest of the
   // planets and send a message before it starts turning again. To send a
   // message, the call is simply messenger.send("message"). The "newYear"
   // message is picked up by the accept("newYear"...) statement earlier, and
   // calls the incYear function as a result
   m_orbitPeriodEarth = new CLerpNodePathInterval("orbitPeriodEarthInterval",
                                                  m_yearscale,
                                                  CLerpInterval::BT_no_blend,
                                                  true,
                                                  false,
                                                  m_orbitRootEarth,
                                                  NodePath());
   m_orbitPeriodEarth->set_start_hpr(LVecBase3f(  0, 0, 0));
   m_orbitPeriodEarth->set_end_hpr  (LVecBase3f(360, 0, 0));
   m_orbitPeriodEarth->set_done_event("newYear");

   m_dayPeriodEarth = new CLerpNodePathInterval("dayPeriodEarthInterval",
                                                  m_dayscale,
                                                  CLerpInterval::BT_no_blend,
                                                  true,
                                                  false,
                                                  m_earth,
                                                  NodePath());
   m_dayPeriodEarth->set_start_hpr(LVecBase3f(  0, 0, 0));
   m_dayPeriodEarth->set_end_hpr  (LVecBase3f(360, 0, 0));

   m_orbitPeriodMoon = new CLerpNodePathInterval("orbitPeriodMoonInterval",
                                                  0.0749 * m_yearscale,
                                                  CLerpInterval::BT_no_blend,
                                                  true,
                                                  false,
                                                  m_orbitRootMoon,
                                                  NodePath());
   m_orbitPeriodMoon->set_start_hpr(LVecBase3f(  0, 0, 0));
   m_orbitPeriodMoon->set_end_hpr  (LVecBase3f(360, 0, 0));

   m_dayPeriodMoon = new CLerpNodePathInterval("dayPeriodMoonInterval",
                                                  0.0749 * m_dayscale,
                                                  CLerpInterval::BT_no_blend,
                                                  true,
                                                  false,
                                                  m_moon,
                                                  NodePath());
   m_dayPeriodMoon->set_start_hpr(LVecBase3f(  0, 0, 0));
   m_dayPeriodMoon->set_end_hpr  (LVecBase3f(360, 0, 0));

   m_orbitPeriodMars = new CLerpNodePathInterval("orbitPeriodMarsInterval",
                                                 1.881 * m_yearscale,
                                                 CLerpInterval::BT_no_blend,
                                                 true,
                                                 false,
                                                 m_orbitRootMars,
                                                 NodePath());
   m_orbitPeriodMars->set_start_hpr(LVecBase3f(  0, 0, 0));
   m_orbitPeriodMars->set_end_hpr  (LVecBase3f(360, 0, 0));

   m_dayPeriodMars = new CLerpNodePathInterval("dayPeriodMarsInterval",
                                               1.03 * m_dayscale,
                                               CLerpInterval::BT_no_blend,
                                               true,
                                               false,
                                               m_mars,
                                               NodePath());
   m_dayPeriodMars->set_start_hpr(LVecBase3f(  0, 0, 0));
   m_dayPeriodMars->set_end_hpr  (LVecBase3f(360, 0, 0));

   m_dayPeriodSun->loop();
   m_orbitPeriodMercury->loop();
   m_dayPeriodMercury->loop();
   m_orbitPeriodVenus->loop();
   m_dayPeriodVenus->loop();
   m_orbitPeriodEarth->loop();
   m_dayPeriodEarth->loop();
   m_orbitPeriodMoon->loop();
   m_dayPeriodMoon->loop();
   m_orbitPeriodMars->loop();
   m_dayPeriodMars->loop();

   // Note: setup a task to step the interval manager
   AsyncTaskManager::get_global_ptr()->add(
      new GenericAsyncTask("intervalManagerTask", step_interval_manager, this));
   }

AsyncTask::DoneStatus World6::step_interval_manager(GenericAsyncTask* taskPtr,
                                                   void* dataPtr)
   {
   CIntervalManager::get_global_ptr()->step();
   return AsyncTask::DS_cont;
   }

void World6::sys_exit(const Event* eventPtr, void* dataPtr)
   {
   exit(0);
   }
