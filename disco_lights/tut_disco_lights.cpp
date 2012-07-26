/*
 * tut_disco_lights.cpp
 *
 *  Created on: 2012-07-17
 *      Author: dri
 *
 * This is a python to C++ translation of Panda3d 1.7 sample/tutorial:
 * Tut-Disco-Lights.py
 *
 * Here is the original header of the python tutorial:
 * Author: Jason Pratt (pratt@andrew.cmu.edu)
 * Last Updated: 10/20/2003
 *
 * This project demonstrates how to use various types of
 * lighting
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

   // Make an instance of our class
   World world(windowFramework);

   // Note: optional, reduce CPU use when the window is minimized.
   SleepWhenMinimized sleepWhenMinimized(windowFramework);

   // and run the world
   pandaFramework.main_loop();

   // quit Panda3d
   pandaFramework.close_framework();
   return 0; // success
   }
