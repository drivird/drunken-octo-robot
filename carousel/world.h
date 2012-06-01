/*
 * world.h
 *
 *  Created on: 2012-05-29
 *      Author: dri
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "../p3util/cLerpFunctionInterval.h"
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

   enum PandaId
      {
      P_panda1,
      P_panda2,
      P_panda3,
      P_panda4,
      P_pandas
      };

   void load_models();
   void setup_lights();
   void start_carousel();
   template<int pandaId> static void oscillate_panda(const double& rad, void* dataPtr);

   World(); // to prevent use of the default constructor
   template<int lightId, int blinkId> static void call_blink_lights(void* dataPtr);
   void blink_lights(LightId lightId, BlinkId blinkId);
   static AsyncTask::DoneStatus step_interval_manager(GenericAsyncTask* taskPtr, void* dataPtr);

   PT(WindowFramework) m_windowFrameworkPtr;
   PT(Texture) m_lightOffTexPtr;
   PT(Texture) m_lightOnTexPtr;
   PT(CLerpNodePathInterval) m_carouselSpinIntervalPtr;
   PT(CMetaInterval) m_lightBlinkIntervalPtr;
   vector<PT(CLerpFunctionInterval)> m_moveIntervalPtrVec;
   vector<CLerpFunctionInterval::LerpFunc*> m_lerpFuncPtrVec;
   NodePath m_titleNp;
   NodePath m_carouselNp;
   NodePath m_lights1Np;
   NodePath m_lights2Np;
   NodePath m_envNp;
   vector<NodePath> m_pandasNp;
   vector<NodePath> m_modelsNp;
   };

#endif /* WORLD_H_ */
