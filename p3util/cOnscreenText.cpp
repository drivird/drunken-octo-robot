/*
 * cOnscreenText.cpp
 *
 *  Created on: 2012-05-28
 *      Author: dri
 */

#include "cOnscreenText.h"

COnscreenText::COnscreenText(WindowFramework* windowFrameworkPtr,
                             const string& text,
                             const Colorf& fg,
                             const LPoint2f& pos,
                             Alignment align,
                             float scale)
   : m_windowFrameworkPtr(windowFrameworkPtr)
   {
   if(m_windowFrameworkPtr == NULL)
      {
      nout << "ERROR: parameter windowFramework cannot be NULL." << endl;
      return;
      }

   PT(TextNode) textNodePtr = new TextNode("OnscreenText");
   if(textNodePtr != NULL)
      {
      textNodePtr->set_text(text);
      textNodePtr->set_text_color(fg);
      textNodePtr->set_align(static_cast<TextNode::Alignment>(align));
      NodePath::operator=(m_windowFrameworkPtr->get_aspect_2d().attach_new_node(textNodePtr));
      set_pos(pos.get_x(), 0, pos.get_y());
      set_scale(scale);
      }
   }

COnscreenText::~COnscreenText()
   {
   ;
   }
