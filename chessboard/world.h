/*
 * world.h
 *
 *  Created on: 2012-07-08
 *      Author: dri
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "pandaFramework.h"
#include "util.h"
#include "collisionTraverser.h"
#include "collisionHandlerQueue.h"
#include "collisionRay.h"
#include "piece.h"

class Piece;

class World
   {
   public:

   World(WindowFramework* WindowFramework);
   ~World();

   private:

   static const Colorf BLACK;
   static const Colorf WHITE;
   static const Colorf HIGHLIGHT;
   static const Colorf PIECEBLACK;

   World(); // to prevent use of the default constructor
   void swap_pieces(int fr, int to);
   void mouse_task();
   void grab_piece();
   void release_piece();
   void setup_lights();

   static LPoint3f point_at_z(float z, LPoint3f point, LVector3f vec);
   static const Colorf& square_color(int i);
   static void call_exit(const Event* event, void* data);
   static AsyncTask::DoneStatus call_mouse_task(GenericAsyncTask *task, void *user_data);
   static void call_grab_piece(const Event* event, void* data);
   static void call_release_piece(const Event* event, void* data);

   PT(WindowFramework) m_windowFramework;
   NodePath m_title;
   NodePath m_escapeEvent;
   NodePath m_mouse1Event;
   CollisionTraverser m_picker;
   PT(CollisionHandlerQueue) m_pq;
   PT(CollisionNode) m_pickerNode;
   NodePath m_pickerNP;
   PT(CollisionRay) m_pickerRay;
   NodePath m_squareRoot;
   vector<NodePath> m_squares;
   vector<Piece*> m_pieces;
   int m_hiSq;
   int m_dragging;
   PT(GenericAsyncTask) m_mouseTask;
   };

#endif /* WORLD_H_ */
