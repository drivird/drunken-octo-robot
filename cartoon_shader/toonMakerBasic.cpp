/*
 * toonMakerBasic.cpp
 *
 *  Created on: 2012-06-23
 *      Author: dri
 */

#include "cLerpNodePathInterval.h"
#include "cIntervalManager.h"
#include "ambientLight.h"
#include "pointLight.h"
#include "lightRampAttrib.h"
#include "toonMakerBasic.h"

// Function to put instructions on the screen.
NodePath ToonMakerBasic::add_instructions(float pos, const string& msg)
   {
   COnscreenText instruction("instruction");
   instruction.set_text(msg);
   instruction.set_fg(Colorf(1, 1, 1, 1));
   instruction.set_pos(LVecBase2f(-1.3, pos));
   instruction.set_align(TextNode::A_left);
   instruction.set_scale(0.05);
   instruction.reparent_to(m_windowFrameworkPtr->get_aspect_2d());
   return instruction.generate();
   }

// Function to put title on the screen.
NodePath ToonMakerBasic::add_title(const string& text)
   {
   COnscreenText title("title");
   title.set_text(text);
   title.set_fg(Colorf(1, 1, 1, 1));
   title.set_pos(LVecBase2f(1.3, -0.95));
   title.set_align(TextNode::A_right);
   title.set_scale(0.07);
   title.reparent_to(m_windowFrameworkPtr->get_aspect_2d());
   // Note: set the draw order to make sure the the BufferViewer
   //       renders in front of the title just like in the
   //       Python original version.
   title.set_draw_order(0);
   return title.generate();
   }

