/*
 * robot.h
 *
 *  Created on: 2012-05-24
 *      Author: dri
 *
 * This class is a specialized actor class with animation intervals. It sh/could harbor
 * the actor initialization and also each interval initialization code. However, in
 * order to remain close to the original python tutorial, it does not.
 */

#ifndef ROBOT_H_
#define ROBOT_H_

#include "../p3util/cActor.h"
#include "cMetaInterval.h"

class Robot : public CActor
   {
   public:

   Robot();
   virtual ~Robot();

   PT(CMetaInterval) m_punchLeftPtr;
   PT(CMetaInterval) m_punchRightPtr;
   PT(CMetaInterval) m_resetHeadPtr;
   };

#endif /* ROBOT_H_ */
