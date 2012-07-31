/*
 * tut_infinite_tunnel.cpp
 *
 *  Created on: 2012-07-26
 *      Author: dri
 *
 * This is a python to C++ translation of Panda3d 1.7 sample/tutorial:
 * Tut-Infinite-Tunnel.py
 *
 * Here is the original header of the python tutorial:
 * Author: Shao Zhang and Phil Saltzman
 * Last Updated: 4/18/2005
 *
 * This tutorial will cover fog and how it can be used to make a finite length
 * tunnel seem endless by hiding its endpoint in darkness. Fog in panda works by
 * coloring objects based on their distance from the camera. Fog is not a 3D
 * volume object like real world fog.
 * With the right settings, Fog in panda can mimic the appearence of real world
 * fog.
 *
 * The tunnel and texture was originally created by Vamsi Bandaru and Victoria
 * Webb for the Entertainment Technology Center class Building Virtual Worlds
 *
 */

#include "pandaFramework.h"
#include "../p3util/sleepWhenMinimized.h"
#include "world.h"

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

   // Make an instance of our class
   World world(windowFramework);

   // and run the world
   pandaFramework.main_loop();

   // quit Panda3d
   pandaFramework.close_framework();
   return 0; // success
   }
