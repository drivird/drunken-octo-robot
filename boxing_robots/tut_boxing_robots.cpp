/*
 * tut_boxing_robots.cpp
 *
 *  Created on: 2012-05-22
 *      Author: dri
 *
 * This is a python to C++ translation of Panda3d 1.7 sample/tutorial:
 * Tut-Boxing-Robots.py
 *
 * Here's the original python tutorial header:
 *
 * This tutorial shows how to play animations on models aka "actors".
 * It is based on the popular game of "Rock 'em Sock 'em Robots".
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

   // Create an instance of our class
   World world(windowFrameworkPtr);

   // Run the simulation
   pandaFramework.main_loop();

   // quit Panda3d
   pandaFramework.close_framework();
   return 0; // success
   }
