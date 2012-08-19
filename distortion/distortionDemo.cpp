/*
 * distortionDemo.cpp
 *
 *  Created on: 2012-08-17
 *      Author: dri
 */

#include "../p3util/cOnscreenText.h"
#include "../p3util/callbackUtil.h"
#include "texturePool.h"
#include "cIntervalManager.h"
#include "shaderPool.h"
#include "distortionDemo.h"

// Note: These macros lighten the call to template functions
#define DEMO_DEFINE_KEY(event_name, description, func)  \
   define_key_t<DistortionDemo, &DistortionDemo::func>( \
      m_windowFramework,                                \
      event_name,                                       \
      description,                                      \
      this)

#define DEMO_ADD_TASK(name, func) \
   add_task_t<DistortionDemo, &DistortionDemo::func>(name, this)

// Function to put instructions on the screen.
NodePath DistortionDemo::add_instructions(float pos, const string& msg) const
   {
   COnscreenText instruction("instruction");
   instruction.set_text(msg);
   instruction.set_fg(Colorf(1, 1, 1, 1));
   instruction.set_pos(LVecBase2f(-1.3, pos));
   instruction.set_align(TextNode::A_left);
   instruction.set_scale(0.05);
   instruction.reparent_to(m_windowFramework->get_aspect_2d());
   return instruction.generate();
   }


// Function to put title on the screen.
NodePath add_title(WindowFramework* windowFramework, const string& text)
   {
   COnscreenText title("title");
   title.set_text(text);
   title.set_fg(Colorf(1, 1, 1, 1));
   title.set_pos(LVecBase2f(1.28, -0.95));
   title.set_align(TextNode::A_right);
   title.set_scale(0.07);
   title.reparent_to(windowFramework->get_aspect_2d());
   // Note: set the draw order to make sure the the BufferViewer
   //       renders in front of the title just like in the
   //       Python original version.
   title.set_draw_order(0);
   return title.generate();
   }

