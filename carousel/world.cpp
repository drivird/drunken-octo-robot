/*
 * world.cpp
 *
 *  Created on: 2012-05-29
 *      Author: dri
 */

#include "../p3util/cOnscreenText.h"
#include "../p3util/genericFunctionInterval.h"
#include "texturePool.h"
#include "ambientLight.h"
#include "directionalLight.h"
#include "genericAsyncTask.h"
#include "waitInterval.h"
#include "cIntervalManager.h"
#include "world.h"

World::World(WindowFramework* windowFrameworkPtr)
   : m_windowFrameworkPtr(windowFrameworkPtr),
     m_pandasNp(P_pandas),
     m_modelsNp(P_pandas)
   {
   // preconditions
   if(m_windowFrameworkPtr == NULL)
      {
      nout << "ERROR: parameter windowFrameworkPtr cannot be NULL." << endl;
      return;
      }

   // This creates the on screen title that is in every tutorial
   m_titleNp = COnscreenText(m_windowFrameworkPtr,
                             "Panda3D: Tutorial 2 - Carousel",
                             Colorf(1,1,1,1),
                             LPoint2f(0.87,-0.95),
                             COnscreenText::A_center,
                             0.07);

   // Set the background color
   m_windowFrameworkPtr->get_graphics_window()->get_active_display_region(0)->set_clear_color(Colorf(0.6, 0.6, 1, 1));
   // Allow manual positioning of the camera
   // Note: in that state by default in C++
   NodePath cameraNp = m_windowFrameworkPtr->get_camera_group();
   // Set the cameras' position and orientation
   cameraNp.set_pos_hpr(0, -8, 2.5, 0, -9, 0);

   // Load and position our models
   load_models();
   // Add some basic lighting
   setup_lights();
   // Create the needed intervals and put the carousel into motion
   start_carousel();
   }

void World::load_models()
   {
   // Load the carousel base
   NodePath modelsNp = m_windowFrameworkPtr->get_panda_framework()->get_models();
   m_carouselNp = m_windowFrameworkPtr->load_model(modelsNp, "../models/carousel_base");
   // Attach it to render
   NodePath renderNp = m_windowFrameworkPtr->get_render();
   m_carouselNp.reparent_to(renderNp);

   // Load the modeled lights that are on the outer rim of the carousel
   // (not Panda lights)
   // There are 2 groups of lights. At any given time, one group will have the
   // "on" texture and the other will have the "off" texture.
   m_lights1Np = m_windowFrameworkPtr->load_model(modelsNp, "../models/carousel_lights");
   m_lights1Np.reparent_to(m_carouselNp);

   // Load the 2nd set of lights
   m_lights2Np = m_windowFrameworkPtr->load_model(modelsNp, "../models/carousel_lights");
   // We need to rotate the 2nd so it doesn't overlap with the 1st set.
   m_lights2Np.set_h(36);
   m_lights2Np.reparent_to(m_carouselNp);

   // Load the textures for the lights. One texture is for the "on" state,
   // the other is for the "off" state.
   m_lightOffTexPtr = TexturePool::load_texture("../models/carousel_lights_off.jpg");
   m_lightOnTexPtr  = TexturePool::load_texture("../models/carousel_lights_on.jpg" );

   // Create an list (m_pandasNp) with filled with 4 dummy nodes attached to
   // the carousel.
   // This uses a python concept called "Array Comprehensions." Check the Python
   // manual for more information on how they work
   for(int i = 0; i < P_pandas; ++i)
      {
      string nodeName("panda");
      nodeName += i;
      m_pandasNp[i] = m_carouselNp.attach_new_node(nodeName);
      m_modelsNp[i] = m_windowFrameworkPtr->load_model(modelsNp, "../models/carousel_panda");
      // Note: we'll be using a task, we won't need these
      // self.moves = [0 for i in range(4)]

      // set the position and orientation of the ith panda node we just created
      // The Z value of the position will be the base height of the pandas.
      // The headings are multiplied by i to put each panda in its own position
      // around the carousel
      m_pandasNp[i].set_pos_hpr(0, 0, 1.3, i*90, 0, 0);

      // Load the actual panda model, and parent it to its dummy node
      m_modelsNp[i].reparent_to(m_pandasNp[i]);
      // Set the distance from the center. This distance is based on the way the
      // carousel was modeled in Maya
      m_modelsNp[i].set_y(.85);
      }

   // Load the environment (Sky sphere and ground plane)
   m_envNp = m_windowFrameworkPtr->load_model(modelsNp, "../models/env");
   m_envNp.reparent_to(renderNp);
   m_envNp.set_scale(7);
   }

