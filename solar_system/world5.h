/*
 * world5.h
 *
 *  Created on: 2012-07-04
 *      Author: dri
 */

#ifndef WORLD5_H_
#define WORLD5_H_

#include "../p3util/cOnscreenText.h"
#include "cLerpNodePathInterval.h"
#include "pandaFramework.h"

class World5
   {
   public:

   World5(WindowFramework* windowFrameworkPtr);

   private:

   World5(); // to prevent use of the default constructor
   void load_planets();
   void rotate_planets();
   static AsyncTask::DoneStatus step_interval_manager(GenericAsyncTask* taskPtr,
                                                      void* dataPtr);

   PT(WindowFramework) m_windowFrameworkPtr;
   COnscreenText m_title;
   float m_sizescale;
   NodePath m_sky;
   PT(Texture) m_skyTex;
   NodePath m_sun;
   PT(Texture) m_sunTex;
   float m_orbitscale;
   NodePath m_orbitRootMercury;
   NodePath m_orbitRootVenus;
   NodePath m_orbitRootMars;
   NodePath m_orbitRootEarth;
   NodePath m_orbitRootMoon;
   NodePath m_mercury;
   PT(Texture) m_mercuryTex;
   NodePath m_venus;
   PT(Texture) m_venusTex;
   NodePath m_mars;
   PT(Texture) m_marsTex;
   NodePath m_earth;
   PT(Texture) m_earthTex;
   NodePath m_moon;
   PT(Texture) m_moonTex;
   float m_yearscale;
   float m_dayscale;
   PT(CLerpNodePathInterval) m_dayPeriodSun;
   PT(CLerpNodePathInterval) m_orbitPeriodMercury;
   PT(CLerpNodePathInterval) m_dayPeriodMercury;
   PT(CLerpNodePathInterval) m_orbitPeriodVenus;
   PT(CLerpNodePathInterval) m_dayPeriodVenus;
   PT(CLerpNodePathInterval) m_orbitPeriodEarth;
   PT(CLerpNodePathInterval) m_dayPeriodEarth;
   PT(CLerpNodePathInterval) m_orbitPeriodMoon;
   PT(CLerpNodePathInterval) m_dayPeriodMoon;
   PT(CLerpNodePathInterval) m_orbitPeriodMars;
   PT(CLerpNodePathInterval) m_dayPeriodMars;
   };

#endif /* WORLD5_H_ */
