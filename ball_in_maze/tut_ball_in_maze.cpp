/*
 * tut_ball_in_maze.cpp
 *
 *  Created on: 2012-05-19
 *      Author: dri
 *
 * This is a python to C++ translation of Panda3d 1.7 sample/tutorial:
 * Tut-Ball-in-Maze.py
 *
 * Here's the original python tutorial header:
 *
 * Author: Shao Zhang, Phil Saltzman
 * Last Updated: 5/2/2005
 *
 * This tutorial shows how to detect and respond to collisions. It uses solids
 * create in code and the egg files, how to set up collision masks, a traverser,
 * and a handler, how to detect collisions, and how to dispatch function based
 * on the collisions. All of this is put together to simulate a labyrinth-style
 * game
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

   // Finally, create an instance of our class and start 3d rendering
   World world(windowFrameworkPtr);

   // Run the simulation
   pandaFramework.main_loop();

   // quit Panda3d
   pandaFramework.close_framework();
   return 0; // success
   }
