/*
 * tut_chessboard.cpp
 *
 *  Created on: 2012-07-08
 *      Author: dri
 *
 * This is a python to C++ translation of Panda3d 1.7 samples/tutorials:
 * Tut-Chessboard.py
 *
 * Original python file header:
 *
 * Author: Shao Zhang and Phil Saltzman
 * Models: Eddie Canaan
 * Last Updated: 5/1/2005
 *
 * This tutorial shows how to determine what objects the mouse is pointing to
 * We do this using a collision ray that extends from the mouse position
 * and points straight into the scene, and see what it collides with. We pick
 * the object with the closest collision
 */

#include "pandaFramework.h"
#include "world.h"

int main(int argc, char *argv[])
   {
   // Initialize Panda and create a window
   PandaFramework pandaFramework;
   pandaFramework.open_framework(argc, argv);
   PT(WindowFramework) windowFrameworkPtr = pandaFramework.open_window();
   if(windowFrameworkPtr == NULL)
      {
      nout << "ERROR: could not open the WindowFramework." << endl;
      return 1; // error
      }

   // Do the main initialization and start 3D rendering
   World world(windowFrameworkPtr);

   pandaFramework.main_loop();

   // quit Panda3d
   pandaFramework.close_framework();
   return 0; // success
   }
