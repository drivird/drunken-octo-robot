/*
 * world3.h
 *
 *  Created on: 2012-07-04
 *      Author: dri
 */

#ifndef WORLD3_H_
#define WORLD3_H_

#include "../p3util/cOnscreenText.h"
#include "pandaFramework.h"

class World3
   {
   public:

   World3(WindowFramework* windowFrameworkPtr);

   private:

   World3(); // to prevent use of the default constructor
   void load_planets();

   PT(WindowFramework) m_windowFrameworkPtr;
   COnscreenText m_title;
   float m_sizescale;
   NodePath m_sky;
   PT(Texture) m_skyTex;
   NodePath m_sun;
   PT(Texture) m_sunTex;
   };

#endif /* WORLD3_H_ */
