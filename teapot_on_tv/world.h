/*
 * world.h
 *
 *  Created on: 2012-08-07
 *      Author: dri
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "pandaFramework.h"
#include "cLerpNodePathInterval.h"
#include "../p3util/cBufferViewer.h"
#include "../p3util/cActor.h"

class World
   {
   public:

   World(WindowFramework* windowFramework);
   ~World();

   private:

   NodePath add_instructions(float pos, const string& msg);
   NodePath add_title(const string& text);
   AsyncTask::DoneStatus async_task(GenericAsyncTask *task);
   void toggle_buffer_viewer(const Event* event);
   void make_tv_man(float x, float y, float z, Texture* tex, float playrate);
   void quit(const Event* event);
   void zoom_in(const Event* event);
   void zoom_out(const Event* event);
   void move_left(const Event* event);
   void move_right(const Event* event);

   PT(WindowFramework) m_windowFramework;
   NodePath m_title;
   NodePath m_inst1;
   NodePath m_inst2;
   NodePath m_inst3;
   NodePath m_inst4;
   NodePath m_altCam;
   NodePath m_teapot;
   PT(CLerpNodePathInterval) m_teapotInterval;
   CBufferViewer* m_bufferViewer;
   vector<CActor> m_tvMen;
   };

inline
World::~World()
   {
   delete m_bufferViewer;
   }

#endif /* WORLD_H_ */
