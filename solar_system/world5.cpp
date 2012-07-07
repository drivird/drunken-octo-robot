/*
 * world5.cpp
 *
 *  Created on: 2012-07-04
 *      Author: dri
 */

#include "texturePool.h"
#include "cIntervalManager.h"
#include "world5.h"

// The initialization method caused when a world object is created
World5::World5(WindowFramework* windowFrameworkPtr)
   : m_windowFrameworkPtr(windowFrameworkPtr),
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
     m_dayPeriodMars(NULL)
   {
   // This is the initialization we had before
   m_title.set_text("Panda3D: Tutorial 1 - Solar System");
   m_title.set_fg(Colorf(1, 1, 1, 1));
   m_title.set_pos(LVecBase2f(0.8, -0.95));
   m_title.set_scale(0.07);
   m_title.reparent_to(m_windowFrameworkPtr->get_aspect_2d());

   // Set the background to black
   m_windowFrameworkPtr->get_graphics_window()->get_active_display_region(0)->
      set_clear_color(Colorf(0, 0, 0, 0));
   // Note: mouse ain't enable by default in C++
   // base.disableMouse()              // disable mouse control of the camera
   NodePath camera = m_windowFrameworkPtr->get_camera_group();
   camera.set_pos(0, 0, 45);           // Set the camera position (X, Y, Z)
   camera.set_hpr(0, -90, 0);          // Set the camera orientation
                                       // (heading, pitch, roll) in degrees

   // Here again is where we put our global variables. Added this time are
   // variables to control the relative speeds of spinning and orbits in the
   // simulation
   // Number of seconds a full rotation of Earth around the sun should take
   m_yearscale = 60;
   // Number of seconds a day rotation of Earth should take.
   // It is scaled from its correct value for easier visability
   m_dayscale = m_yearscale / 365.0 * 5;
   m_sizescale = 0.6;                  // relative size of planets
   m_orbitscale = 10;                  // relative size of orbits

   load_planets();                     // Load our models and make them render

   // Finally, we call the rotatePlanets function which puts the planets,
   // sun, and moon into motion.
   rotate_planets();
   }

void World5::load_planets()
   {
   // This is the same function that we completed in the previous step
   // It is unchanged in this version

   // Create the dummy nodes
   NodePath render = m_windowFrameworkPtr->get_render();
   m_orbitRootMercury = render.attach_new_node("orbit_root_mercury");
   m_orbitRootVenus = render.attach_new_node("orbit_root_venus");
   m_orbitRootMars = render.attach_new_node("orbit_root_mars");
   m_orbitRootEarth = render.attach_new_node("orbit_root_earth");

   // The moon orbits Earth, not the sun
   m_orbitRootMoon = m_orbitRootEarth.attach_new_node("orbit_root_moon");

   //################################################################

   // Load the sky
   NodePath models = m_windowFrameworkPtr->get_panda_framework()->get_models();
   m_sky = m_windowFrameworkPtr->load_model(models,
                                            "../models/solar_sky_sphere");
   m_skyTex = TexturePool::load_texture("../models/stars_1k_tex.jpg");
   m_sky.set_texture(m_skyTex);
   m_sky.reparent_to(render);
   m_sky.set_scale(40);

   // Load the Sun
   m_sun = m_windowFrameworkPtr->load_model(models, "../models/planet_sphere");
   m_sunTex = TexturePool::load_texture("../models/sun_1k_tex.jpg");
   m_sun.set_texture(m_sunTex);
   m_sun.reparent_to(render);
   m_sun.set_scale(2 * m_sizescale);

   // Load mercury
   m_mercury = m_windowFrameworkPtr->load_model(models,
                                                "../models/planet_sphere");
   m_mercuryTex = TexturePool::load_texture("../models/mercury_1k_tex.jpg");
   m_mercury.set_texture(m_mercuryTex);
   m_mercury.reparent_to(m_orbitRootMercury);
   m_mercury.set_pos( 0.38 * m_orbitscale, 0, 0);
   m_mercury.set_scale(0.385 * m_sizescale);

   // Load Venus
   m_venus = m_windowFrameworkPtr->load_model(models,
                                              "../models/planet_sphere");
   m_venusTex = TexturePool::load_texture("../models/venus_1k_tex.jpg");
   m_venus.set_texture(m_venusTex);
   m_venus.reparent_to(m_orbitRootVenus);
   m_venus.set_pos( 0.72 * m_orbitscale, 0, 0);
   m_venus.set_scale(0.923 * m_sizescale);

   // Load Mars
   m_mars = m_windowFrameworkPtr->load_model(models, "../models/planet_sphere");
   m_marsTex = TexturePool::load_texture("../models/mars_1k_tex.jpg");
   m_mars.set_texture(m_marsTex);
   m_mars.reparent_to(m_orbitRootMars);
   m_mars.set_pos(1.52 * m_orbitscale, 0, 0);
   m_mars.set_scale(0.515 * m_sizescale);

   // Load Earth
   m_earth = m_windowFrameworkPtr->load_model(models,
                                              "../models/planet_sphere");
   m_earthTex = TexturePool::load_texture("../models/earth_1k_tex.jpg");
   m_earth.set_texture(m_earthTex);
   m_earth.reparent_to(m_orbitRootEarth);
   m_earth.set_scale(m_sizescale);
   m_earth.set_pos(m_orbitscale, 0, 0);

   // Offest the moon dummy node so that it is positioned properly
   m_orbitRootMoon.set_pos(m_orbitscale, 0, 0);

   // Load the moon
   m_moon = m_windowFrameworkPtr->load_model(models, "../models/planet_sphere");
   m_moonTex = TexturePool::load_texture("../models/moon_1k_tex.jpg");
   m_moon.set_texture(m_moonTex);
   m_moon.reparent_to(m_orbitRootMoon);
   m_moon.set_scale(0.1 * m_sizescale);
   m_moon.set_pos(0.1 * m_orbitscale, 0, 0);
   }

void World5::rotate_planets()
   {
   // rotatePlanets creates intervals to actually use the hierarchy we created
   // to turn the sun, planets, and moon to give a rough representation of the
   // solar system. The next lesson will go into more depth on intervals.
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

   m_orbitPeriodEarth = new CLerpNodePathInterval("orbitPeriodEarthInterval",
                                                  m_yearscale,
                                                  CLerpInterval::BT_no_blend,
                                                  true,
                                                  false,
                                                  m_orbitRootEarth,
                                                  NodePath());
   m_orbitPeriodEarth->set_start_hpr(LVecBase3f(  0, 0, 0));
   m_orbitPeriodEarth->set_end_hpr  (LVecBase3f(360, 0, 0));

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
   m_dayPeriodMercury->output(nout);
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

AsyncTask::DoneStatus World5::step_interval_manager(GenericAsyncTask* taskPtr,
                                                   void* dataPtr)
   {
   CIntervalManager::get_global_ptr()->step();
   return AsyncTask::DS_cont;
   }