// Panda Lighting
void World::setup_lights()
   {
   NodePath renderNp = m_windowFrameworkPtr->get_render();

   // Create some lights and add them to the scene. By setting the lights on
   // render they affect the entire scene
   // Check out the lighting tutorial for more information on lights
   PT(AmbientLight) ambientLightPtr = new AmbientLight("ambientLight");
   if(ambientLightPtr != NULL)
      {
      ambientLightPtr->set_color(Colorf(0.4, 0.4, 0.35, 1));
      renderNp.set_light(renderNp.attach_new_node(ambientLightPtr));
      }
   PT(DirectionalLight) directionalLightPtr = new DirectionalLight("directionalLight");
   if(directionalLightPtr != NULL)
      {
      directionalLightPtr->set_direction(LVecBase3f(0, 8, -2.5));
      directionalLightPtr->set_color(Colorf(0.9, 0.8, 0.9, 1));
      renderNp.set_light(renderNp.attach_new_node(directionalLightPtr));
      }

   // Explicitly set the environment to not be lit
   m_envNp.set_light_off();
   }

void World::start_carousel()
   {
   // Here's where we actually create the intervals to move the carousel
   // The first type of interval we use is one created directly from a NodePath
   // This interval tells the NodePath to vary its orientation (hpr) from its
   // current value (0,0,0) to (360,0,0) over 20 seconds. Intervals created from
   // NodePaths also exist for position, scale, color, and shear
   m_carouselSpinIntervalPtr = new CLerpNodePathInterval("carouselSpinInterval",
                                                         20,
                                                         CLerpNodePathInterval::BT_no_blend,
                                                         true,
                                                         false,
                                                         m_carouselNp,
                                                         NodePath());
   if(m_carouselSpinIntervalPtr != NULL)
      {
      m_carouselSpinIntervalPtr->set_start_hpr(LVecBase3f(  0, 0, 0));
      m_carouselSpinIntervalPtr->set_end_hpr  (LVecBase3f(360, 0, 0));

      // Once an interval is created, we need to tell it to actually move.
      // start() will cause an interval to play once. loop() will tell an interval
      // to repeat once it finished. To keep the carousel turning, we use loop()
      m_carouselSpinIntervalPtr->loop();
      }

   // The next type of interval we use is called a LerpFunc interval. It is
   // called that because it linearly interpolates (aka Lerp) values passed to
   // a function over a given amount of time.

   // In this specific case, horses on a carousel don't move constantly up,
   // suddenly stop, and then constantly move down again. Instead, they start
   // slowly, get fast in the middle, and slow down at the top. This motion is
   // close to a sine wave. This LerpFunc calls the function oscilatePanda
   // (which we will create below), which changes the height of the panda based
   // on the sin of the value passed in. In this way we achieve non-linear
   // motion by linearly changing the input to a function
   m_lerpFuncPtrVec.resize(P_pandas);
   m_lerpFuncPtrVec[P_panda1] = oscillate_panda<P_panda1>;
   m_lerpFuncPtrVec[P_panda2] = oscillate_panda<P_panda2>;
   m_lerpFuncPtrVec[P_panda3] = oscillate_panda<P_panda3>;
   m_lerpFuncPtrVec[P_panda4] = oscillate_panda<P_panda4>;

   m_moveIntervalPtrVec.resize(P_pandas);
   for(int i = 0; i < P_pandas; ++i)
      {
      string intervalName("moveInterval");
      intervalName += i;
      m_moveIntervalPtrVec[i] = new CLerpFunctionInterval(intervalName,
                                                          m_lerpFuncPtrVec[i],
                                                          this,
                                                          3,
                                                          0,
                                                          2*PI,
                                                          CLerpFunctionInterval::BT_no_blend);
      if(m_moveIntervalPtrVec[i] != NULL)
         {
         m_moveIntervalPtrVec[i]->loop();
         }
      }

   // Finally, we combine Sequence, Parallel, Func, and Wait intervals,
   // to schedule texture swapping on the lights to simulate the lights turning
   // on and off.
   // Sequence intervals play other intervals in a sequence. In other words,
   // it waits for the current interval to finish before playing the next
   // one.
   // Parallel intervals play a group of intervals at the same time
   // Wait intervals simply do nothing for a given amount of time
   // Func intervals simply make a single function call. This is helpful because
   // it allows us to schedule functions to be called in a larger sequence. They
   // take virtually no time so they don't cause a Sequence to wait.

   m_lightBlinkIntervalPtr = new CMetaInterval("lightBlinkInterval");

   if(m_lightBlinkIntervalPtr != NULL)
      {
      // For the first step in our sequence we will set the on texture on one
      // light and set the off texture on the other light at the same time
      m_lightBlinkIntervalPtr->add_c_interval(new GenericFunctionInterval("lights1OnInterval",
                                                                          call_blink_lights<L_light1, B_blink_on>,
                                                                          this,
                                                                          true));
      m_lightBlinkIntervalPtr->add_c_interval(new GenericFunctionInterval("lights2OffInterval",
                                                                          call_blink_lights<L_light2, B_blink_off>,
                                                                          this,
                                                                          true),
                                              0,
                                              CMetaInterval::RS_previous_begin);
      // Then we will wait 1 second
      m_lightBlinkIntervalPtr->add_c_interval(new WaitInterval(1));

      // Then we will switch the textures at the same time
      m_lightBlinkIntervalPtr->add_c_interval(new GenericFunctionInterval("lights1OffInterval",
                                                                          call_blink_lights<L_light1, B_blink_off>,
                                                                          this,
                                                                          true));
      m_lightBlinkIntervalPtr->add_c_interval(new GenericFunctionInterval("lights2OnInterval",
                                                                          call_blink_lights<L_light2, B_blink_on>,
                                                                          this,
                                                                          true),
                                              0,
                                              CMetaInterval::RS_previous_begin);
      // Then we will wait another second
      m_lightBlinkIntervalPtr->add_c_interval(new WaitInterval(1));

      // Loop this sequence continuously
      m_lightBlinkIntervalPtr->loop();
      }

   // Note: setup a task to step the interval manager
   AsyncTaskManager::get_global_ptr()->add(new GenericAsyncTask("intervalManagerTask",
                                                                step_interval_manager,
                                                                this));
   }

