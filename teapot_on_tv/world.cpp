/*
 * world.cpp
 *
 *  Created on: 2012-08-07
 *      Author: dri
 */

#include "../p3util/cOnscreenText.h"
#include "../p3util/callbackUtil.h"
#include "cIntervalManager.h"
#include "directionalLight.h"
#include "ambientLight.h"
#include "world.h"

// Note: These macros lighten the call to template functions
#define WORLD_DEFINE_KEY(event_name, description, func) \
   define_key_t<World, &World::func>(m_windowFramework, \
                                     event_name,        \
                                     description,       \
                                     this)

#define WORLD_ADD_TASK(name, func) \
   add_task_t<World, &World::func>(name, this)

// Function to put instructions on the screen.
NodePath World::add_instructions(float pos, const string& msg)
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
NodePath World::add_title(const string& text)
   {
   COnscreenText title("title");
   title.set_text(text);
   title.set_fg(Colorf(1, 1, 1, 1));
   title.set_pos(LVecBase2f(1.3, -0.95));
   title.set_align(TextNode::A_right);
   title.set_scale(0.07);
   title.reparent_to(m_windowFramework->get_aspect_2d());
   // Note: set the draw order to make sure the the BufferViewer
   //       renders in front of the title just like in the
   //       Python original version.
   title.set_draw_order(0);
   return title.generate();
   }

World::World(WindowFramework* windowFramework)
   : m_windowFramework(windowFramework),
     m_title(),
     m_inst1(),
     m_inst2(),
     m_inst3(),
     m_inst4(),
     m_altCam(),
     m_teapot(),
     m_teapotInterval(),
     m_bufferViewer(NULL)
     // m_tvMen
   {
   // Note: set background color here
   m_windowFramework->get_graphics_output()->get_active_display_region(0)->
      set_clear_color(Colorf(0, 0, 0, 1));

   // Post the instructions.
   m_title = add_title("Panda3D: Tutorial - Using Render-to-Texture");
   m_inst1 = add_instructions(0.95,"ESC: Quit");
   m_inst2 = add_instructions(0.90,"Up/Down: Zoom in/out on the Teapot");
   m_inst3 = add_instructions(0.85,"Left/Right: Move teapot left/right");
   m_inst4 = add_instructions(0.80,"V: View the render-to-texture results");

   //we get a handle to the default window
   PT(GraphicsOutput) mainWindow = m_windowFramework->get_graphics_output();

   // we now get buffer thats going to hold the texture of our new scene
   PT(GraphicsOutput) altBuffer = mainWindow->make_texture_buffer(
      "hello", 256, 256);

   // now we have to setup a new scene graph to make this scene
   NodePath altRender("new render");

   // this takes care of setting up the camera properly
   m_altCam = m_windowFramework->make_camera();
   // Note: set the size and shape of the "film" within the lens equal to the
   //       buffer of our new scene
   DCAST(Camera, m_altCam.node())->get_lens()->set_film_size(
      altBuffer->get_x_size(), altBuffer->get_y_size());
   // Note: make a DisplayRegion for the camera
   PT(DisplayRegion) dr = altBuffer->make_display_region(0, 1, 0, 1);
   dr->set_sort(0);
   dr->set_camera(m_altCam);
   m_altCam.reparent_to(altRender);
   m_altCam.set_pos(0, -10, 0);

   // get the teapot and rotates it for a simple animation
   const NodePath& models =
      m_windowFramework->get_panda_framework()->get_models();
   m_teapot = m_windowFramework->load_model(models, "../models/teapot");
   m_teapot.reparent_to(altRender);
   m_teapot.set_pos(0, 0, -1);

   const bool bakeInStart = true;
   const bool fluid = false;
   m_teapotInterval = new CLerpNodePathInterval("teapotInterval", 1.5,
      CLerpInterval::BT_no_blend, bakeInStart, fluid, m_teapot, NodePath());
   m_teapotInterval->set_start_hpr(m_teapot.get_hpr());
   m_teapotInterval->set_end_hpr(LVecBase3f(m_teapot.get_h()+360,
                                            m_teapot.get_p()+360,
                                            m_teapot.get_r()+360));
   m_teapotInterval->loop();

   // put some lighting on the teapot
   PT(DirectionalLight) dlight = new DirectionalLight("dlight");
   PT(AmbientLight) alight = new AmbientLight("alight");
   NodePath dlnp = altRender.attach_new_node(dlight);
   NodePath alnp = altRender.attach_new_node(alight);
   dlight->set_color(Colorf(0.8, 0.8, 0.5, 1));
   alight->set_color(Colorf(0.2, 0.2, 0.2, 1));
   dlnp.set_hpr(0, -60, 0);
   altRender.set_light(dlnp);
   altRender.set_light(alnp);

   // Panda contains a built-in viewer that lets you view the results of
   // your render-to-texture operations.  This code configures the viewer.

   WORLD_DEFINE_KEY("v", "toggleBufferViewer", toggle_buffer_viewer);
   m_bufferViewer = new CBufferViewer(m_windowFramework);
   m_bufferViewer->set_position(CBufferViewer::CP_llcorner);
   m_bufferViewer->set_card_size(1.0, 0.0);

   // Create the tv-men. Each TV-man will display the
   // offscreen-texture on his TV screen.
   make_tv_man(-5, 30,  1, altBuffer->get_texture(), 0.9);
   make_tv_man( 5, 30,  1, altBuffer->get_texture(), 1.4);
   make_tv_man( 0, 23, -3, altBuffer->get_texture(), 2.0);
   make_tv_man(-5, 20, -6, altBuffer->get_texture(), 1.1);
   make_tv_man( 5, 18, -5, altBuffer->get_texture(), 1.7);

   WORLD_DEFINE_KEY("escape", "exit", quit);
   WORLD_DEFINE_KEY("arrow_up", "zoomIn", zoom_in);
   WORLD_DEFINE_KEY("arrow_down", "zoomOut", zoom_out);
   WORLD_DEFINE_KEY("arrow_left", "moveLeft", move_left);
   WORLD_DEFINE_KEY("arrow_right", "moveRight", move_right);

   WORLD_ADD_TASK("worldAsyncTask", async_task);
   }

