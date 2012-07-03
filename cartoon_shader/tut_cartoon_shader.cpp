/*
 * tut_cartoon_shader.cpp
 *
 *  Created on: 2012-05-31
 *      Author: dri
 *
 * This is a python to C++ translation of Panda3d 1.7 sample/tutorial:
 * Tut-Cartoon-Basic.py
 * Tut-Cartoon-Advanced.py
 *
 * Here's the original python tutorial header:
 *
 * Author: Kwasi Mensah
 * Date: 7/11/2005
 *
 * This is a tutorial to show some of the more advanced things
 * you can do with Cg. Specifically, with Non Photo Realistic
 * effects like Toon Shading. It also shows how to implement
 * multiple buffers in Panda.
 */

#include "pandaFramework.h"

#define RUN_TOON_MAKER_ADVANCED 0
#if RUN_TOON_MAKER_ADVANCED
#include "toonMakerAdvanced.h"
typedef ToonMakerAdvanced ToonMaker;
#else
#include "toonMakerBasic.h"
typedef ToonMakerBasic ToonMaker;
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
   ToonMaker toonMaker(windowFrameworkPtr);

   // Run the simulation
   pandaFramework.main_loop();

   // quit Panda3d
   pandaFramework.close_framework();
   return 0; // success
   }