template<int pandaId>
void World::oscillate_panda(const double& rad, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   double offset = PI*(pandaId%2);
   static_cast<World*>(dataPtr)->m_modelsNp[pandaId].set_z(sin(rad + offset) * 0.2);
   }

template<int lightId, int blinkId>
void World::call_blink_lights(void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<World*>(dataPtr)->blink_lights((LightId)lightId, (BlinkId)blinkId);
   }

void World::blink_lights(LightId lightId, BlinkId blinkId)
   {
   NodePath lightsNp;
   switch(lightId)
      {
      case L_light1:
         lightsNp = m_lights1Np;
         break;
      case L_light2:
         lightsNp = m_lights2Np;
         break;
      default:
         nout << "ERROR: forgot a LightId?" << endl;
         return;
      }

   switch(blinkId)
      {
      case B_blink_on:
         lightsNp.set_texture(m_lightOnTexPtr);
         break;
      case B_blink_off:
         lightsNp.set_texture(m_lightOffTexPtr);
         break;
      default:
         nout << "ERROR: forgot a BlinkId?" << endl;
         return;
      }
   }

AsyncTask::DoneStatus World::step_interval_manager(GenericAsyncTask* taskPtr, void* dataPtr)
   {
   CIntervalManager::get_global_ptr()->step();
   return AsyncTask::DS_cont;
   }