ToonMakerBasic::ToonMakerBasic(WindowFramework* windowFrameworkPtr)
   : m_windowFrameworkPtr(windowFrameworkPtr),
     m_bufferViewer(windowFrameworkPtr),
     m_separation(1),
     m_filters(),
     m_titleNp(),
     m_inst1Np(),
     m_inst2Np(),
     m_inst3Np(),
     m_character()
   {
   // Note: no need to disable the mouse in C++
   // base.disableMouse()
   m_windowFrameworkPtr->get_camera(0)->get_lens()->set_near(10.0);
   m_windowFrameworkPtr->get_camera(0)->get_lens()->set_far(200.0);
   m_windowFrameworkPtr->get_camera_group().set_pos(0, -50, 0);

   // Check video card capabilities.

   if(!m_windowFrameworkPtr->get_graphics_window()->get_gsg()->
      get_supports_basic_shaders())
      {
      add_title(
         "Toon Shader: Video driver reports that shaders are not supported.");
      return;
      }

   // Enable a 'light ramp' - this discretizes the lighting,
   // which is half of what makes a model look like a cartoon.
   // Light ramps only work if shader generation is enabled,
   // so we call 'setShaderAuto'.

   NodePath tempnode = NodePath(new PandaNode("temp node"));
   tempnode.set_attrib(LightRampAttrib::make_single_threshold(0.5, 0.4));
   tempnode.set_shader_auto();
   m_windowFrameworkPtr->get_camera(0)->set_initial_state(tempnode.get_state());

   m_filters.reset(new CCommonFilters(windowFrameworkPtr->get_graphics_output(),
      NodePath(windowFrameworkPtr->get_camera(0))));

   // Use class 'CommonFilters' to enable a cartoon inking filter.
   // This can fail if the video card is not powerful enough, if so,
   // display an error and exit.

   bool filterok = m_filters->set_cartoon_ink(m_separation);
   if(!filterok)
      {
      add_title("Toon Shader: "
         "Video card not powerful enough to do image postprocessing");
      return;
      }

   // Post the instructions.

   m_titleNp = add_title(
      "Panda3D: Tutorial - Toon Shading with Normals-Based Inking");
   m_inst1Np = add_instructions(0.95, "ESC: Quit");
   m_inst2Np = add_instructions(0.90,
      "Up/Down: Increase/Decrease Line Thickness");
   m_inst3Np = add_instructions(0.85, "V: View the render-to-texture results");

   // Load a dragon model and animate it.

   CActor::AnimMap animMap;
   animMap["../models/nik-dragon"].push_back("win");
   m_character.load_actor(m_windowFrameworkPtr,
                          "../models/nik-dragon",
                          &animMap,
                          PartGroup::HMF_ok_anim_extra      |
                          PartGroup::HMF_ok_part_extra      |
                          PartGroup::HMF_ok_wrong_root_name);
   NodePath render = m_windowFrameworkPtr->get_render();
   m_character.reparent_to(render);
   m_character.loop("win", true);

   PT(CLerpNodePathInterval) hprIntervalPtr =
      new CLerpNodePathInterval("hprInterval",
                                15,
                                CLerpInterval::BT_no_blend,
                                true,
                                false,
                                m_character,
                                NodePath());
   if(hprIntervalPtr != NULL)
      {
      hprIntervalPtr->set_start_hpr(LVecBase3f(0  , 0, 0));
      hprIntervalPtr->set_end_hpr  (LVecBase3f(360, 0, 0));
      hprIntervalPtr->loop();
      }

   AsyncTaskManager::get_global_ptr()->add(
      new GenericAsyncTask("stepIntervalManagerTask",
                           step_interval_manager,
                           NULL));

   // Create a non-attenuating point light and an ambient light.

   PT(PointLight) plightnode = new PointLight("point light");
   plightnode->set_attenuation(LVecBase3f(1, 0, 0));
   NodePath plight = render.attach_new_node(plightnode);
   plight.set_pos(30, -50, 0);
   PT(AmbientLight) alightnode = new AmbientLight("ambient light");
   alightnode->set_color(Colorf(0.8, 0.8, 0.8, 1));
   NodePath alight = render.attach_new_node(alightnode);
   render.set_light(alight);
   render.set_light(plight);

   // Panda contains a built-in viewer that lets you view the
   // results of all render-to-texture operations.  This lets you
   // see what class CommonFilters is doing behind the scenes.

   m_windowFrameworkPtr->enable_keyboard();
   m_windowFrameworkPtr->get_panda_framework()->define_key("v",
      "toggleBufferViewerEnable", call_buffer_viewer_toggle_enable, this);
   m_bufferViewer.set_position(CBufferViewer::CP_llcorner);
   m_windowFrameworkPtr->get_panda_framework()->define_key("s",
      "filtersManagerResizeBuffers", call_filters_manager_resize_buffers, this);

   // These allow you to change cartooning parameters in realtime

   m_windowFrameworkPtr->get_panda_framework()->
      define_key("escape", "sysExit", sys_exit, NULL);
   m_windowFrameworkPtr->get_panda_framework()->
      define_key("arrow_up", "increaseSeparation", call_increase_separation,
         this);
   m_windowFrameworkPtr->get_panda_framework()->
      define_key("arrow_down", "decreaseSeparation", call_decrease_separation,
         this);
   }

void ToonMakerBasic::increase_separation()
   {
   m_separation *= 1.11111111;
   nout << m_separation << endl;
   m_filters->set_cartoon_ink(m_separation);
   }

void ToonMakerBasic::decrease_separation()
   {
   m_separation *= 0.90000000;
   nout << m_separation << endl;
   m_filters->set_cartoon_ink(m_separation);
   }

AsyncTask::DoneStatus ToonMakerBasic::
step_interval_manager(GenericAsyncTask* taskPtr, void* dataPtr)
   {
   CIntervalManager::get_global_ptr()->step();
   return AsyncTask::DS_cont;
   }

void ToonMakerBasic::sys_exit(const Event* eventPtr, void* dataPtr)
   {
   exit(0);
   }

void ToonMakerBasic::call_increase_separation(const Event* eventPtr,
                                              void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<ToonMakerBasic*>(dataPtr)->increase_separation();
   }

void ToonMakerBasic::call_decrease_separation(const Event* eventPtr,
                                              void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<ToonMakerBasic*>(dataPtr)->decrease_separation();
   }

void ToonMakerBasic::call_buffer_viewer_toggle_enable(const Event* eventPtr,
                                                      void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<ToonMakerBasic*>(dataPtr)->m_bufferViewer.toggle_enable();
   }

void ToonMakerBasic::call_filters_manager_resize_buffers(const Event* eventPtr,
                                                         void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<ToonMakerBasic*>(dataPtr)->m_filters->m_manager.resize_buffers();
   }
