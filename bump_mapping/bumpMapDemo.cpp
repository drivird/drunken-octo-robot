/*
 * bumpMapDemo.cpp
 *
 *  Created on: 2012-05-12
 *      Author: dri
 */

#include "../p3util/cOnscreenText.h"
#include "bumpMapDemo.h"
#include "pandaFramework.h"
#include "cLerpNodePathInterval.h"
#include "pointLight.h"
#include "ambientLight.h"
#include "cIntervalManager.h"

// Function to put instructions on the screen.
NodePath BumpMapDemo::add_instructions(float pos, const string& msg) const
   {
   COnscreenText instructions("instructions", COnscreenText::TS_plain);
   instructions.set_text(msg);
   instructions.set_fg(Colorf(1,1,1,1));
   instructions.set_pos(LVecBase2f(-1.3, pos));
   instructions.set_align(TextNode::A_left);
   instructions.set_scale(0.05);
   instructions.reparent_to(m_windowFrameworkPtr->get_aspect_2d());
   return instructions.generate();
   }

// Function to put title on the screen.
NodePath BumpMapDemo::add_title(const string& text) const
   {
   COnscreenText title("title", COnscreenText::TS_plain);
   title.set_text(text);
   title.set_fg(Colorf(1,1,1,1));
   title.set_pos(LVecBase2f(1.3,-0.95));
   title.set_align(TextNode::A_right);
   title.set_scale(0.07);
   title.reparent_to(m_windowFrameworkPtr->get_aspect_2d());
   return title.generate();
   }

