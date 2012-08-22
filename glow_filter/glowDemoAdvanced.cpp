/*
 * glowDemoAdvanced.cpp
 *
 *  Created on: 2012-07-01
 *      Author: dri
 */

#include "glowDemoAdvanced.h"
#include "colorBlendAttrib.h"
#include "shaderPool.h"
#include "cIntervalManager.h"
#include "directionalLight.h"
#include "ambientLight.h"
#include "glowDemoAdvanced.h"

// Function to put instructions on the screen.
NodePath GlowDemoAdvanced::add_instructions(float pos, const string& msg)
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
NodePath GlowDemoAdvanced::add_title(const string& text)
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

// This function is responsible for setting up the two blur filters.
// It just makes a temp Buffer, puts a screen aligned card, and then sets
// the appropiate shader to do all the work. Gaussian blurs are decomposable
// into a two-pass algorithm which is faster than the equivalent one-pass
// algorithm, so we do it in two passes. The full explanation (for math buffs)
// can be found in the article above
GraphicsOutput* GlowDemoAdvanced::make_filter_buffer(GraphicsOutput* srcbuffer,
                                                     const string& name,
                                                     int sort,
                                                     const string& prog)
   {
   PT(GraphicsOutput) blurBuffer = m_windowFrameworkPtr->get_graphics_window()->
      make_texture_buffer(name, 512, 512);
   blurBuffer->set_sort(sort);
   blurBuffer->set_clear_color(Colorf(1, 0, 0, 1));
   // Note: makeCamera2d is python function replicated here.
   NodePath blurCamera = make_camera_2d(MakeCamera2dParameters(blurBuffer));
   NodePath blurScene = NodePath("new Scene");
   DCAST(Camera, blurCamera.node())->set_scene(blurScene);
   CPT(Shader) shader = ShaderPool::load_shader(prog);
   NodePath card = srcbuffer->get_texture_card();
   card.reparent_to(blurScene);
   card.set_shader(shader);
   return blurBuffer;
   }

// Note: see makeCamera2d in direct/src/showbase/ShowBase.py
// Makes a new camera2d associated with the indicated window, and
// assigns it to render the indicated subrectangle of render2d.
GlowDemoAdvanced::MakeCamera2dParameters::
MakeCamera2dParameters(GraphicsOutput* win)
   : win(win),
     sort(10),
     displayRegion(0, 1, 0, 1),
     coords(-1, 1, -1, 1),
     lens(NULL),
     cameraName()
   {
   // Empty
   }
NodePath GlowDemoAdvanced::make_camera_2d(const MakeCamera2dParameters& params)
   {
   PT(GraphicsOutput) win = params.win;
   int sort = params.sort;
   const LVecBase4f& displayRegion = params.displayRegion;
   const LVecBase4f& coords = params.coords;
   PT(OrthographicLens) lens = params.lens;
   const string& cameraName = params.cameraName;

   PT(DisplayRegion) dr = win->
      make_mono_display_region(displayRegion[0],
                               displayRegion[1],
                               displayRegion[2],
                               displayRegion[3]);
   dr->set_sort(sort);

   // Enable clearing of the depth buffer on this new display
   // region (see the comment in setupRender2d, above).
   dr->set_clear_depth_active(true);

   // Make any texture reloads on the gui come up immediately.
   dr->set_incomplete_render(false);

   float left = coords[0];
   float right = coords[1];
   float bottom = coords[2];
   float top = coords[3];

   // Now make a new Camera node.
   PT(Camera) cam2dNode = NULL;
   if(cameraName.size() > 0)
      {
      cam2dNode = new Camera("cam2d_" + cameraName);
      }
   else
      {
      cam2dNode = new Camera("cam2d");
      }

   if(lens == NULL)
      {
      lens = new OrthographicLens();
      lens->set_film_size(right - left, top - bottom);
      lens->set_film_offset((right + left) * 0.5, (top + bottom) * 0.5);
      lens->set_near_far(-1000, 1000);
      }
   cam2dNode->set_lens(lens);

   // self.camera2d is the analog of self.camera, although it's
   // not as clear how useful it is.
   if(m_camera2d.is_empty())
      {
      m_camera2d = m_windowFrameworkPtr->get_render_2d().
         attach_new_node("camera2d");
      }

   NodePath camera2d = m_camera2d.attach_new_node(cam2dNode);
   dr->set_camera(camera2d);

   // if self.cam2d == None:
   // self.cam2d = camera2d

   return camera2d;
   }

