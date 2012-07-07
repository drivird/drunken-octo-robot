/*
 * world6.h
 *
 *  Created on: 2012-07-05
 *      Author: dri
 */

#ifndef WORLD6_H_
#define WORLD6_H_

#include "../p3util/cOnscreenText.h"
#include "cLerpNodePathInterval.h"
#include "pandaFramework.h"

class World6
   {
   public:

   enum Planet
      {
      P_sun,
      P_mercury,
      P_venus,
      P_earth,
      P_moon,
      P_mars
      };

   World6(WindowFramework* windowFrameworkPtr);

   private:

   World6(); // to prevent use of the default constructor
   COnscreenText gen_label_text(const string& text, int i) const;
   void load_planets();
   void rotate_planets();
   void handle_mouse_click();
   void toggle_planet(Planet planetId);
   void toggle_interval(CLerpNodePathInterval* interval);
   void handle_earth();
   void inc_year();

   template<int planet>
   static void call_toggle_planet(const Event* eventPtr, void* dataPtr);
   static void call_handle_mouse_click(const Event* eventPtr, void* dataPtr);
   static void call_handle_earth(const Event* eventPtr, void* dataPtr);
   static void call_inc_year(const Event* eventPtr, void* dataPtr);
   static void sys_exit(const Event* eventPtr, void* dataPtr);
   static AsyncTask::DoneStatus step_interval_manager(GenericAsyncTask* taskPtr,
                                                      void* dataPtr);

   PT(WindowFramework) m_windowFramework;
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
   COnscreenText m_mouse1EventText;
   COnscreenText m_skeyEventText;
   COnscreenText m_ykeyEventText;
   COnscreenText m_vkeyEventText;
   COnscreenText m_ekeyEventText;
   COnscreenText m_mkeyEventText;
   COnscreenText m_yearCounterText;
   int m_yearCounter;
   bool m_simRunning;
   };

#endif /* WORLD6_H_ */
