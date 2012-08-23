/*
 * motionTrails.h
 *
 *  Created on: 2012-08-18
 *      Author: dri
 */

#ifndef MOTIONTRAILS_H_
#define MOTIONTRAILS_H_

#include "pandaFramework.h"

class MotionTrails
   {
   public:

   MotionTrails(WindowFramework* windowFramework);

   private:
   NodePath add_instructions(float pos, const string& msg) const;
   void choose_effect_ghost(const Event* event = NULL);
   void choose_effect_paint_brush(const Event* event);
   void choose_effect_double_vision(const Event* event);
   void choose_effect_wings_of_blue(const Event* event);
   void choose_effect_whirlpool(const Event* event);
   AsyncTask::DoneStatus take_snap_shot(GenericAsyncTask *task);
   void quit(const Event* event);

   PT(WindowFramework) m_windowFramework;
   PT(Texture) m_tex;
   NodePath m_backcam;
   NodePath m_background;
   NodePath m_bcard;
   NodePath m_fcard;
   float m_clickrate;
   float m_nextclick;
   NodePath m_title;
   NodePath m_instr0;
   NodePath m_instr1;
   NodePath m_instr2;
   NodePath m_instr3;
   NodePath m_instr4;
   NodePath m_instr5;
   };

#endif /* MOTIONTRAILS_H_ */
