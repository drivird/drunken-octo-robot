/*
 * tut_looking_and_gripping.cpp
 *
 *  Created on: 2012-05-08
 *      Author: dri
 *
 * This is a python to C++ translation of Panda3d 1.7 sample/tutorial:
 * Tut-Looking-and-Gripping.py
 *
 * Here's the original python tutorial header:
 *
 * Author: Shao Zhang and Phil Saltzman
 * Models and Textures by: Shaun Budhram, Will Houng, and David Tucker
 * Last Updated: 5/1/2005
 *
 * This tutorial will cover exposing joints and manipulating them. Specifically,
 * we will take control of the neck joint of a humanoid character and rotate that
 * joint to always face the mouse cursor.  This will in turn make the head of the
 * character "look" at the mouse cursor.  We will also expose the hand joint and
 * use it as a point to "attach" objects that the character can hold.  By
 * parenting an object to a hand joint, the object will stay in the character's
 * hand even if the hand is moving through an animation.
 */

#include "pandaFramework.h"
#include "windowFramework.h"
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
