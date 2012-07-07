/*
 * world2.h
 *
 *  Created on: 2012-07-04
 *      Author: dri
 */

#ifndef WORLD2_H_
#define WORLD2_H_

#include "../p3util/cOnscreenText.h"
#include "pandaFramework.h"

class World2
   {
   public:

   World2(WindowFramework* windowFrameworkPtr);

   private:

   World2(); // to prevent use of the default constructor
   PT(WindowFramework) m_windowFrameworkPtr;
   COnscreenText m_title;
   };

#endif /* WORLD2_H_ */
