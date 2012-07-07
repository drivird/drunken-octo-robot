/*
 * tut_solar_system.cpp
 *
 *  Created on: 2012-07-04
 *      Author: dri
 *
 * This is a python to C++ translation of Panda3d 1.7 samples/tutorials:
 * Tut-Step-1-Blank-Window.py
 * Tut-Step-2-Basic-Setup.py 
 * Tut-Step-3-Load-Model.py 
 * Tut-Step-4-Load-System.py 
 * Tut-Step-5-Complete-Solar-System.py 
 * Tut-Step-6-Controllable-System.py
 * 
 * Original python file header:
 *
 * Author: Shao Zhang and Phil Saltzman
 * Last Updated: 3/20/2005
 *
 * This tutorial is intended as a initial panda scripting lesson going over
 * display initialization, loading models, placing objects, and the scene graph.
 *
 */

#include "pandaFramework.h"

// Change this #define to compile the different steps of this tutorial.
#define STEP 6

#if STEP == 1
// Step 1: DirectStart contains the main Panda3D modules. Importing it
// initializes Panda and creates the window. The run() command causes the
// real-time simulation to begin
// Note: Step 1 is to write the main(), i.e. this file.

#elif STEP == 2
// Step 2: After initializing panda, we define a class called World. We put
// all of our code in a class to provide a convenient way to keep track of
// all of the variables our project will use, and in later tutorials to handle
// keyboard input.
// The code contained in the __init__ method is executed when we instantiate
// the class (at the end of this file).  Inside __init__ we will first change
// the background color of the window.  We then disable the mouse-based camera
// control and set the camera position.
#include "world2.h"
typedef World2 World;

#elif STEP == 3
//  Step 3: In this step, we create a function called loadPlanets, which will
//  eventually be used to load all of the planets in our simulation. For now
//  we will load just the sun and and the sky-sphere we use to create the
//  star-field.
#include "world3.h"
typedef World3 World;

#elif STEP == 4
// Step 4: In this step, we will load the rest of the planets up to Mars.
// In addition to loading them, we will organize how the planets are grouped
// hierarchically in the scene. This will help us rotate them in the next step
// to give a rough simulation of the solar system.  You can see them move by
// running step_5_complete_solar_system.py.
#include "world4.h"
typedef World4 World;

#elif STEP == 5
// Step 5: Here we put the finishing touches on our solar system model by
// making the planets move. The actual code for doing the movement is covered
// in the next tutorial, but watching it move really shows what inheritance on
// the scene graph is all about.
#include "world5.h"
typedef World5 World;

#elif STEP == 6
// This tutorial will cover events and how they can be used in Panda
// Specifically, this lesson will use events to capture keyboard presses and
// mouse clicks to trigger actions in the world. It will also use events
// to count the number of orbits the Earth makes around the sun. This
// tutorial uses the same base code from the solar system tutorial.
#include "world6.h"
typedef World6 World;

#else
#error Invalid STEP (only STEPs 1 trough 6 are valid)

#endif

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

   // Now that our class is defined, we create an instance of it.
   // Doing so calls the __init__ method set up above
   // Note: except for step 1 which has no class defined.
#if STEP > 1
   World world(windowFrameworkPtr);
#endif

   // As usual - run() must be called before anything can be shown on screen
   pandaFramework.main_loop();

   // quit Panda3d
   pandaFramework.close_framework();
   return 0; // success
   }
