/*
 * tut_texture_swapping.cpp
 *
 *  Created on: 2012-08-10
 *      Author: dri
 *
 * This is a python to C++ translation of Panda3d 1.7 samples/tutorials:
 * Tut-Texture-Swapping.py
 * Tut-Texture-Swapping-II.py
 *
 */

#include "pandaFramework.h"
#include "../p3util/sleepWhenMinimized.h"

#define RUN_TUTORIAL_II 0
#if RUN_TUTORIAL_II
#include "world2.h"
#define World World2
#else
#include "world.h"
#endif

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