BumpMapDemo::BumpMapDemo(WindowFramework* windowFrameworkPtr)
   : m_windowFrameworkPtr(windowFrameworkPtr)
   {
   // preconditions
   if(m_windowFrameworkPtr == NULL)
      {
      nout << "ERROR: parameter windowFrameworkPtr cannot be NULL." << endl;
      return;
      }

   // Check video card capabilities.

   if(!m_windowFrameworkPtr->get_graphics_window()->get_gsg()->get_supports_basic_shaders())
      {
      add_title("Bump Mapping: Video driver reports that shaders are not supported.");
      return;
      }

   // Post the instructions
   m_titleNp = add_title("Panda3D: Tutorial - Bump Mapping");
   m_inst1Np = add_instructions(0.95, "Press ESC to exit");
   m_inst2Np = add_instructions(0.90, "Move mouse to rotate camera");
   m_inst3Np = add_instructions(0.85, "Left mouse button: Move forwards");
   m_inst4Np = add_instructions(0.80, "Right mouse button: Move backwards");
   m_inst5Np = add_instructions(0.75, "Enter: Turn bump maps Off");

   // Load the 'abstract room' model.  This is a model of an
   // empty room containing a pillar, a pyramid, and a bunch
   // of exaggeratedly bumpy textures.

   NodePath modelsNp = m_windowFrameworkPtr->get_panda_framework()->get_models();
   m_roomNp = m_windowFrameworkPtr->load_model(modelsNp, "../models/abstractroom");
   NodePath renderNp = m_windowFrameworkPtr->get_render();
   m_roomNp.reparent_to(renderNp);

   // Make the mouse invisible, turn off normal mouse controls
   // Note: mouse controls are off by default in C++.
   WindowProperties props;
   props.set_cursor_hidden(true);
   m_windowFrameworkPtr->get_graphics_window()->request_properties(props);

   // Set the current viewing target
   m_focus = LVecBase3f(55,-55,20);
   m_heading = 180;
   m_pitch = 0;
   m_mouseX = 0; // Note: unused
   m_mouseY = 0; // Note: unused
   m_last = 0;
   m_mouseBtn.resize(B_buttons);
   m_mouseBtn[B_btn1] = false;
   m_mouseBtn[B_btn2] = false;
   m_mouseBtn[B_btn3] = false;

   // Start the camera control task:
   PT(GenericAsyncTask) controlCameraTaskPtr = new GenericAsyncTask("camera-task", call_control_camera, this);
   if(controlCameraTaskPtr != NULL)
      {
      AsyncTaskManager::get_global_ptr()->add(controlCameraTaskPtr);
      }
   m_windowFrameworkPtr->enable_keyboard();
   m_windowFrameworkPtr->get_panda_framework()->define_key("escape"     , "sysExit"            , sys_exit             , NULL);
   m_windowFrameworkPtr->get_panda_framework()->define_key("mouse1"     , "setMouseBtn1"       , call_set_mouse_btn<B_btn1, true >, this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("mouse1-up"  , "setMouseBtn1Up"     , call_set_mouse_btn<B_btn1, false>, this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("mouse2"     , "setMouseBtn2"       , call_set_mouse_btn<B_btn2, true >, this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("mouse2-up"  , "setMouseBtn2Up"     , call_set_mouse_btn<B_btn2, false>, this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("mouse3"     , "setMouseBtn3"       , call_set_mouse_btn<B_btn3, true >, this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("mouse3-up"  , "setMouseBtn3Up"     , call_set_mouse_btn<B_btn3, false>, this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("enter"      , "toggleShader"       , call_toggle_shader               , this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("j"          , "rotateLightNegative", call_rotate_light<O_negative    >, this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("k"          , "rotateLightPositive", call_rotate_light<O_positive    >, this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("arrow_left" , "rotateCamNegative"  , call_rotate_cam<O_negative      >, this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("arrow_right", "rotateCamPositive"  , call_rotate_cam<O_positive      >, this);

   // Add a light to the scene.
   m_lightPivotNp = renderNp.attach_new_node("lightpivot");
   m_lightPivotNp.set_pos(0, 0, 25);
   // Note: the hprInterval function is a python only extension to NodePath
   PT(CLerpNodePathInterval) cLerpNodePathIntervalPtr = new CLerpNodePathInterval("hprInterval",
                                                                                  10,
                                                                                  CLerpNodePathInterval::BT_no_blend,
                                                                                  true,
                                                                                  false,
                                                                                  m_lightPivotNp,
                                                                                  NodePath());
   if(cLerpNodePathIntervalPtr != NULL)
      {
      cLerpNodePathIntervalPtr->set_start_hpr(LPoint3f(0,0,0));
      cLerpNodePathIntervalPtr->set_end_hpr(LPoint3f(360,0,0));
      // Note: comment the following line if you want keys `j' and `k' to have an effect.
      cLerpNodePathIntervalPtr->loop();
      }
   PT(GenericAsyncTask) stepIntervalManagerTaskPtr = new GenericAsyncTask("stepIntervalManager", step_interval_manager, NULL);
   if(stepIntervalManagerTaskPtr != NULL)
      {
      AsyncTaskManager::get_global_ptr()->add(stepIntervalManagerTaskPtr);
      }


   NodePath pointLightNp;
   PT(PointLight) pointLightPtr = new PointLight("plight");
   if(pointLightPtr != NULL)
      {
      pointLightPtr->set_color(Colorf(1, 1, 1, 1));
      pointLightPtr->set_attenuation(LVecBase3f(0.7,0.05,0));
      pointLightNp = m_lightPivotNp.attach_new_node(pointLightPtr);
      pointLightNp.set_pos(45, 0, 0);
      m_roomNp.set_light(pointLightNp);
      m_roomNp.set_shader_input("light", pointLightNp);
      }

   // Add an ambient light
   PT(AmbientLight) ambientLightPtr = new AmbientLight("alight");
   if(ambientLightPtr != NULL)
      {
      ambientLightPtr->set_color(Colorf(0.2, 0.2, 0.2, 1));
      NodePath ambientLightNp = renderNp.attach_new_node(ambientLightPtr);
      m_roomNp.set_light(ambientLightNp);
      }

   // create a sphere to denote the light
   NodePath sphereNp = m_windowFrameworkPtr->load_model(modelsNp, "../models/sphere");
   sphereNp.reparent_to(pointLightNp);

   // load and apply the shader.  This is using panda's
   // built-in shader generation capabilities to create the
   // shader for you.  However, if desired, you can supply
   // the shader manually.  Change this line of code to:
   //   m_roomNp.set_shader(Shader::load("bumpMapper.sha"));
   m_roomNp.set_shader_auto();

   m_shaderEnable = true;
   }

void BumpMapDemo::set_mouse_btn(Button btn, bool value)
   {
   m_mouseBtn[btn] = value;
   }

void BumpMapDemo::rotate_light(Offset offset)
   {
   m_lightPivotNp.set_h(m_lightPivotNp.get_h() + offset*20);
   }

void BumpMapDemo::rotate_cam(Offset offset)
   {
   m_heading -= offset*10;
   }

void BumpMapDemo::toggle_shader()
   {
   m_inst5Np.remove_node();
   if(m_shaderEnable)
      {
      m_inst5Np = add_instructions(0.75, "Enter: Turn bump maps On");
      m_shaderEnable = false;
      m_roomNp.set_shader_off();
      }
   else
      {
      m_inst5Np = add_instructions(0.75, "Enter: Turn bump maps Off");
      m_shaderEnable = true;
      m_roomNp.set_shader_auto();
      }
   }

void BumpMapDemo::control_camera(GenericAsyncTask* taskPtr)
   {
   // figure out how much the mouse has moved (in pixels)
   MouseData mouseData = m_windowFrameworkPtr->get_graphics_window()->get_pointer(0);
   int x = mouseData.get_x();
   int y = mouseData.get_y();
   if(m_windowFrameworkPtr->get_graphics_window()->move_pointer(0, 100, 100))
      {
      m_heading -= (x - 100) * 0.2;
      m_pitch   -= (y - 100) * 0.2;
      }
   if(m_pitch < -45){ m_pitch = -45; }
   if(m_pitch >  45){ m_pitch =  45; }
   NodePath cameraNp = m_windowFrameworkPtr->get_camera_group();
   cameraNp.set_hpr(m_heading, m_pitch, 0);
   LVecBase3f dir = cameraNp.get_mat().get_row3(1);
   double elapsed = taskPtr->get_elapsed_time() - m_last;
   if(m_last == 0)
      {
      elapsed = 0;
      }
   if(m_mouseBtn[B_btn1])
      {
      m_focus += dir * elapsed*30;
      }
   if (m_mouseBtn[B_btn2] || m_mouseBtn[B_btn3])
      {
      m_focus -= dir * elapsed*30;
      }
   cameraNp.set_pos(m_focus - dir*5);
   if(cameraNp.get_x() < -59.0){ cameraNp.set_x(-59); }
   if(cameraNp.get_x() >  59.0){ cameraNp.set_x( 59); }
   if(cameraNp.get_y() < -59.0){ cameraNp.set_y(-59); }
   if(cameraNp.get_y() >  59.0){ cameraNp.set_y( 59); }
   if(cameraNp.get_z() <   5.0){ cameraNp.set_z(  5); }
   if(cameraNp.get_z() >  45.0){ cameraNp.set_z( 45); }
   m_focus = cameraNp.get_pos() + dir*5;
   m_last = taskPtr->get_elapsed_time();
   }

void BumpMapDemo::sys_exit(const Event* eventPtr, void* dataPtr)
   {
   exit(0);
   }

template<int btn, bool value>
void BumpMapDemo::call_set_mouse_btn(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }
   if(btn < 0 || btn >= B_buttons)
      {
      nout << "ERROR: parameter btn is out of range: " << btn << endl;
      return;
      }

   static_cast<BumpMapDemo*>(dataPtr)->set_mouse_btn(static_cast<Button>(btn), value);
   }

void BumpMapDemo::call_toggle_shader(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   BumpMapDemo* bumpMapDemoPtr = static_cast<BumpMapDemo*>(dataPtr);
   bumpMapDemoPtr->toggle_shader();
   }

template<int offset>
void BumpMapDemo::call_rotate_light(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }
   if(!(offset == -1 || offset == 1))
      {
      nout << "ERROR: parameter offset is illegal: " << offset << endl;
      return;
      }

   static_cast<BumpMapDemo*>(dataPtr)->rotate_light(static_cast<Offset>(offset));
   }

template<int offset>
void BumpMapDemo::call_rotate_cam(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }
   if(!(offset == -1 || offset == 1))
      {
      nout << "ERROR: parameter offset is illegal: " << offset << endl;
      return;
      }

   static_cast<BumpMapDemo*>(dataPtr)->rotate_cam(static_cast<Offset>(offset));
   }

AsyncTask::DoneStatus BumpMapDemo::call_control_camera(GenericAsyncTask* taskPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return AsyncTask::DS_done;
      }

   BumpMapDemo* bumpMapDemoPtr = static_cast<BumpMapDemo*>(dataPtr);
   bumpMapDemoPtr->control_camera(taskPtr);
   return AsyncTask::DS_cont;
   }

AsyncTask::DoneStatus BumpMapDemo::step_interval_manager(GenericAsyncTask* taskPtr, void* dataPtr)
   {
   CIntervalManager::get_global_ptr()->step();
   return AsyncTask::DS_cont;
   }
