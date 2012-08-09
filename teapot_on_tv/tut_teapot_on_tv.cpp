/*
 * tut_teapot_on_tv.cpp
 *
 *  Created on: 2012-08-07
 *      Author: dri
  *
 * This is a python to C++ translation of Panda3d 1.7 sample/tutorial:
 * Tut-Teapot-on-TV.py
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

   // and run it!
   pandaFramework.main_loop();

   // quit Panda3d
   pandaFramework.close_framework();
   return 0; // success
   }
