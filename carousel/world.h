/*
 * world.h
 *
 *  Created on: 2012-05-29
 *      Author: dri
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "cLerpNodePathInterval.h"
#include "cMetaInterval.h"

class World
   {
   public:

   World(WindowFramework* windowFrameworkPtr);

   private:

   static const double PI = 3.14159265;

   enum LightId
      {
      L_light1,
      L_light2
      };

   enum BlinkId
      {
      B_blink_on,
      B_blink_off
      };

   void load_models();
   void setup_lights();
   void start_carousel();
   static void oscillate_panda(double rad, NodePath pandaNp, double offset);

   World(); // to prevent use of the default constructor
   static AsyncTask::DoneStatus call_oscillate_pandas(GenericAsyncTask* taskPtr, void* dataPtr);
   void oscillate_pandas(double dt);
   template<int lightId, int blinkId> static void call_blink_lights(void* dataPtr);
   void blink_lights(LightId lightId, BlinkId blinkId);

   PT(WindowFramework) m_windowFrameworkPtr;
   PT(Texture) m_lightOffTexPtr;
   PT(Texture) m_lightOnTexPtr;
   PT(CLerpNodePathInterval) m_carouselSpinIntervalPtr;
   PT(CMetaInterval) m_lightBlinkIntervalPtr;
   NodePath m_titleNp;
   NodePath m_carouselNp;
   NodePath m_lights1Np;
   NodePath m_lights2Np;
   NodePath m_envNp;
   vector<NodePath> m_pandasNp;
   vector<NodePath> m_modelsNp;
   };

#endif /* WORLD_H_ */