void World::make_tv_man(float x, float y, float z, Texture* tex, float playrate)
   {
   CActor man;
   CActor::AnimMap manAnim;
   manAnim["../models/mechman_idle"].push_back("mechman_anim");
   man.load_actor(m_windowFramework,
                  "../models/mechman_idle",
                  &manAnim,
                  PartGroup::HMF_ok_part_extra |
                  PartGroup::HMF_ok_anim_extra |
                  PartGroup::HMF_ok_wrong_root_name);
   man.set_pos(x, y, z);
   const NodePath& render = m_windowFramework->get_render();
   man.reparent_to(render);
   NodePath fp = man.find("**/faceplate");
   fp.set_texture(tex, 1);
   man.find_anim("mechman_anim")->set_play_rate(playrate);
   const bool restart = true;
   man.loop("mechman_anim", restart);
   m_tvMen.push_back(man);
   }

void World::zoom_in(const Event* event)
   {
   m_altCam.set_y(m_altCam.get_y() * 0.9);
   }

void World::zoom_out(const Event* event)
   {
   m_altCam.set_y(m_altCam.get_y() * 1.2);
   }

void World::move_left(const Event* event)
   {
   m_altCam.set_x(m_altCam.get_x() + 1);
   }

void World::move_right(const Event* event)
   {
   m_altCam.set_x(m_altCam.get_x() - 1);
   }

AsyncTask::DoneStatus World::async_task(GenericAsyncTask *task)
   {
   CIntervalManager::get_global_ptr()->step();

   return AsyncTask::DS_cont;
   }

void World::toggle_buffer_viewer(const Event* event)
   {
   m_bufferViewer->toggle_enable();
   }

void World::quit(const Event* event)
   {
   m_windowFramework->get_panda_framework()->set_exit_flag();
   }