GlowDemoAdvanced::GlowDemoAdvanced(WindowFramework* windowFrameworkPtr)
   : m_windowFrameworkPtr(windowFrameworkPtr),
     m_titleNp(),
     m_inst1Np(),
     m_inst2Np(),
     m_inst3Np(),
     m_inst4Np(),
     m_tron(),
     m_isRunning(false),
     m_finalcard(),
     m_bufferViewer(windowFrameworkPtr),
     m_glowOn(true),
     m_hprIntervalPtr(NULL),
     m_camera2d()
   {
   // Note: no need to disable the mouse in C++
   // base.disableMouse()
   m_windowFrameworkPtr->set_background_type(WindowFramework::BT_black);
   m_windowFrameworkPtr->get_camera_group().set_pos(0, -50, 0);

   // Check video card capabilities.

   if(!m_windowFrameworkPtr->get_graphics_window()->get_gsg()->
      get_supports_basic_shaders())
      {
      add_title(
         "Glow Filter: Video driver reports that shaders are not supported.");
      return;
      }

   // Post the instructions.

   m_titleNp = add_title("Panda3D: Tutorial - Glow Filter");
   m_inst1Np = add_instructions(0.95, "ESC: Quit");
   m_inst2Np = add_instructions(0.90, "Space: Toggle Glow Filter On/Off");
   m_inst3Np = add_instructions(0.85, "Enter: Toggle Running/Spinning");
   m_inst4Np = add_instructions(0.80, "V: View the render-to-texture results");

   // create the shader that will determime what parts of the scene will glow
   CPT(Shader) glowShader = ShaderPool::load_shader("../glowShader.sha");

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

   // create the glow buffer. This buffer renders like a normal scene,
   // except that only the glowing materials should show up nonblack.
   PT(GraphicsOutput) glowBuffer = m_windowFrameworkPtr->get_graphics_window()->
         make_texture_buffer("Glow scene", 512, 512);
   glowBuffer->set_sort(-3);
   glowBuffer->set_clear_color(Colorf(0, 0, 0, 1));

   // We have to attach a camera to the glow buffer. The glow camera
   // must have the same frustum as the main camera. As long as the aspect
   // ratios match, the rest will take care of itself.
   // Node: see how makeCamera works in direct/src/showbase/ShowBase.py
   PT(Camera) camNode = new Camera("cam");
   camNode->set_lens(m_windowFrameworkPtr->get_camera(0)->get_lens(0));
   NodePath glowCamera = m_windowFrameworkPtr->
      get_camera_group().attach_new_node(camNode);
   glowBuffer->make_display_region()->set_camera(glowCamera);

   // Tell the glow camera to use the glow shader
   NodePath tempnode = NodePath("temp node");
   tempnode.set_shader(glowShader);
   DCAST(Camera, glowCamera.node())->set_initial_state(tempnode.get_state());

   // set up the pipeline: from glow scene to blur x to blur y to main window.
   PT(GraphicsOutput) blurXBuffer = make_filter_buffer(glowBuffer,
                                                       "Blur X",
                                                       -2,
                                                       "../XBlurShader.sha");
   PT(GraphicsOutput) blurYBuffer = make_filter_buffer(blurXBuffer,
                                                       "Blur Y",
                                                       -1,
                                                       "../YBlurShader.sha");
   m_finalcard = blurYBuffer->get_texture_card();
   m_finalcard.reparent_to(m_windowFrameworkPtr->get_render_2d());
   m_finalcard.set_attrib(ColorBlendAttrib::make(ColorBlendAttrib::M_add));

   // Panda contains a built-in viewer that lets you view the results of
   // your render-to-texture operations.  This code configures the viewer.

   m_windowFrameworkPtr->enable_keyboard();
   m_windowFrameworkPtr->get_panda_framework()->define_key("v",
      "toggleBufferViewerEnable", call_buffer_viewer_toggle_enable, this);
   m_bufferViewer.set_position(CBufferViewer::CP_llcorner);
   m_bufferViewer.set_layout(CBufferViewer::CL_hline);
   m_bufferViewer.set_card_size(0.652, 0);

   // event handling
   m_windowFrameworkPtr->get_panda_framework()->
      define_key("space", "toggleGlow", call_toggle_glow, this);
   m_windowFrameworkPtr->get_panda_framework()->
      define_key("enter", "toggleDisplay", call_toggle_display,
         this);
   m_windowFrameworkPtr->get_panda_framework()->
      define_key("escape", "sysExit", sys_exit, NULL);
   }

void GlowDemoAdvanced::toggle_glow()
   {
   if(m_glowOn)
      {
      m_finalcard.detach_node();
      }
   else
      {
      m_finalcard.reparent_to(m_windowFrameworkPtr->get_render_2d());
      }
   m_glowOn = !m_glowOn;
   }

void GlowDemoAdvanced::toggle_display()
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

AsyncTask::DoneStatus GlowDemoAdvanced::
step_interval_manager(GenericAsyncTask* taskPtr, void* dataPtr)
   {
   CIntervalManager::get_global_ptr()->step();
   return AsyncTask::DS_cont;
   }

void GlowDemoAdvanced::call_buffer_viewer_toggle_enable(const Event* eventPtr,
                                                      void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<GlowDemoAdvanced*>(dataPtr)->m_bufferViewer.toggle_enable();
   }

void GlowDemoAdvanced::call_toggle_glow(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<GlowDemoAdvanced*>(dataPtr)->toggle_glow();
   }

void GlowDemoAdvanced::call_toggle_display(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<GlowDemoAdvanced*>(dataPtr)->toggle_display();
   }

void GlowDemoAdvanced::sys_exit(const Event* eventPtr, void* dataPtr)
   {
   exit(0);
   }
