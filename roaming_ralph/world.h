/*
 * world.h
 *
 *  Created on: 2012-05-14
 *      Author: dri
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "pandaFramework.h"
#include "collisionTraverser.h"
#include "collisionRay.h"
#include "collisionHandlerQueue.h"

class World
   {
   public:

   World(WindowFramework* windowFrameworkPtr);

   private:

   enum Alignment
      {
      A_left   = TextNode::A_left,
      A_right  = TextNode::A_right
      };

   NodePath add_title(const string& text) const;
   NodePath add_instructions(float pos, const string& msg) const;
   void set_key(const string& key, bool value);
   AsyncTask::DoneStatus move(GenericAsyncTask* taskPtr);

   World(); // to prevent use of the default constructor
   NodePath onscreen_text(const string& text, const Colorf& fg, const LPoint2f& pos, Alignment align, float scale) const;
   int add_anim(const string& animName, const string& fileName);
   static void sys_exit(const Event* eventPtr, void* dataPtr);
   static void set_key_left(const Event* eventPtr, void* dataPtr);
   static void set_key_right(const Event* eventPtr, void* dataPtr);
   static void set_key_forward(const Event* eventPtr, void* dataPtr);
   static void set_key_cam_left(const Event* eventPtr, void* dataPtr);
   static void set_key_cam_right(const Event* eventPtr, void* dataPtr);
   static void unset_key_left(const Event* eventPtr, void* dataPtr);
   static void unset_key_right(const Event* eventPtr, void* dataPtr);
   static void unset_key_forward(const Event* eventPtr, void* dataPtr);
   static void unset_key_cam_left(const Event* eventPtr, void* dataPtr);
   static void unset_key_cam_right(const Event* eventPtr, void* dataPtr);
   static AsyncTask::DoneStatus move_task(GenericAsyncTask* taskPtr, void* dataPtr);

   PT(WindowFramework) m_windowFrameworkPtr;
   map<string, bool> m_keyMap;
   NodePath m_titleNp;
   NodePath m_inst1Np;
   NodePath m_inst2Np;
   NodePath m_inst3Np;
   NodePath m_inst4Np;
   NodePath m_inst6Np;
   NodePath m_inst7Np;
   NodePath m_environNp;
   NodePath m_ralphNp;
   NodePath m_floaterNp;
   bool m_isMoving;
   AnimControlCollection m_animControlCollection;
   CollisionTraverser m_collisionTraverser;
   PT(CollisionHandlerQueue) m_ralphGroundHandlerPtr;
   PT(CollisionHandlerQueue) m_camGroundHandlerPtr;
   };

#endif /* WORLD_H_ */
