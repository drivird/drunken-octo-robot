/*
 * world.h
 *
 *  Created on: 2012-05-19
 *      Author: dri
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "collisionRay.h"
#include "collisionHandlerQueue.h"

class World
   {
   public:

   World(WindowFramework* windowFrameworkPtr);

   private:

   // Some constants for the program
   static const int NB_HOLES     =  6; // Note: Holes in the maze
   static const int ACCEL        = 70; // Acceleration in ft/sec/sec
   static const int MAX_SPEED    =  5; // Max speed in ft/sec
   static const int MAX_SPEED_SQ = 25; // Squared to make it easier to use lengthSquared
                                       // Instead of length
   const LVecBase3f UP; // We need this vector a lot, so its better to just have one
                        // instead of creating a new one every time we need it

   enum Alignment
      {
      A_center  = TextNode::A_center,
      A_left   = TextNode::A_left
      };

   void start();
   void ground_collide_handler(const CollisionEntry& colEntry);
   void wall_collide_handler(const CollisionEntry& colEntry);
   void lose_game(const CollisionEntry& entry);

   World(); // to prevent use of the default constructor
   NodePath onscreen_text(const string& text, const Colorf& fg, const LPoint2f& pos, Alignment align, float scale) const;
   void traverse();
   AsyncTask::DoneStatus roll(GenericAsyncTask *taskPtr);
   static void sys_exit(const Event* eventPtr, void* dataPtr);
   static AsyncTask::DoneStatus call_traverse(GenericAsyncTask *taskPtr, void *dataPtr);
   static AsyncTask::DoneStatus call_roll(GenericAsyncTask *taskPtr, void *dataPtr);
   static AsyncTask::DoneStatus step_interval_manager(GenericAsyncTask *taskPtr, void *dataPtr);
   static void call_start(const Event* eventPtr, void* dataPtr);

   PT(WindowFramework) m_windowFrameworkPtr;
   PT(CollisionRay) m_ballGroundRayPtr;
   PT(CollisionNode) m_ballGroundColPtr;
   PT(CollisionHandlerQueue) m_cHandlerPtr;
   NodePath m_titleNp;
   NodePath m_instructionsNp;
   NodePath m_mazeNp;
   NodePath m_wallsNp;
   NodePath m_mazeGroundNp;
   NodePath m_ballRootNp;
   NodePath m_ballNp;
   NodePath m_ballSphereNp;
   NodePath m_ballGroundColNp;
   vector<NodePath> m_loseTriggers;
   CollisionTraverser m_cTrav;
   LVecBase3f m_ballV;
   LVecBase3f m_accelV;
   double m_last;
   };

#endif /* WORLD_H_ */
