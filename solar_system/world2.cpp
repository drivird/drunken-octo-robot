/*
 * world2.cpp
 *
 *  Created on: 2012-07-04
 *      Author: dri
 */

#include "world2.h"

// The initialization method caused when a world object is created
World2::World2(WindowFramework* windowFrameworkPtr)
   : m_windowFrameworkPtr(windowFrameworkPtr),
     m_title("title", COnscreenText::TS_plain)
   {
   // Create some text overlayed on our screen.
   // We will use similar commands in all of our tutorials to create titles and
   // instruction guides.
   m_title.set_text("Panda3D: Tutorial 1 - Solar System");
   m_title.set_fg(Colorf(1, 1, 1, 1));
   m_title.set_pos(LVecBase2f(0.8, -0.95));
   m_title.set_scale(0.07);
   m_title.reparent_to(windowFrameworkPtr->get_aspect_2d());

   // Make the background color black (R=0, G=0, B=0)
   // instead of the default grey
   m_windowFrameworkPtr->set_background_type(WindowFramework::BT_black);

   // By default, the mouse controls the camera. Often, we disable that so that
   // the camera can be placed manually (if we don't do this, our placement
   // commands will be overridden by the mouse control)
   // base.disableMouse() // Note: do not have to do this in C++

   // Set the camera position (x, y, z)
   NodePath camera = windowFrameworkPtr->get_camera_group();
   camera.set_pos(0, 0, 45);

   // Set the camera orientation (heading, pitch, roll) in degrees
   camera.set_hpr(0, -90, 0);
   }
