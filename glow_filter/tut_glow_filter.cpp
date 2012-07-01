/*
 * tut_glow_filter.cpp
 *
 *  Created on: 2012-06-29
 *      Author: dri
 *
 * This is a python to C++ translation of Panda3d 1.7 sample/tutorials:
 * Tut-Glow-Basic.py
 * Tut-Glow-Advanced.py
 *
 * Here's the original python tutorial header:
 *
 * Author: Kwasi Mensah (kmensah@andrew.cmu.edu)
 * Date: 7/25/2005
 *
 */

#include "pandaFramework.h"

#define RUN_GLOW_DEMO_ADVANCED 0
#if RUN_GLOW_DEMO_ADVANCED
#include "glowDemoAdvanced.h"
typedef GlowDemoAdvanced GlowDemo;
#else
#include "glowDemoBasic.h"
typedef GlowDemoBasic GlowDemo;
#endif

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
   GlowDemo glowDemo(windowFrameworkPtr);

   // Run the simulation
   pandaFramework.main_loop();

   // quit Panda3d
   pandaFramework.close_framework();
   return 0; // success
   }
