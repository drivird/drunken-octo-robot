/*
 * world3.cpp
 *
 *  Created on: 2012-07-04
 *      Author: dri
 */

#include "texturePool.h"
#include "world3.h"

// The initialization method caused when a world object is created
World3::World3(WindowFramework* windowFrameworkPtr)
   : m_windowFrameworkPtr(windowFrameworkPtr),
     m_title("title", COnscreenText::TS_plain),
     m_sizescale(1),
     m_sky(),
     m_skyTex(NULL),
     m_sun(),
     m_sunTex(NULL)
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

   // We will now define a variable to help keep a consistent scale in
   // our model. As we progress, we will continue to add variables here as we
   // need them

   // The value of this variable scales the size of the planets. True scale size
   // would be 1
   m_sizescale = 0.6;;

   // Now that we have finished basic initialization, we call loadPlanets which
   // will handle actually getting our objects in the world
   load_planets();
   }

void World3::load_planets()
   {
   // Here, inside our class, is where we are creating the loadPlanets function
   // For now we are just loading the star-field and sun. In the next step we
   // will load all of the planets

   // Loading objects in Panda is done via the command loader.loadModel, which
   // takes one argument, the path to the model file. Models in Panda come in
   // two types, .egg (which is readable in a text editor), and .bam (which is
   // not readable but makes smaller files). When you load a file you leave the
   // extension off so that it can choose the right version

   // Load model returns a NodePath, which you can think of as an object
   // containing your model

   // Here we load the sky model. For all the planets we will use the same
   // sphere model and simply change textures. However, even though the sky is
   // a sphere, it is different from the planet model because its polygons
   // (which are always one-sided in Panda) face inside the sphere instead of
   // outside (this is known as a model with reversed normals). Because of
   // that it has to be a separate model.
   NodePath models = m_windowFrameworkPtr->get_panda_framework()->get_models();
   m_sky = m_windowFrameworkPtr->load_model(models,
                                            "../models/solar_sky_sphere");

   // After the object is loaded, it must be placed in the scene. We do this by
   // changing the parent of self.sky to render, which is a special NodePath.
   // Each frame, Panda starts with render and renders everything attached to
   // it.
   NodePath render = m_windowFrameworkPtr->get_render();
   m_sky.reparent_to(render);

   // You can set the position, orientation, and scale on a NodePath the same
   // way that you set those properties on the camera. In fact, the camera is
   // just another special NodePath
   m_sky.set_scale(40);

   // Very often, the egg file will know what textures are needed and load them
   // automatically. But sometimes we want to set our textures manually, (for
   // instance we want to put different textures on the same planet model)
   // Loading textures works the same way as loading models, but instead of
   // calling loader.loadModel, we call loader.loadTexture
   m_skyTex = TexturePool::load_texture("../models/stars_1k_tex.jpg");

   // Finally, the following line sets our new sky texture on our sky model.
   // The second argument must be one or the command will be ignored.
   m_sky.set_texture(m_skyTex);

   // Now we load the sun.
   m_sun = m_windowFrameworkPtr->load_model(models, "../models/planet_sphere");
   // Now we repeat our other steps
   m_sun.reparent_to(render);
   m_sunTex = TexturePool::load_texture("../models/sun_1k_tex.jpg");
   m_sun.set_texture(m_sunTex);
   m_sun.set_scale(2 * m_sizescale); // The sun is really much bigger than
                                     // this, but to be able to see the
                                     // planets we're making it smaller
   }
