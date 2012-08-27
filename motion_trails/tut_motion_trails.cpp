/*
 * tut_motion_trails.cpp
 *
 *  Created on: 2012-08-18
 *      Author: dri
 *
 * This is a python to C++ translation of Panda3d 1.7 samples/tutorial:
 * Tut-Motion-Trails.py
 *
 * Original tutorial header:
 * Author: Josh Yelon
 *
 * This is a tutorial to show one of the simplest applications
 * of copy-to-texture: motion trails.
 */

#include "pandaFramework.h"
#include "../p3util/sleepWhenMinimized.h"
#include "../p3util/cActor.h"
#include "directionalLight.h"
#include "ambientLight.h"
#include "motionTrails.h"

int main(int argc, char *argv[])
   {
   // setup Panda3d
   PandaFramework pandaFramework;
   pandaFramework.open_framework(argc, argv);
   PT(WindowFramework) windowFramework = pandaFramework.open_window();
   if(windowFramework == NULL)
      {
      nout << "ERROR: could not open the WindowFramework." << endl;
      return 1; // error
      }

   // Note: optional, reduce CPU use when the window is minimized.
   SleepWhenMinimized sleepWhenMinimized(windowFramework);

   // base.disableMouse() // no need in C++
   NodePath camera = windowFramework->get_camera_group();
   camera.set_pos(0, -26, 4);
   // Note: disable background on the 3D display region (sort==0) or else the
   //       MotionTrails background (sort==-10) will be cleared by it.
   windowFramework->set_background_type(WindowFramework::BT_none);

   CActor character;
   CActor::AnimMap animMap;
   animMap["../models/dancer"].push_back("win");
   character.load_actor(windowFramework, "../models/dancer", &animMap, 0);
   NodePath render = windowFramework->get_render();
   character.reparent_to(render);
   const bool restart = false;
   character.loop("win", restart);
   // character.hprInterval(15, Point3(360, 0,0)).loop()

   // put some lighting on the model
   PT(DirectionalLight) dlight = new DirectionalLight("dlight");
   PT(AmbientLight) alight = new AmbientLight("alight");
   NodePath dlnp = render.attach_new_node(dlight);
   NodePath alnp = render.attach_new_node(alight);
   dlight->set_color(Colorf(1.0, 0.9, 0.8, 1));
   alight->set_color(Colorf(0.2, 0.3, 0.4, 1));
   dlnp.set_hpr(0, -60, 0);
   render.set_light(dlnp);
   render.set_light(alnp);

   // Make an instance of our class
   MotionTrails motionTrails(windowFramework);

   // and run it!
   pandaFramework.main_loop();

   // quit Panda3d
   pandaFramework.close_framework();
   return 0; // success
   }
