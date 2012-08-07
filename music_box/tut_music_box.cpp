/*
 * tut_music_box.cpp
 *
 *  Created on: 2012-07-31
 *      Author: dri
 *
 * This is a python to C++ translation of Panda3d 1.7 sample/tutorial:
 * Tut-Music-Box.py
 *
 * Here is the original header of the python tutorial:
 * Author: Shao Zhang, Phil Saltzman, and Elan Ruskin
 * Last Updated: 4/20/2005
 *
 * This tutorial shows how to load, play, and manipulate sounds
 * and sound intervals in a panda project.
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

   // and we can run!
   pandaFramework.main_loop();

   // quit Panda3d
   pandaFramework.close_framework();
   return 0; // success
   }
