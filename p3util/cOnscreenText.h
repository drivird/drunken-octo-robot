/*
 * cOnscreenText.h
 *
 *  Created on: 2012-05-28
 *      Author: dri
 *
 * onscreenText is a python only function. It's capabilities are emulated here
 * to simplify the translation to C++.
 */

#ifndef CONSCREENTEXT_H_
#define CONSCREENTEXT_H_

#include "pandaFramework.h"

class COnscreenText : public NodePath
   {
   public:

   enum Alignment
      {
      A_center = TextNode::A_center,
      A_left   = TextNode::A_left
      };

   COnscreenText(WindowFramework* windowFrameworkPtr,
                 const string& text,
                 const Colorf& fg,
                 const LPoint2f& pos,
                 Alignment align,
                 float scale);
   virtual ~COnscreenText();

   private:

   COnscreenText(); // prevent use of the default constructor

   PT(WindowFramework) m_windowFrameworkPtr;
   };

#endif /* CONSCREENTEXT_H_ */
