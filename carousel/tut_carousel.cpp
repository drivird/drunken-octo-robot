/*
 * tut_carousel.cpp
 *
 *  Created on: 2012-05-29
 *      Author: dri
 *
 * This is a python to C++ translation of Panda3d 1.7 sample/tutorial:
 * Tut-Carousel.py
 *
 * Here's the original python tutorial header:
 *
 * Author: Shao Zhang, Phil Saltzman, and Eddie Canaan
 * Last Updated: 4/19/2005
 *
 * This tutorial will demonstrate some uses for intervals in Panda
 * to move objects in your panda world.
 * Intervals are tools that change a value of something, like position, rotation
 * or anything else, linearly, over a set period of time. They can be also be
 * combined to work in sequence or in Parallel
 *
 * In this lesson, we will simulate a carousel in motion using intervals.
 * The carousel will spin using an hprInterval while 4 pandas will represent
 * the horses on a traditional carousel. The 4 pandas will rotate with the
 * carousel and also move up and down on their poles using a LerpFunc interval.
 * Finally there will also be lights on the outer edge of the carousel that
 * will turn on and off by switching their texture with intervals in Sequence
 * and Parallel
 *
 */

#include "pandaFramework.h"
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
