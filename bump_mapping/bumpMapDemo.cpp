/*
 * bumpMapDemo.cpp
 *
 *  Created on: 2012-05-12
 *      Author: dri
 */

#include "bumpMapDemo.h"
#include "pandaFramework.h"
#include "cLerpNodePathInterval.h"
#include "pointLight.h"
#include "ambientLight.h"
#include "cIntervalManager.h"

// Function to put title on the screen.
NodePath BumpMapDemo::add_title(const string& text) const
   {
   return onscreen_text(text, Colorf(1,1,1,1), LPoint2f(1.3,-0.95), A_right, 0.07);
   }

// Function to put instructions on the screen.
NodePath BumpMapDemo::add_instructions(float pos, const string& msg) const
   {
   return onscreen_text(msg, Colorf(1,1,1,1), LPoint2f(-1.3, pos), A_left, 0.05);
   }

BumpMapDemo::BumpMapDemo(WindowFramework* windowFrameworkPtr)
   : m_windowFrameworkPtr(windowFrameworkPtr)
   {
   // preconditions
   if(m_windowFrameworkPtr == NULL)
      {
      nout << "ERROR: BumpMapDemo::BumpMapDemo(WindowFramework* windowFrameworkPtr) parameter windowFrameworkPtr cannot be NULL." << endl;
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

   m_roomNp = m_windowFrameworkPtr->load_model(m_windowFrameworkPtr->get_panda_framework()->get_models(), "../models/abstractroom");
   m_roomNp.reparent_to(m_windowFrameworkPtr->get_render());

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
   AsyncTaskManager::get_global_ptr()->add(new GenericAsyncTask("camera-task", control_camera, this));
   m_windowFrameworkPtr->enable_keyboard();
   m_windowFrameworkPtr->get_panda_framework()->define_key("escape"     , "sysExit"            , sys_exit             , NULL);
   m_windowFrameworkPtr->get_panda_framework()->define_key("mouse1"     , "setMouseBtn1"       , set_mouse_btn1       , this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("mouse1-up"  , "setMouseBtn1Up"     , set_mouse_btn1_up    , this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("mouse2"     , "setMouseBtn2"       , set_mouse_btn2       , this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("mouse2-up"  , "setMouseBtn2Up"     , set_mouse_btn2_up    , this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("mouse3"     , "setMouseBtn3"       , set_mouse_btn3       , this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("mouse3-up"  , "setMouseBtn3Up"     , set_mouse_btn3_up    , this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("enter"      , "toggleShader"       , call_toggle_shader   , this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("j"          , "rotateLightNegative", rotate_light_negative, this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("k"          , "rotateLightPositive", rotate_light_positive, this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("arrow_left" , "rotateCamNegative"  , rotate_cam_negative  , this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("arrow_right", "rotateCamPositive"  , rotate_cam_positive  , this);

   // Add a light to the scene.
   m_lightPivotNp = m_windowFrameworkPtr->get_render().attach_new_node("lightpivot");
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
   AsyncTaskManager::get_global_ptr()->add(new GenericAsyncTask("stepIntervalManager", step_interval_manager, NULL));

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
      NodePath ambientLightNp = m_windowFrameworkPtr->get_render().attach_new_node(ambientLightPtr);
      m_roomNp.set_light(ambientLightNp);
      }

   // create a sphere to denote the light
   NodePath sphereNp = m_windowFrameworkPtr->load_model(m_windowFrameworkPtr->get_panda_framework()->get_models(), "../models/sphere");
   sphereNp.reparent_to(pointLightNp);

   // load and apply the shader.  This is using panda's
   // built-in shader generation capabilities to create the
   // shader for you.  However, if desired, you can supply
   // the shader manually.  Change this line of code to:
   //   m_roomNp.set_shader(Shader::load("bumpMapper.sha"));
   m_roomNp.set_shader_auto();

   m_shaderEnable = true;
   }

void BumpMapDemo::set_mouse_btn(int btn, bool value)
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

AsyncTask::DoneStatus BumpMapDemo::control_camera(GenericAsyncTask* taskPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: AsyncTask::DoneStatus BumpMapDemo::control_camera(GenericAsyncTask* taskPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return AsyncTask::DS_done;
      }

   BumpMapDemo* bumpMapDemoPtr = static_cast<BumpMapDemo*>(dataPtr);
   // figure out how much the mouse has moved (in pixels)
   MouseData mouseData = bumpMapDemoPtr->m_windowFrameworkPtr->get_graphics_window()->get_pointer(0);
   int x = mouseData.get_x();
   int y = mouseData.get_y();
   if(bumpMapDemoPtr->m_windowFrameworkPtr->get_graphics_window()->move_pointer(0, 100, 100))
      {
      bumpMapDemoPtr->m_heading -= (x - 100) * 0.2;
      bumpMapDemoPtr->m_pitch   -= (y - 100) * 0.2;
      }
   if(bumpMapDemoPtr->m_pitch < -45){ bumpMapDemoPtr->m_pitch = -45; }
   if(bumpMapDemoPtr->m_pitch >  45){ bumpMapDemoPtr->m_pitch =  45; }
   NodePath cameraNp = bumpMapDemoPtr->m_windowFrameworkPtr->get_camera_group();
   cameraNp.set_hpr(bumpMapDemoPtr->m_heading, bumpMapDemoPtr->m_pitch, 0);
   LVecBase3f dir = cameraNp.get_mat().get_row3(1);
   double elapsed = taskPtr->get_elapsed_time() - bumpMapDemoPtr->m_last;
   if(bumpMapDemoPtr->m_last == 0)
      {
      elapsed = 0;
      }
   if(bumpMapDemoPtr->m_mouseBtn[B_btn1])
      {
      bumpMapDemoPtr->m_focus += dir * elapsed*30;
      }
   if (bumpMapDemoPtr->m_mouseBtn[B_btn2] || bumpMapDemoPtr->m_mouseBtn[B_btn3])
      {
      bumpMapDemoPtr->m_focus -= dir * elapsed*30;
      }
   cameraNp.set_pos(bumpMapDemoPtr->m_focus - dir*5);
   if(cameraNp.get_x() < -59.0){ cameraNp.set_x(-59); }
   if(cameraNp.get_x() >  59.0){ cameraNp.set_x( 59); }
   if(cameraNp.get_y() < -59.0){ cameraNp.set_y(-59); }
   if(cameraNp.get_y() >  59.0){ cameraNp.set_y( 59); }
   if(cameraNp.get_z() <   5.0){ cameraNp.set_z(  5); }
   if(cameraNp.get_z() >  45.0){ cameraNp.set_z( 45); }
   bumpMapDemoPtr->m_focus = cameraNp.get_pos() + dir*5;
   bumpMapDemoPtr->m_last = taskPtr->get_elapsed_time();
   return AsyncTask::DS_cont;
   }

// Note: OnscreenText is a python only function. It's capabilities are emulated here
//       to simplify the translation to C++.
NodePath BumpMapDemo::onscreen_text(const string& text, const Colorf& fg, const LPoint2f& pos, Alignment align, float scale) const
   {
   NodePath textNodeNp;

   if(m_windowFrameworkPtr != NULL)
      {
      PT(TextNode) textNodePtr = new TextNode("OnscreenText");
      if(textNodePtr != NULL)
         {
         textNodePtr->set_text(text);
         textNodePtr->set_text_color(fg);
         textNodePtr->set_align(static_cast<TextNode::Alignment>(align));
         textNodeNp = m_windowFrameworkPtr->get_aspect_2d().attach_new_node(textNodePtr);
         textNodeNp.set_pos(pos.get_x(), 0, pos.get_y());
         textNodeNp.set_scale(scale);
         }
      }

   return textNodeNp;
   }

void BumpMapDemo::sys_exit(const Event* eventPtr, void* dataPtr)
   {
   exit(0);
   }

void BumpMapDemo::set_mouse_btn1(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void BumpMapDemo::set_mouse_btn1(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   BumpMapDemo* bumpMapDemoPtr = static_cast<BumpMapDemo*>(dataPtr);
   bumpMapDemoPtr->set_mouse_btn(B_btn1, true);
   }

void BumpMapDemo::set_mouse_btn1_up(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void BumpMapDemo::set_mouse_btn1_up(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   BumpMapDemo* bumpMapDemoPtr = static_cast<BumpMapDemo*>(dataPtr);
   bumpMapDemoPtr->set_mouse_btn(B_btn1, false);
   }

void BumpMapDemo::set_mouse_btn2(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void BumpMapDemo::set_mouse_btn2(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   BumpMapDemo* bumpMapDemoPtr = static_cast<BumpMapDemo*>(dataPtr);
   bumpMapDemoPtr->set_mouse_btn(B_btn2, true);
   }

void BumpMapDemo::set_mouse_btn2_up(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void BumpMapDemo::set_mouse_btn2_up(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   BumpMapDemo* bumpMapDemoPtr = static_cast<BumpMapDemo*>(dataPtr);
   bumpMapDemoPtr->set_mouse_btn(B_btn2, false);
   }

void BumpMapDemo::set_mouse_btn3(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void BumpMapDemo::set_mouse_btn3(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   BumpMapDemo* bumpMapDemoPtr = static_cast<BumpMapDemo*>(dataPtr);
   bumpMapDemoPtr->set_mouse_btn(B_btn3, true);
   }

void BumpMapDemo::set_mouse_btn3_up(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void BumpMapDemo::set_mouse_btn3_up(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   BumpMapDemo* bumpMapDemoPtr = static_cast<BumpMapDemo*>(dataPtr);
   bumpMapDemoPtr->set_mouse_btn(B_btn3, false);
   }

void BumpMapDemo::call_toggle_shader(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void BumpMapDemo::toggle_shader(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   BumpMapDemo* bumpMapDemoPtr = static_cast<BumpMapDemo*>(dataPtr);
   bumpMapDemoPtr->toggle_shader();
   }

void BumpMapDemo::rotate_light_positive(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void BumpMapDemo::rotate_light_inv(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   BumpMapDemo* bumpMapDemoPtr = static_cast<BumpMapDemo*>(dataPtr);
   bumpMapDemoPtr->rotate_light(O_positive);
   }

void BumpMapDemo::rotate_light_negative(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void BumpMapDemo::rotate_light(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   BumpMapDemo* bumpMapDemoPtr = static_cast<BumpMapDemo*>(dataPtr);
   bumpMapDemoPtr->rotate_light(O_negative);
   }

void BumpMapDemo::rotate_cam_negative(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void BumpMapDemo::rotate_cam(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   BumpMapDemo* bumpMapDemoPtr = static_cast<BumpMapDemo*>(dataPtr);
   bumpMapDemoPtr->rotate_cam(O_negative);
   }

void BumpMapDemo::rotate_cam_positive(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void BumpMapDemo::rotate_cam_inv(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   BumpMapDemo* bumpMapDemoPtr = static_cast<BumpMapDemo*>(dataPtr);
   bumpMapDemoPtr->rotate_cam(O_positive);
   }

AsyncTask::DoneStatus BumpMapDemo::step_interval_manager(GenericAsyncTask* taskPtr, void* dataPtr)
   {
   CIntervalManager::get_global_ptr()->step();
   return AsyncTask::DS_cont;
   }
