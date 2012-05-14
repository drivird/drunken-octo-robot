/*
 * tut_bump_mapping.cpp
 *
 *  Created on: 2012-05-12
 *      Author: dri
 *
 * This is a python to C++ translation of Panda3d 1.7 sample/tutorial:
 * Tut-Bump-Mapping.py
 *
 * Here's the original python tutorial header:
 *
 * Bump mapping is a way of making polygonal surfaces look
 * less flat.  This sample uses normal mapping for all
 * surfaces, and also parallax mapping for the column.
 *
 * This is a tutorial to show how to do normal mapping
 * in panda3d using the Shader Generator.
 *
 */

#include "pandaFramework.h"
#include "windowFramework.h"
#include "bumpMapDemo.h"
#include "load_prc_file.h"

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

   // Configure the parallax mapping settings (these are just the defaults)
   load_prc_file_data("", "parallax-mapping-samples 3");
   load_prc_file_data("", "parallax-mapping-scale 0.1");

   // Create an instance of our class
   BumpMapDemo bumpMapDemo(windowFrameworkPtr);

   // Run the simulation
   pandaFramework.main_loop();

   // quit Panda3d
   pandaFramework.close_framework();
   return 0; // success
   }