DistortionDemo::DistortionDemo(WindowFramework* windowFramework)
   : m_windowFramework(windowFramework),
     m_title(),
     m_inst1(),
     m_inst2(),
     m_inst4(),
     m_seascape(),
     m_distortionBuffer(),
     m_distortionObject(),
     m_hprInterval(),
     m_texDistortion(new Texture()),
     m_bufferViewer(windowFramework),
     m_distortionOn(true)
   {
   // base.disableMouse() // no for this in C++
   m_windowFramework->get_graphics_output()->get_active_display_region(0)->
      set_clear_color(Colorf(0, 0, 0, 1));
   DEMO_ADD_TASK("updateScene", update_scene);

   // Show the instructions
   m_title = add_title(m_windowFramework,
      "Panda3D: Tutorial - Distortion Effect");
   m_inst1 = add_instructions(0.95, "ESC: Quit");
   m_inst2 = add_instructions(0.90, "Space: Toggle distortion filter On/Off");
   m_inst4 = add_instructions(0.85, "V: View the render-to-texture results");

   // Load background
   const NodePath& models =
      m_windowFramework->get_panda_framework()->get_models();
   m_seascape = m_windowFramework->load_model(models, "../models/plane");
   const NodePath& render = m_windowFramework->get_render();
   m_seascape.reparent_to(render);
   m_seascape.set_pos_hpr(0, 145, 0, 0, 0, 0);
   m_seascape.set_scale(100);
   m_seascape.set_texture(TexturePool::load_texture("../models/ocean.jpg"));

   // Create the distortion buffer. This buffer renders like a normal scene,
   m_distortionBuffer = make_FBO("model buffer");
   m_distortionBuffer->set_sort(-3);
   m_distortionBuffer->set_clear_color(Colorf(0, 0, 0, 0));

   // We have to attach a camera to the distortion buffer. The distortion camera
   // must have the same frustum as the main camera. As long as the aspect
   // ratios match, the rest will take care of itself.
   // Note: make a new camera
   NodePath distortionCamera = m_windowFramework->make_camera();
   // Note: set the scene the camera has to film
   DCAST(Camera, distortionCamera.node())->set_scene(render);
   // Note: set the camera mask, it is used to show/hide the distortion object
   //       that will share the same mask
   DCAST(Camera, distortionCamera.node())->set_camera_mask(BitMask32::bit(4));
   // Note: tell the distortion buffer to make a display region for the camera
   //       to display into
   PT(DisplayRegion) dr = m_distortionBuffer->make_display_region();
   // Note: connect the camera to its new display region
   dr->set_camera(distortionCamera);

   // load the object with the distortion
   m_distortionObject = m_windowFramework->load_model(models, "../models/boat");
   m_distortionObject.set_scale(1);
   m_distortionObject.set_pos(0, 20, -3);
   const bool bakeInStart = true;
   const bool fluid = false;
   m_hprInterval = new CLerpNodePathInterval("hprInterval", 10,
      CLerpInterval::BT_no_blend, bakeInStart, fluid, m_distortionObject,
      NodePath());
   m_hprInterval->set_start_hpr(LVecBase3f(0, 0, 0));
   m_hprInterval->set_end_hpr(LVecBase3f(360, 0, 0));
   m_hprInterval->loop();
   m_distortionObject.reparent_to(render);

   // Create the shader that will determine what parts of the scene will
   // distortion
   CPT(Shader) distortionShader = ShaderPool::load_shader("distortion.sha");
   m_distortionObject.set_shader(distortionShader);
   // Note: set the hiding mask of the distortion object so that show/hide
   //       commands will have effects only on the distortion camera
   m_distortionObject.hide(BitMask32::bit(4));

   // Textures
   PT(Texture) tex1 = TexturePool::load_texture("../models/water.png");
   m_distortionObject.set_shader_input("waves", tex1);

   m_distortionBuffer->add_render_texture(m_texDistortion,
      GraphicsOutput::RTM_bind_or_copy, GraphicsOutput::RTP_color);
   m_distortionObject.set_shader_input("screen", m_texDistortion);

   // Panda contains a built-in viewer that lets you view the results of
   // your render-to-texture operations.  This code configures the viewer.
   DEMO_DEFINE_KEY("v", "toggleBufferViewer", toggle_buffer_viewer);
   m_bufferViewer.set_position(CBufferViewer::CP_llcorner);
   m_bufferViewer.set_layout(CBufferViewer::CL_hline);
   m_bufferViewer.set_card_size(0.652, 0);

   // event handling
   DEMO_DEFINE_KEY("space", "toggleDistortion", toggle_distortion);
   DEMO_DEFINE_KEY("escape", "exit", quit);
   m_distortionOn = true;
   }

GraphicsOutput* DistortionDemo::make_FBO(const string& name) const
   {
   // This routine creates an offscreen buffer.  All the complicated
   // parameters are basically demanding capabilities from the offscreen
   // buffer - we demand that it be able to render to texture on every
   // bitplane, that it can support aux bitplanes, that it track
   // the size of the host window, that it can render to texture
   // cumulatively, and so forth.
   WindowProperties winprops;
   FrameBufferProperties props;
   props.set_rgb_color(1);
   PT(GraphicsOutput) baseWin = m_windowFramework->get_graphics_output();
   return baseWin->get_engine()->make_output(baseWin->get_pipe(), name, -2,
      props, winprops, GraphicsPipe::BF_size_track_host |
      GraphicsPipe::BF_refuse_window, baseWin->get_gsg(), baseWin);
   }

void DistortionDemo::toggle_distortion(const Event* event)
   {
   // Toggles the distortion on/off.
   if(m_distortionOn)
      {
      m_distortionObject.hide();
      }
   else
      {
      m_distortionObject.show();
      }
   m_distortionOn = !m_distortionOn;
   }

AsyncTask::DoneStatus DistortionDemo::update_scene(GenericAsyncTask *task)
   {
   // This is a task that keeps passing the time to the shader.
   NodePath render = m_windowFramework->get_render();
   render.set_shader_input("time",
      ClockObject::get_global_clock()->get_frame_time());

   // Step the interval manager to execute intervals
   CIntervalManager::get_global_ptr()->step();

   return AsyncTask::DS_cont;
   }

void DistortionDemo::toggle_buffer_viewer(const Event* event)
   {
   m_bufferViewer.toggle_enable();
   }


void DistortionDemo::quit(const Event* event)
   {
   m_windowFramework->get_panda_framework()->set_exit_flag();
   }
