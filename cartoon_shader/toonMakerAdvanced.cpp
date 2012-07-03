/*
 * toonMakerAdvanced.cpp
 *
 *  Created on: 2012-06-01
 *      Author: dri
 */

#include "pandaFramework.h"
#include "shaderPool.h"
#include "cLerpNodePathInterval.h"
#include "cIntervalManager.h"
#include "toonMakerAdvanced.h"

// Function to put instructions on the screen.
NodePath ToonMakerAdvanced::add_instructions(float pos, const string& msg)
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
NodePath ToonMakerAdvanced::add_title(const string& text)
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

ToonMakerAdvanced::ToonMakerAdvanced(WindowFramework* windowFrameworkPtr)
   : m_windowFrameworkPtr(windowFrameworkPtr),
     m_bufferViewer(windowFrameworkPtr)
   {
   // precondition
   if(windowFrameworkPtr == NULL)
      {
      nout << "ERROR: parameter windowFrameworkPtr cannot be NULL.";
      return;
      }

   // base.disableMouse() no need to disable the mouse in C++
   m_windowFrameworkPtr->get_camera_group().set_pos(0, -50, 0);

   // Check video card capabilities.

   if(!m_windowFrameworkPtr->get_graphics_window()->get_gsg()->get_supports_basic_shaders())
      {
      add_title("Toon Shader: Video driver reports that shaders are not supported.");
      return;
      }

   // Post the instructions.
   m_titleNp = add_title("Panda3D: Tutorial - Toon Shading with Normals-Based Inking");
   m_inst1Np = add_instructions(0.95, "ESC: Quit");
   m_inst2Np = add_instructions(0.90, "Up/Down: Increase/Decrease Line Thickness");
   m_inst3Np = add_instructions(0.85, "Left/Right: Decrease/Increase Line Darkness");
   m_inst4Np = add_instructions(0.80, "V: View the render-to-texture results");

   // This shader's job is to render the model with discrete lighting
   // levels.  The lighting calculations built into the shader assume
   // a single nonattenuating point light.

   NodePath tempnodeNp = NodePath("temp node");
   tempnodeNp.set_shader(ShaderPool::load_shader("../lightingGen.sha"));
   m_windowFrameworkPtr->get_camera(0)->set_initial_state(tempnodeNp.get_state());

   // This is the object that represents the single "light", as far
   // the shader is concerned.  It's not a real Panda3D LightNode, but
   // the shader doesn't care about that.

   NodePath renderNp = m_windowFrameworkPtr->get_render();
   NodePath lightNp = renderNp.attach_new_node("light");
   lightNp.set_pos(30, -50, 0);

   // this call puts the light's nodepath into the render state.
   // this enables the shader to access this light by name.

   renderNp.set_shader_input("light", lightNp);

   // The "normals buffer" will contain a picture of the model colorized
   // so that the color of the model is a representation of the model's
   // normal at that point.

   PT(GraphicsOutput) normalsBufferPtr = m_windowFrameworkPtr->get_graphics_window()->make_texture_buffer("normalsBuffer", 0, 0);
   normalsBufferPtr->set_clear_color(Colorf(0.5, 0.5, 0.5, 1));
   m_normalsBufferPtr = normalsBufferPtr;
   NodePath normalsCameraNp = m_windowFrameworkPtr->make_camera();
   // Note: the new camera needs to use the lens of the main window camera
   PT(Lens) lensPtr = m_windowFrameworkPtr->get_camera(0)->get_lens();
   DCAST(Camera, normalsCameraNp.node())->set_lens(lensPtr);
   DCAST(Camera, normalsCameraNp.node())->set_scene(renderNp);
   // Note: the new camera needs to be associated to the texture buffer through a display region
   PT(DisplayRegion) m_displayRegionPtr = m_normalsBufferPtr->make_display_region(0, 1, 0, 1);
   m_displayRegionPtr->set_sort(10000);
   m_displayRegionPtr->set_camera(normalsCameraNp);
   tempnodeNp = NodePath("temp node");
   tempnodeNp.set_shader(ShaderPool::load_shader("../normalGen.sha"));
   static_cast<Camera*>(normalsCameraNp.node())->set_initial_state(tempnodeNp.get_state());

   // what we actually do to put edges on screen is apply them as a texture to
   // a transparent screen-fitted card

   NodePath drawnSceneNp = normalsBufferPtr->get_texture_card();
   drawnSceneNp.set_transparency(TransparencyAttrib::M_alpha);
   drawnSceneNp.set_color(1, 1, 1, 0);
   NodePath render2dNp = m_windowFrameworkPtr->get_render_2d();
   drawnSceneNp.reparent_to(render2dNp);
   m_drawnSceneNp = drawnSceneNp;

   // this shader accepts, as input, the picture from the normals buffer.
   // it compares each adjacent pixel, looking for discontinuities.
   // wherever a discontinuity exists, it emits black ink.

   m_separation = 0.001;
   m_cutoff = 0.3;
   CPT(Shader) inkGenPtr = ShaderPool::load_shader("../inkGen.sha");
   drawnSceneNp.set_shader(inkGenPtr);
   drawnSceneNp.set_shader_input("separation", LVector4f(m_separation, 0, m_separation, 0));
   drawnSceneNp.set_shader_input("cutoff", LVector4f(m_cutoff, m_cutoff, m_cutoff, m_cutoff));

   // Panda contains a built-in viewer that lets you view the results of
   // your render-to-texture operations.  This code configures the viewer.
   m_windowFrameworkPtr->enable_keyboard();
   m_windowFrameworkPtr->get_panda_framework()->define_key("v", "toggleBufferViewerEnable", call_buffer_viewer_toggle_enable, this);
   m_bufferViewer.set_position(CBufferViewer::CP_llcorner);

   // Load a dragon model and animate it.

   CActor::AnimMap animMap;
   animMap["../models/nik-dragon"].push_back("win");
   m_character.load_actor(m_windowFrameworkPtr,
                          "../models/nik-dragon",
                          &animMap,
                          PartGroup::HMF_ok_anim_extra      |
                          PartGroup::HMF_ok_part_extra      |
                          PartGroup::HMF_ok_wrong_root_name);
   m_character.reparent_to(renderNp);
   m_character.loop("win", true);

   PT(CLerpNodePathInterval) hprIntervalPtr = new CLerpNodePathInterval("hprInterval",
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

   AsyncTaskManager::get_global_ptr()->add(new GenericAsyncTask("stepIntervalManagerTask",
                                                                step_interval_manager,
                                                                NULL));

   // these allow you to change cartooning parameters in realtime

   m_windowFrameworkPtr->get_panda_framework()->define_key("escape"     , "sysExit"           , sys_exit                , NULL);
   m_windowFrameworkPtr->get_panda_framework()->define_key("arrow_up"   , "increaseSeparation", call_increase_separation, this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("arrow_down" , "decreaseSeparation", call_decrease_separation, this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("arrow_left" , "increaseCutoff"    , call_increase_cutoff    , this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("arrow_right", "decreaseCutoff"    , call_decrease_cutoff    , this);
   }

void ToonMakerAdvanced::increase_separation()
   {
   m_separation *= 1.11111111;
   nout << m_separation << endl;
   m_drawnSceneNp.set_shader_input("separation", LVector4f(m_separation, 0, m_separation, 0));
   }

void ToonMakerAdvanced::decrease_separation()
   {
   m_separation *= 0.90000000;
   nout << m_separation << endl;
   m_drawnSceneNp.set_shader_input("separation", LVector4f(m_separation, 0, m_separation, 0));
   }

void ToonMakerAdvanced::increase_cutoff()
   {
   m_cutoff *= 1.11111111;
   nout << m_cutoff << endl;
   m_drawnSceneNp.set_shader_input("cutoff", LVector4f(m_cutoff, m_cutoff, m_cutoff, m_cutoff));
   }

void ToonMakerAdvanced::decrease_cutoff()
   {
   m_cutoff *= 0.90000000;
   nout << m_cutoff << endl;
   m_drawnSceneNp.set_shader_input("cutoff", LVector4f(m_cutoff, m_cutoff, m_cutoff, m_cutoff));
   }

AsyncTask::DoneStatus ToonMakerAdvanced::step_interval_manager(GenericAsyncTask* taskPtr, void* dataPtr)
   {
   CIntervalManager::get_global_ptr()->step();
   return AsyncTask::DS_cont;
   }

void ToonMakerAdvanced::sys_exit(const Event* eventPtr, void* dataPtr)
   {
   exit(0);
   }

void ToonMakerAdvanced::call_buffer_viewer_toggle_enable(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<ToonMakerAdvanced*>(dataPtr)->m_bufferViewer.toggle_enable();
   }

void ToonMakerAdvanced::call_increase_separation(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<ToonMakerAdvanced*>(dataPtr)->increase_separation();
   }

void ToonMakerAdvanced::call_decrease_separation(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<ToonMakerAdvanced*>(dataPtr)->decrease_separation();
   }

void ToonMakerAdvanced::call_increase_cutoff(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<ToonMakerAdvanced*>(dataPtr)->increase_cutoff();
   }

void ToonMakerAdvanced::call_decrease_cutoff(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<ToonMakerAdvanced*>(dataPtr)->decrease_cutoff();
   }
