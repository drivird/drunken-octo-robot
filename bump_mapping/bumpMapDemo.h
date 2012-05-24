/*
 * bumpMapDemo.h
 *
 *  Created on: 2012-05-12
 *      Author: dri
 */

#ifndef BUMPMAPDEMO_H_
#define BUMPMAPDEMO_H_

#include "windowFramework.h"

class BumpMapDemo
   {
   public:

   BumpMapDemo(WindowFramework* windowFrameworkPtr);

   private:

   enum Alignment
      {
      A_left   = TextNode::A_left,
      A_right  = TextNode::A_right
      };

   enum Button
      {
      B_btn1 = 0,
      B_btn2,
      B_btn3,
      B_buttons
      };

   enum Offset
      {
      O_positive = 1,
      O_negative = -1
      };

   NodePath add_title(const string& text) const;
   NodePath add_instructions(float pos, const string& msg) const;
   void set_mouse_btn(Button btn, bool value);
   void rotate_light(Offset offset);
   void rotate_cam(Offset offset);
   void toggle_shader();
   void control_camera(GenericAsyncTask* taskPtr);

   BumpMapDemo(); // to prevent use of the default constructor
   NodePath onscreen_text(const string& text, const Colorf& fg, const LPoint2f& pos, Alignment align, float scale) const;
   static void sys_exit(const Event* eventPtr, void* dataPtr);
   template<int btn, bool value> static void call_set_mouse_btn(const Event* eventPtr, void* dataPtr);
   static void call_toggle_shader(const Event* eventPtr, void* dataPtr);
   template<int offset> static void call_rotate_light(const Event* eventPtr, void* dataPtr);
   template<int offset> static void call_rotate_cam(const Event* eventPtr, void* dataPtr);
   static AsyncTask::DoneStatus call_control_camera(GenericAsyncTask* taskPtr, void* dataPtr);
   static AsyncTask::DoneStatus step_interval_manager(GenericAsyncTask* taskPtr, void* dataPtr);

   PT(WindowFramework) m_windowFrameworkPtr;
   NodePath m_titleNp;
   NodePath m_inst1Np;
   NodePath m_inst2Np;
   NodePath m_inst3Np;
   NodePath m_inst4Np;
   NodePath m_inst5Np;
   NodePath m_roomNp;
   NodePath m_lightPivotNp;
   LVecBase3f m_focus;
   float m_heading;
   float m_pitch;
   float m_mouseX; // Note: unused
   float m_mouseY; // Note: unused
   double m_last;
   vector<bool> m_mouseBtn;
   bool m_shaderEnable;
   };

#endif /* BUMPMAPDEMO_H_ */
