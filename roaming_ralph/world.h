/*
 * world.h
 *
 *  Created on: 2012-05-14
 *      Author: dri
 */

#ifndef WORLD_H_
#define WORLD_H_

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

   enum Key
      {
      K_left = 0,
      K_right,
      K_forward,
      K_cam_left,
      K_cam_right,
      K_keys
      };

   NodePath add_title(const string& text) const;
   NodePath add_instructions(float pos, const string& msg) const;
   void set_key(Key key, bool value);
   void move();

   World(); // to prevent use of the default constructor
   NodePath onscreen_text(const string& text, const Colorf& fg, const LPoint2f& pos, Alignment align, float scale) const;
   NodePath load_actor(AnimControlCollection* controlsPtr, const string& actorFilename, const map<string,string>& animMap, int hierarchyMatchFlags = 0);
   static void sys_exit(const Event* eventPtr, void* dataPtr);
   template<int key, bool value> static void call_set_key(const Event* eventPtr, void* dataPtr);
   static AsyncTask::DoneStatus call_move(GenericAsyncTask* taskPtr, void* dataPtr);

   PT(WindowFramework) m_windowFrameworkPtr;
   vector<bool> m_keyMap;
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
   PT(CollisionRay) m_ralphGroundRayPtr;
   PT(CollisionRay) m_camGroundRayPtr;
   PT(CollisionNode) m_ralphGroundColPtr;
   PT(CollisionNode) m_camGroundColPtr;
   PT(CollisionHandlerQueue) m_ralphGroundHandlerPtr;
   PT(CollisionHandlerQueue) m_camGroundHandlerPtr;
   };

#endif /* WORLD_H_ */
