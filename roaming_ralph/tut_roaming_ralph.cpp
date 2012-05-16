/*
 * tut_roaming_ralph.cpp
 *
 *  Created on: 2012-05-14
 *      Author: dri
 *
 * This is a python to C++ translation of Panda3d 1.7 sample/tutorial:
 * Tut-Roaming-Ralph.py
 *
 * Here's the original python tutorial header:
 *
 * Author: Ryan Myers
 * Models: Jeff Styers, Reagan Heller
 *
 *
 * Last Updated: 6/13/2005
 *
 * This tutorial provides an example of creating a character
 * and having it walk around on uneven terrain, as well
 * as implementing a fully rotatable camera.
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
