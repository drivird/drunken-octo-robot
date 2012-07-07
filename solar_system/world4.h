/*
 * world4.h
 *
 *  Created on: 2012-07-04
 *      Author: dri
 */

#ifndef WORLD4_H_
#define WORLD4_H_

#include "../p3util/cOnscreenText.h"
#include "pandaFramework.h"

class World4
   {
   public:

   World4(WindowFramework* windowFrameworkPtr);

   private:

   World4(); // to prevent use of the default constructor
   void load_planets();

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
   };

#endif /* WORLD4_H_ */
