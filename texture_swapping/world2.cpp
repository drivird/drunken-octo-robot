/*
 * world2.cpp
 *
 *  Created on: 2012-08-12
 *      Author: dri
 */

#include "../p3util/cOnscreenText.h"
#include "../p3util/callbackUtil.h"
#include "texturePool.h"
#include "cIntervalManager.h"
#include "world2.h"

// Note: These macros lighten the call to template functions
#define WORLD2_DEFINE_KEY(event_name, description, func) \
   define_key_t<World2, &World2::func>(m_windowFramework, \
                                       event_name,        \
                                       description,       \
                                       this)

#define WORLD2_ADD_TASK(name, func) \
   add_task_t<World2, &World2::func>(name, this)

// Our specialized function to load texture movies and output them as lists
// From textureMovie.py. Look there for a full explanation on how it works
void World2::load_texture_movie(int frames,
                                const string& name,
                                const string& suffix,
                                int padding,
                                vector<PT(Texture)>* texs)
   {
   for(int frameItr = 0; frameItr < frames; ++frameItr)
      {
      ostringstream filename;
      filename << name << setfill('0') << setw(padding)
         << frameItr << "." << suffix;
      PT(Texture) tex = TexturePool::load_texture(filename.str());
      (*texs).push_back(tex);
      }
   }

World2::World2(WindowFramework* windowFramework)
   : m_windowFramework(windowFramework),
     m_title(),
     m_elevator(),
     m_shadowPlane(),
     m_shaft(),
     m_elevatorInterval()
   {
   // Standard initialization stuff
   // Standard title that's on screen in every tutorial
   COnscreenText title("title");
   title.set_text("Panda3D: Tutorial - Texture \"Movies\" (Elevator)");
   title.set_fg(Colorf(1, 1, 1, 1));
   title.set_pos(0.6, -0.95);
   title.set_scale(0.07);
   const NodePath& aspect2d = m_windowFramework->get_aspect_2d();
   title.reparent_to(aspect2d);
   m_title = title.generate();

   // Load the elevator and attach it to render
   const NodePath& models =
      m_windowFramework->get_panda_framework()->get_models();
   m_elevator = m_windowFramework->load_model(models, "../models/elevator");
   const NodePath& render = m_windowFramework->get_render();
   m_elevator.reparent_to(render);

   // Load the plane that will be animated and attach it to the elevator itself
   m_shadowPlane = m_windowFramework->load_model(models,
      "../models/shadowPlane");
   m_shadowPlane.reparent_to(m_elevator);
   m_shadowPlane.set_pos(0, 0, 0.01);
   // Load the textures that will be applied to the polygon
   load_texture_movie(60, "../shadow/barShadows.", "jpg", 1, &m_shadowTexs);

   // Add the task that will animate the plane
   WORLD2_ADD_TASK("elevatorTask", elevator_shadows);

   // Builds the shaft, which is a 30ft repeatable segment
   for(int i = 0; i < 4; ++i)
      {
      NodePath sh = m_windowFramework->load_model(models, "../models/shaft");
      sh.reparent_to(render);
      sh.set_pos(-6.977, 0, 30*(i-1));
      m_shaft.push_back(sh);
      }

   // Linearly move the elevator's height using an interval.
   // If you replaced this with some other way of moving the elevator, the
   // texture would compensate since it's based on height and not time
   const bool bakeInStart = true;
   const bool fluid = false;
   m_elevatorInterval = new CLerpNodePathInterval("elevatorInterval", 5,
      CLerpInterval::BT_no_blend, bakeInStart, fluid, m_elevator, NodePath());
   const float x = m_elevator.get_x();
   const float y = m_elevator.get_y();
   m_elevatorInterval->set_start_pos(LVecBase3f(x, y, 30));
   m_elevatorInterval->set_end_pos(LVecBase3f(x, y, -30));
   m_elevatorInterval->loop();

   // Puts the camera relative to the elevator in a position that
   // shows off the texture movie
   // base.disableMouse() no need for that in C++
   NodePath camera = m_windowFramework->get_camera_group();
   camera.reparent_to(m_elevator);
   camera.set_pos_hpr(-9, 0, 20, -90, -60, 0);
   }

// The task that runs the elevator. This is nearly the same as the one in
// textureMovie.py with a few differences:
// 1) This task is only used once so the parameters were placed with hard-coded
//    Values
// 2) Intead of basing the current frame on task.time,
//    it's based on elevator.getZ()
AsyncTask::DoneStatus World2::elevator_shadows(GenericAsyncTask *task)
   {
   const float z = m_elevator.get_z();
   int texFrame = int(2*(z-floor(z/30)*30)+26)%60;
   m_shadowPlane.set_texture(m_shadowTexs[texFrame], 1);

   // Note: step the interval manager to run effectively move the elevator
   CIntervalManager::get_global_ptr()->step();
   return AsyncTask::DS_cont;
   }
