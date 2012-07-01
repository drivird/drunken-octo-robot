/*
 * glowDemoBasic.cpp
 *
 *  Created on: 2012-06-29
 *      Author: dri
 */

#include "cIntervalManager.h"
#include "directionalLight.h"
#include "ambientLight.h"
#include "glowDemoBasic.h"

// Function to put instructions on the screen.
NodePath GlowDemoBasic::add_instructions(float pos, const string& msg)
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
NodePath GlowDemoBasic::add_title(const string& text)
   {
   COnscreenText title("title");
   title.set_text(text);
   title.set_fg(Colorf(1, 1, 1, 1));
   title.set_pos(LVecBase2f(1.28, -0.95));
   title.set_align(TextNode::A_right);
   title.set_scale(0.07);
   title.reparent_to(m_windowFrameworkPtr->get_aspect_2d());
   // Note: set the draw order to make sure the the BufferViewer
   //       renders in front of the title just like in the
   //       Python original version.
   title.set_draw_order(0);
   return title.generate();
   }

GlowDemoBasic::GlowDemoBasic(WindowFramework* windowFrameworkPtr)
   : m_windowFrameworkPtr(windowFrameworkPtr),
     m_filters(),
     m_glowSize(1),
     m_titleNp(),
     m_inst1Np(),
     m_inst2Np(),
     m_inst3Np(),
     m_inst4Np(),
     m_tron(),
     m_isRunning(false),
     m_hprIntervalPtr(NULL),
     m_bufferViewer(windowFrameworkPtr)
   {
   // Note: no need to disable the mouse in C++
   // base.disableMouse()
   m_windowFrameworkPtr->get_graphics_window()->get_active_display_region(0)->
      set_clear_color(Colorf(0,0,0,1));
   m_windowFrameworkPtr->get_camera_group().set_pos(0, -50, 0);

   // Check video card capabilities.

   if(!m_windowFrameworkPtr->get_graphics_window()->get_gsg()->
      get_supports_basic_shaders())
      {
      add_title(
         "Glow Filter: Video driver reports that shaders are not supported.");
      return;
      }

   // Use class 'CommonFilters' to enable a bloom filter.
   // The brightness of a pixel is measured using a weighted average
   // of R,G,B,A.  We put all the weight on Alpha, meaning that for
   // us, the framebuffer's alpha channel alpha controls bloom.

   m_filters.reset(new CCommonFilters(windowFrameworkPtr->get_graphics_output(),
      NodePath(windowFrameworkPtr->get_camera(0))));
   CCommonFilters::SetBloomParameters params;
   params.blend = LVecBase4f(0, 0, 0, 1);
   params.desat = -0.5;
   params.intensity = 3.0;
   params.size = "small";
   bool filterok = m_filters->set_bloom(params);
   if(!filterok)
      {
      add_title("Glow Filter: "
         "Video card not powerful enough to do image postprocessing");
      return;
      }

   // Post the instructions.

   m_titleNp = add_title("Panda3D: Tutorial - Glow Filter");
   m_inst1Np = add_instructions(0.95, "ESC: Quit");
   m_inst2Np = add_instructions(0.90,
      "Space: Toggle Glow Filter Small/Med/Large/Off");
   m_inst3Np = add_instructions(0.85, "Enter: Toggle Running/Spinning");
   m_inst4Np = add_instructions(0.80, "V: View the render-to-texture results");

   // Load our model

   CActor::AnimMap animMap;
   animMap["../models/tron_anim"].push_back("running");
   m_tron.load_actor(m_windowFrameworkPtr,
                     "../models/tron",
                     &animMap,
                     PartGroup::HMF_ok_anim_extra      |
                     PartGroup::HMF_ok_part_extra      |
                     PartGroup::HMF_ok_wrong_root_name);
   NodePath render = m_windowFrameworkPtr->get_render();
   m_tron.reparent_to(render);

   m_hprIntervalPtr = new CLerpNodePathInterval("hprInterval",
                                                60,
                                                CLerpInterval::BT_no_blend,
                                                true,
                                                false,
                                                m_tron,
                                                NodePath());
   if(m_hprIntervalPtr != NULL)
      {
      m_hprIntervalPtr->set_start_hpr(LVecBase3f(0  , 0, 0));
      m_hprIntervalPtr->set_end_hpr  (LVecBase3f(360, 0, 0));
      m_hprIntervalPtr->loop();
      }

   AsyncTaskManager::get_global_ptr()->add(
      new GenericAsyncTask("stepIntervalManagerTask",
                           step_interval_manager,
                           NULL));

   // put some lighting on the model

   PT(DirectionalLight) dlight = new DirectionalLight("dlight");
   PT(AmbientLight) alight = new AmbientLight("alight");
   NodePath dlnp = render.attach_new_node(dlight);
   NodePath alnp = render.attach_new_node(alight);
   dlight->set_color(Colorf(1.0, 0.7, 0.2, 1));
   alight->set_color(Colorf(0.2, 0.2, 0.2, 1));
   dlnp.set_hpr(0, -60, 0);
   render.set_light(dlnp);
   render.set_light(alnp);

   // Panda contains a built-in viewer that lets you view the results of
   // your render-to-texture operations.  This code configures the viewer.

   m_windowFrameworkPtr->enable_keyboard();
   m_windowFrameworkPtr->get_panda_framework()->define_key("v",
      "toggleBufferViewerEnable", call_buffer_viewer_toggle_enable, this);
   m_bufferViewer.set_position(CBufferViewer::CP_llcorner);
   m_bufferViewer.set_layout(CBufferViewer::CL_hline);
   //base.camLens.setFov(100)
   // event handling
   m_windowFrameworkPtr->get_panda_framework()->
      define_key("space", "toggleGlow", call_toggle_glow, this);
   m_windowFrameworkPtr->get_panda_framework()->
      define_key("enter", "toggleDisplay", call_toggle_display,
         this);
   m_windowFrameworkPtr->get_panda_framework()->
      define_key("escape", "sysExit", sys_exit, NULL);
   }

