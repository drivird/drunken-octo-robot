/*
 * tut_asteroids.cpp
 *
 *  Created on: 2012-05-17
 *      Author: dri
 *
 * This is a python to C++ translation of Panda3d 1.7 sample/tutorial:
 * Tut-Asteroids.py
 *
 * Here's the original python tutorial header:
 *
 * Author: Shao Zhang, Phil Saltzman, and Greg Lindley
 * Last Updated: 5/1/2005
 *
 * This tutorial demonstrates the use of tasks. A task is a function that gets
 * called once every frame. They are good for things that need to be updated
 * very often. In the case of asteroids, we use tasks to update the positions of
 * all the objects, and to check if the bullets or the ship have hit the
 * asteroids.
 *
 * Note: This definitely a complicated example. Tasks are the cores most games
 * so it seemed appropriate to show what a full game in Panda could look like
 *
 */

#include "pandaFramework.h"
#include "world.h"

int main(int argc, char *argv[])
   {
   // setup Panda3d
   PandaFramework pandaFramework;
   pandaFramework.open_framework(argc, argv);
   PT(WindowFramework) windowFrameworkPtr = pandaFramework.open_window();
   if(windowFrameworkPtr == NULL)
      {
      nout << "ERROR: could not open the WindowFramework." << endl;
      return 1; // error
      }

   // We now have everything we need. Make an instance of the class and start
   // 3D rendering
   World world(windowFrameworkPtr);

   // Run the simulation
   pandaFramework.main_loop();

   // quit Panda3d
   pandaFramework.close_framework();
   return 0; // success
   }
