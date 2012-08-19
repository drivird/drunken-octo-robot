/*
 * tut_distortion.cpp
 *
 *  Created on: 2012-08-17
 *      Author: dri
 *
 * This is a python to C++ translation of Panda3d 1.7 samples/tutorials:
 * Tut-Distortion.py
 *
 * Original tutorial header:
 * Author: Tree Form starplant@gmail.com
 */

#include "pandaFramework.h"
#include "../p3util/sleepWhenMinimized.h"
#include "distortionDemo.h"

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

   NodePath title;
   if(windowFramework->get_graphics_output()->get_gsg()->
      get_supports_basic_shaders())
      {
      // Note: optional, reduce CPU use when the window is minimized.
      SleepWhenMinimized sleepWhenMinimized(windowFramework);

      // Make an instance of our class
      DistortionDemo distortionDemo(windowFramework);

      // and run it!
      pandaFramework.main_loop();
      }
   else
      {
      title = add_title(windowFramework,
         "distortion Demo: Video driver says shaders not supported.");
      }

   // quit Panda3d
   pandaFramework.close_framework();
   return 0; // success
   }
