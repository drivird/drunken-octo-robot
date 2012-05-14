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

   BumpMapDemo(); // to prevent use of the default constructor
   NodePath add_title(const string& text);
   NodePath onscreen_text(const string& text, const Colorf& fg, const LPoint2f& pos, Alignment align, float scale);
   NodePath add_instructions(float pos, const string& msg);
   static AsyncTask::DoneStatus control_camera(GenericAsyncTask* taskPtr, void* dataPtr);
   static AsyncTask::DoneStatus step_interval_manager(GenericAsyncTask* taskPtr, void* dataPtr);
   static void sys_exit(const Event* eventPtr, void* dataPtr);
   static void set_mouse_btn1(const Event* eventPtr, void* dataPtr);
   static void set_mouse_btn1_up(const Event* eventPtr, void* dataPtr);
   static void set_mouse_btn2(const Event* eventPtr, void* dataPtr);
   static void set_mouse_btn2_up(const Event* eventPtr, void* dataPtr);
   static void set_mouse_btn3(const Event* eventPtr, void* dataPtr);
   static void set_mouse_btn3_up(const Event* eventPtr, void* dataPtr);
   static void toggle_shader(const Event* eventPtr, void* dataPtr);
   static void rotate_light(const Event* eventPtr, void* dataPtr);
   static void rotate_light_inv(const Event* eventPtr, void* dataPtr);
   static void rotate_cam(const Event* eventPtr, void* dataPtr);
   static void rotate_cam_inv(const Event* eventPtr, void* dataPtr);

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
