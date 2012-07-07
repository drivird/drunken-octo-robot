/*
 * world4.cpp
 *
 *  Created on: 2012-07-04
 *      Author: dri
 */

#include "texturePool.h"
#include "world4.h"

// The initialization method caused when a world object is created
World4::World4(WindowFramework* windowFrameworkPtr)
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
     m_moonTex(NULL)
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

   // This section has our variables. This time we are adding a variable to
   // control the relative size of the orbits.
   m_sizescale = 0.6;                  // relative size of planets
   m_orbitscale = 10;                  // relative size of orbits

   load_planets();                     // Load our models and make them render
   }

void World4::load_planets()
   {
   // Here is where we load all of the planets, and place them.
   // The first thing we do is create a dummy node for each planet. A dummy
   // node is simply a node path that does not have any geometry attached to it.
   // This is done by <NodePath>.attachNewNode('name_of_new_node')

   // We do this because positioning the planets around a circular orbit could
   // be done with a lot of messy sine and cosine operations. Instead, we define
   // our planets to be a given distance from a dummy node, and when we turn the
   // dummy, the planets will move along with it, kind of like turning the
   // center of a disc and having an object at its edge move. Most attributes,
   // like position, orientation, scale, texture, color, etc., are inherited
   // this way. Panda deals with the fact that the objects are not attached
   // directly to render (they are attached through other NodePaths to render),
   // and makes sure the attributes inherit.

   // This system of attaching NodePaths to each other is called the Scene Graph
   NodePath render = m_windowFrameworkPtr->get_render();
   m_orbitRootMercury = render.attach_new_node("orbit_root_mercury");
   m_orbitRootVenus = render.attach_new_node("orbit_root_venus");
   m_orbitRootMars = render.attach_new_node("orbit_root_mars");
   m_orbitRootEarth = render.attach_new_node("orbit_root_earth");

   // orbit_root_moon is like all the other orbit_root dummy nodes except that
   // it will be parented to orbit_root_earth so that the moon will orbit the
   // earth instead of the sun. So, the moon will first inherit
   // orbit_root_moon's position and then orbit_root_earth's. There is no hard
   // limit on how many objects can inherit from each other.
   m_orbitRootMoon = m_orbitRootEarth.attach_new_node("orbit_root_moon");

   //################################################################

   // These are the same steps used to load the sky model that we used in the
   // last step
   // Load the model for the sky
   NodePath models = m_windowFrameworkPtr->get_panda_framework()->get_models();
   m_sky = m_windowFrameworkPtr->load_model(models,
                                            "../models/solar_sky_sphere");
   // Load the texture for the sky.
   m_skyTex = TexturePool::load_texture("../models/stars_1k_tex.jpg");
   // Set the sky texture to the sky model
   m_sky.set_texture(m_skyTex);
   // Parent the sky model to the render node so that the sky is rendered
   m_sky.reparent_to(render);
   // Scale the size of the sky.
   m_sky.set_scale(40);

   // These are the same steps we used to load the sun in the last step.
   // Again, we use loader.loadModel since we're using planet_sphere more
   // than once.
   m_sun = m_windowFrameworkPtr->load_model(models, "../models/planet_sphere");
   m_sunTex = TexturePool::load_texture("../models/sun_1k_tex.jpg");
   m_sun.set_texture(m_sunTex);
   m_sun.reparent_to(render);
   m_sun.set_scale(2 * m_sizescale);

   // Now we load the planets, which we load using the same steps we used to
   // load the sun. The only difference is that the models are not parented
   // directly to render for the reasons described above.
   // The values used for scale are the ratio of the planet's radius to Earth's
   // radius, multiplied by our global scale variable. In the same way, the
   // values used for orbit are the ratio of the planet's orbit to Earth's
   // orbit, multiplied by our global orbit scale variable

   // Load mercury
   m_mercury = m_windowFrameworkPtr->load_model(models,
                                                "../models/planet_sphere");
   m_mercuryTex = TexturePool::load_texture("../models/mercury_1k_tex.jpg");
   m_mercury.set_texture(m_mercuryTex);
   m_mercury.reparent_to(m_orbitRootMercury);
   // Set the position of mercury. By default, all nodes are pre assigned the
   // position (0, 0, 0) when they are first loaded. We didn't reposition the
   // sun and sky because they are centered in the solar system. Mercury,
   // however, needs to be offset so we use .setPos to offset the
   // position of mercury in the X direction with respect to its orbit radius.
   // We will do this for the rest of the planets.
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

   // The center of the moon's orbit is exactly the same distance away from
   // The sun as the Earth's distance from the sun
   m_orbitRootMoon.set_pos(m_orbitscale, 0, 0);

   // Load the moon
   m_moon = m_windowFrameworkPtr->load_model(models, "../models/planet_sphere");
   m_moonTex = TexturePool::load_texture("../models/moon_1k_tex.jpg");
   m_moon.set_texture(m_moonTex);
   m_moon.reparent_to(m_orbitRootMoon);
   m_moon.set_scale(0.1 * m_sizescale);
   m_moon.set_pos(0.1 * m_orbitscale, 0, 0);
   }