void GlowDemoBasic::toggle_glow()
   {
   ++m_glowSize;
   if(m_glowSize == 4) { m_glowSize = 0; }
   CCommonFilters::SetBloomParameters params;
   params.blend = LVecBase4f(0, 0, 0, 1);
   params.desat = -0.5;
   params.intensity = 3.0;
   switch(m_glowSize)
      {
      case 0: params.size = "off"   ; break;
      case 1: params.size = "small" ; break;
      case 2: params.size = "medium"; break;
      case 3: params.size = "large" ; break;
      default:
         nout << "ERROR: forgot a glow size case." << endl;
         return;
      }
   m_filters->set_bloom(params);
   }

void GlowDemoBasic::toggle_display()
   {
   m_isRunning = !m_isRunning;
   if(!m_isRunning)
      {
      m_windowFrameworkPtr->get_camera_group().set_pos(0, -50, 0);
      m_tron.stop("running");
      m_tron.pose("running", 0);
      m_hprIntervalPtr->loop();
      }
   else
      {
      m_windowFrameworkPtr->get_camera_group().set_pos(0, -170, 3);
      m_hprIntervalPtr->finish();
      m_tron.set_hpr(0, 0, 0);
      m_tron.loop("running", true);
      }
   }

AsyncTask::DoneStatus GlowDemoBasic::
step_interval_manager(GenericAsyncTask* taskPtr, void* dataPtr)
   {
   CIntervalManager::get_global_ptr()->step();
   return AsyncTask::DS_cont;
   }

void GlowDemoBasic::call_buffer_viewer_toggle_enable(const Event* eventPtr,
                                                      void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<GlowDemoBasic*>(dataPtr)->m_bufferViewer.toggle_enable();
   }

void GlowDemoBasic::call_toggle_glow(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<GlowDemoBasic*>(dataPtr)->toggle_glow();
   }

void GlowDemoBasic::call_toggle_display(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<GlowDemoBasic*>(dataPtr)->toggle_display();
   }

void GlowDemoBasic::sys_exit(const Event* eventPtr, void* dataPtr)
   {
   exit(0);
   }
