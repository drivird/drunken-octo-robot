/*
 * world.cpp
 *
 *  Created on: 2012-05-19
 *      Author: dri
 */

#include "../p3util/cOnscreenText.h"
#include "pandaFramework.h"
#include "bitMask.h"
#include "ambientLight.h"
#include "directionalLight.h"
#include "material.h"
#include "mouseWatcher.h"
#include "cLerpNodePathInterval.h"
#include "waitInterval.h"
#include "cMetaInterval.h"
#include "cIntervalManager.h"
#include "world.h"

World::World(WindowFramework* windowFrameworkPtr)
   : UP(0,0,1),
     m_windowFrameworkPtr(windowFrameworkPtr)
   {
   // preconditions
   if(m_windowFrameworkPtr == NULL)
      {
      nout << "ERROR: parameter windowFrameworkPtr cannot be NULL." << endl;
      return;
      }

   // This code puts the standard title and instruction text on screen
   COnscreenText title("title", COnscreenText::TS_plain);
   title.set_text("Panda3D: Tutorial - Collision Detection");
   title.set_fg(Colorf(1,1,1,1));
   title.set_pos(LVecBase2f(0.7,-0.95));
   title.set_scale(0.07);
   title.reparent_to(m_windowFrameworkPtr->get_aspect_2d());
   m_titleNp = title.generate();


   COnscreenText instructions("instructions");
   instructions.set_text("Mouse pointer tilts the board");
   instructions.set_pos(LVecBase2f(-1.3, 0.95));
   instructions.set_fg(Colorf(1,1,1,1));
   instructions.set_align(TextNode::A_left);
   instructions.set_scale(0.05);
   instructions.reparent_to(m_windowFrameworkPtr->get_aspect_2d());
   m_instructionsNp = instructions.generate();

   // Escape quits
   m_windowFrameworkPtr->enable_keyboard();
   m_windowFrameworkPtr->get_panda_framework()->define_key("escape", "sysExit", sys_exit, NULL);

   // Disable mouse-based camera control
   // Note: irrelevant in C++

   // Place the camera
   NodePath cameraNp = m_windowFrameworkPtr->get_camera_group();
   cameraNp.set_pos_hpr(0, 0, 25, 0, -90, 0);

   // Load the maze and place it in the scene
   NodePath modelsNp = m_windowFrameworkPtr->get_panda_framework()->get_models();
   m_mazeNp = m_windowFrameworkPtr->load_model(modelsNp, "../models/maze");
   NodePath renderNp = m_windowFrameworkPtr->get_render();
   m_mazeNp.reparent_to(renderNp);

   // Most times, you want collisions to be tested against invisible geometry
   // rather than every polygon. This is because testing against every polygon
   // in the scene is usually too slow. You can have simplified or approximate
   // geometry for the solids and still get good results.
   //
   // Sometimes you'll want to create and position your own collision solids in
   // code, but it's often easier to have them built automatically. This can be
   // done by adding special tags into an egg file. Check maze.egg and ball.egg
   // and look for lines starting with <Collide>. The part is brackets tells
   // Panda exactly what to do. Polyset means to use the polygons in that group
   // as solids, while Sphere tells panda to make a collision sphere around them
   // Keep means to keep the polygons in the group as visable geometry (good
   // for the ball, not for the triggers), and descend means to make sure that
   // the settings are applied to any subgroups.
   //
   // Once we have the collision tags in the models, we can get to them using
   // NodePath's find command

   // Find the collision node named wall_collide
   m_wallsNp = m_mazeNp.find("**/wall_collide");

   // Collision objects are sorted using BitMasks. BitMasks are ordinary numbers
   // with extra methods for working with them as binary bits. Every collision
   // solid has both a from mask and an into mask. Before Panda tests two
   // objects, it checks to make sure that the from and into collision masks
   // have at least one bit in common. That way things that shouldn't interact
   // won't. Normal model nodes have collision masks as well. By default they
   // are set to bit 20. If you want to collide against actual visable polygons,
   // set a from collide mask to include bit 20
   //
   // For this example, we will make everything we want the ball to collide with
   // include bit 0
   m_wallsNp.node()->set_into_collide_mask(BitMask32::bit(0));
   // CollisionNodes are usually invisible but can be shown. Uncomment the next
   // line to see the collision walls
   // m_wallsNp.show();

   // We will now find the triggers for the holes and set their masks to 0 as
   // well. We also set their names to make them easier to identify during
   // collisions
   m_loseTriggers.reserve(NB_HOLES);
   for(int i = 0; i < NB_HOLES; ++i)
      {
      ostringstream filename;
      filename << "**/hole_collide" << i;
      NodePath triggerNp = m_mazeNp.find(filename.str());
      triggerNp.node()->set_into_collide_mask(BitMask32::bit(0));
      triggerNp.node()->set_name("loseTrigger");
      m_loseTriggers.push_back(triggerNp);
      // Uncomment this line to see the triggers
      // triggerNp.show();
      }

   // Ground_collide is a single polygon on the same plane as the ground in the
   // maze. We will use a ray to collide with it so that we will know exactly
   // what height to put the ball at every frame. Since this is not something
   // that we want the ball itself to collide with, it has a different
   // bitmask.
   m_mazeGroundNp = m_mazeNp.find("**/ground_collide");
   m_mazeGroundNp.node()->set_into_collide_mask(BitMask32::bit(1));

   // Load the ball and attach it to the scene
   // It is on a root dummy node so that we can rotate the ball itself without
   // rotating the ray that will be attached to it
   m_ballRootNp = renderNp.attach_new_node("ballRoot");
   m_ballNp = m_windowFrameworkPtr->load_model(modelsNp, "../models/ball");
   m_ballNp.reparent_to(m_ballRootNp);

   // Find the collision sphere for the ball which was created in the egg file
   // Notice that it has a from collision mask of bit 0, and an into collision
   // mask of no bits. This means that the ball can only cause collisions, not
   // be collided into
   m_ballSphereNp = m_ballNp.find("**/ball");
   static_cast<CollisionNode*>(m_ballSphereNp.node())->set_from_collide_mask(BitMask32::bit(0));
   m_ballSphereNp.node()->set_into_collide_mask(BitMask32::all_off());

   // No we create a ray to start above the ball and cast down. This is to
   // Determine the height the ball should be at and the angle the floor is
   // tilting. We could have used the sphere around the ball itself, but it
   // would not be as reliable

   // Create the ray
   m_ballGroundRayPtr = new CollisionRay();
   if(m_ballGroundRayPtr != NULL)
      {
      // Set its origin
      m_ballGroundRayPtr->set_origin(0,0,10);
      // And its direction
      m_ballGroundRayPtr->set_direction(0,0,-1);
      // Collision solids go in CollisionNode
      // Create and name the node
      m_ballGroundColPtr = new CollisionNode("groundRay");
      if(m_ballGroundColPtr != NULL)
         {
         // Add the ray
         m_ballGroundColPtr->add_solid(m_ballGroundRayPtr);
         // Set its bitmasks
         m_ballGroundColPtr->set_from_collide_mask(BitMask32::bit(1));
         m_ballGroundColPtr->set_into_collide_mask(BitMask32::all_off());
         // Attach the node to the ballRoot so that the ray is relative to the ball
         // (it will always be 10 feet over the ball and point down)
         m_ballGroundColNp = m_ballRootNp.attach_new_node(m_ballGroundColPtr);
         // Uncomment this line to see the ray
         // m_ballGroundColNp.show();
         }
      }

   // Finally, we create a CollisionTraverser. CollisionTraversers are what
   // do the job of calculating collisions
   // Note: no need to in this implementation

   // Collision traversers tell collision handlers about collisions, and then
   // the handler decides what to do with the information. We are using a
   // CollisionHandlerQueue, which simply creates a list of all of the
   // collisions in a given pass. There are more sophisticated handlers like
   // one that sends events and another that tries to keep collided objects
   // apart, but the results are often better with a simple queue
   m_cHandlerPtr = new CollisionHandlerQueue();
   if(m_cHandlerPtr != NULL)
      {
      // Now we add the collision nodes that can create a collision to the
      // traverser. The traverser will compare these to all others nodes in the
      // scene. There is a limit of 32 CollisionNodes per traverser
      // We add the collider, and the handler to use as a pair
      m_cTrav.add_collider(m_ballSphereNp, m_cHandlerPtr);
      m_cTrav.add_collider(m_ballGroundColNp, m_cHandlerPtr);
      }

   // Collision traversers have a built in tool to help visualize collisions.
   // Uncomment the next line to see it.
   // m_cTrav.show_collisions(renderNp);

   // This section deals with lighting for the ball. Only the ball was lit
   // because the maze has static lighting pregenerated by the modeler
   PT(AmbientLight) ambientLightPtr = new AmbientLight("ambientLight");
   if(ambientLightPtr != NULL)
      {
      ambientLightPtr->set_color(Colorf(0.55, 0.55, 0.55, 1));
      m_ballRootNp.set_light(renderNp.attach_new_node(ambientLightPtr));
      }
   PT(DirectionalLight) directionalLightPtr = new DirectionalLight("directionalLight");
   if(directionalLightPtr != NULL)
      {
      directionalLightPtr->set_direction(LVecBase3f(0, 0, -1));
      directionalLightPtr->set_color(Colorf(0.375, 0.375, 0.375, 1));
      directionalLightPtr->set_specular_color(Colorf(1, 1, 1, 1));
      m_ballRootNp.set_light(renderNp.attach_new_node(directionalLightPtr));
      }

   // This section deals with adding a specular highlight to the ball to make
   // it look shiny
   PT(Material) materialPtr = new Material();
   if(materialPtr != NULL)
      {
      materialPtr->set_specular(Colorf(1,1,1,1));
      materialPtr->set_shininess(96);
      m_ballNp.set_material(materialPtr, 1);
      }

   // Finally, we call start for more initialization
   start();
   }

void World::start()
   {
   // The maze model also has a locator in it for where to start the ball
   // To access it we use the find command
   LPoint3f startPos = m_mazeNp.find("**/start").get_pos();
   // Set the ball in the starting position
   m_ballRootNp.set_pos(startPos);
   // Initial velocity is 0
   m_ballV = LVector3f(0,0,0);
   // Initial acceleration is 0
   m_accelV = LVector3f(0,0,0);

   // For a traverser to actually do collisions, you need to call
   // traverser.traverse() on a part of the scene. Fortunately, base has a
   // task that does this for the entire scene once a frame. This sets up our
   // traverser as the one to be called automatically
   // Note: have to do it manually in C++
   PT(GenericAsyncTask) traverserTaskPtr = new GenericAsyncTask("traverser", call_traverse, this);
   if(traverserTaskPtr != NULL)
      {
      AsyncTaskManager::get_global_ptr()->add(traverserTaskPtr);
      }

   // Create the movement task, but first make sure it is not already running
   PT(GenericAsyncTask) rollTaskPtr = static_cast<GenericAsyncTask*>(AsyncTaskManager::get_global_ptr()->find_task("rollTask"));
   if(rollTaskPtr == NULL)
      {
      rollTaskPtr = new GenericAsyncTask("rollTask", call_roll, this);
      if(rollTaskPtr != NULL)
         {
         AsyncTaskManager::get_global_ptr()->add(rollTaskPtr);
         }
      }
   m_last = 0;
   }

// This function handles the collision between the ray and the ground
// Information about the interaction is passed in colEntry
void World::ground_collide_handler(const CollisionEntry& colEntry)
   {
   // Set the ball to the appropriate Z value for it to be exactly on the ground
   NodePath renderNp = m_windowFrameworkPtr->get_render();
   float newZ = colEntry.get_surface_point(renderNp).get_z();
   m_ballRootNp.set_z(newZ + 0.4);

   // Find the acceleration direction. First the surface normal is crossed with
   // the up vector to get a vector perpendicular to the slope
   LVector3f norm = colEntry.get_surface_normal(renderNp);
   LVector3f accelSide = norm.cross(UP);
   // Then that vector is crossed with the surface normal to get a vector that
   // points down the slope. By getting the acceleration in 3D like this rather
   // than in 2D, we reduce the amount of error per-frame, reducing jitter
   m_accelV = norm.cross(accelSide);
   }

// This function handles the collision between the ball and a wall
void World::wall_collide_handler(const CollisionEntry& colEntry)
   {
   // First we calculate some numbers we need to do a reflection
   NodePath renderNp = m_windowFrameworkPtr->get_render();
   // The normal of the wall
   LVector3f norm = colEntry.get_surface_normal(renderNp) * -1;
   // The current speed
   float curSpeed = m_ballV.length();
   // The direction of travel
   LVector3f inVec = m_ballV / curSpeed;
   // Angle of incidence
   float velAngle = norm.dot(inVec);
   LPoint3f hitDir = colEntry.get_surface_point(renderNp) - m_ballRootNp.get_pos();
   hitDir.normalize();
   // The angle between the ball and the normal
   float hitAngle = norm.dot(hitDir);

   // Ignore the collision if the ball is either moving away from the wall
   // already (so that we don't accidentally send it back into the wall)
   // and ignore it if the collision isn't dead-on (to avoid getting caught on
   // corners)
   if(velAngle > 0 && hitAngle > .995)
      {
      // Standard reflection equation
      LVector3f reflectVec = (norm * norm.dot(inVec * -1) * 2) + inVec;

      // This makes the velocity half of what it was if the hit was dead-on
      // and nearly exactly what it was if this is a glancing blow
      m_ballV = reflectVec * (curSpeed * (((1-velAngle)*.5)+.5));
      // Since we have a collision, the ball is already a little bit buried in
      // the wall. This calculates a vector needed to move it so that it is
      // exactly touching the wall
      LPoint3f disp = (colEntry.get_surface_point(renderNp) -
                       colEntry.get_interior_point(renderNp));
      LPoint3f newPos = m_ballRootNp.get_pos() + disp;
      m_ballRootNp.set_pos(newPos);
      }
   }

// This is the task that deals with making everything interactive
AsyncTask::DoneStatus World::roll(GenericAsyncTask* taskPtr)
   {
   // Standard technique for finding the amount of time since the last frame
   double dt = taskPtr->get_elapsed_time() - m_last;
   m_last = taskPtr->get_elapsed_time();

   // If dt is large, then there has been a # hiccup that could cause the ball
   // to leave the field if this functions runs, so ignore the frame
   if(dt > 0.2) { return AsyncTask::DS_cont; }

   // The collision handler collects the collisions. We dispatch which function
   // to handle the collision based on the name of what was collided into
   for(int i = 0; i < m_cHandlerPtr->get_num_entries(); ++i)
      {
      PT(CollisionEntry) entryPtr = m_cHandlerPtr->get_entry(i);
      const string& name = entryPtr->get_into_node()->get_name();
      if(name == "wall_collide")        { wall_collide_handler(*entryPtr);   }
      else if(name == "ground_collide") { ground_collide_handler(*entryPtr); }
      else if(name == "loseTrigger")    { lose_game(*entryPtr);              }
      }

   // Read the mouse position and tilt the maze accordingly
   PT(MouseWatcher) mouseWatcherPtr = DCAST(MouseWatcher, m_windowFrameworkPtr->get_mouse().node());
   if(mouseWatcherPtr->has_mouse())
      {
      // get the mouse position
      const LPoint2f& mpos = mouseWatcherPtr->get_mouse();
      m_mazeNp.set_p(mpos.get_y() * -10);
      m_mazeNp.set_r(mpos.get_x() * 10);
      }

   // Finally, we move the ball
   // Update the velocity based on acceleration
   m_ballV += m_accelV * dt * ACCEL;
   // Clamp the velocity to the maximum speed
   if(m_ballV.length_squared() > MAX_SPEED_SQ)
      {
      m_ballV.normalize();
      m_ballV *= MAX_SPEED;
      }
   // Update the position based on the velocity
   m_ballRootNp.set_pos(m_ballRootNp.get_pos() + (m_ballV * dt));

   // This block of code rotates the ball. It uses something called a quaternion
   // to rotate the ball around an arbitrary axis. That axis perpendicular to
   // the balls rotation, and the amount has to do with the size of the ball
   // This is multiplied on the previous rotation to incrementally turn it.
   LRotationf prevRot(m_ballNp.get_quat());
   LVector3f axis = UP.cross(m_ballV);
   LRotationf newRot(axis, 45.5 * dt * m_ballV.length());
   m_ballNp.set_quat(prevRot * newRot);

   // Continue the task indefinitely
   return AsyncTask::DS_cont;
   }

// If the ball hits a hole trigger, then it should fall in the hole.
// This is faked rather than dealing with the actual physics of it.
void World::lose_game(const CollisionEntry& entry)
   {
   // The triggers are set up so that the center of the ball should move to the
   // collision point to be in the hole
   NodePath renderNp = m_windowFrameworkPtr->get_render();
   LPoint3f toPos = entry.get_interior_point(renderNp);

   // Stop the maze task
   PT(GenericAsyncTask) rollTaskPtr = static_cast<GenericAsyncTask*>(AsyncTaskManager::get_global_ptr()->find_task("rollTask"));
   if(rollTaskPtr != NULL)
      {
      AsyncTaskManager::get_global_ptr()->remove(rollTaskPtr);
      }

   // Move the ball into the hole over a short sequence of time. Then wait a
   // second and call start to reset the game
   // Note: Sequence is a python only class. We have to manage using CMetaInterval for the animation
   //       with a callback event when the animation is done to callback on World::start() to restart the game.
   PT(CLerpNodePathInterval) lerp1Ptr = new CLerpNodePathInterval("lerp1",
                                                                   0.1,
                                                                   CLerpInterval::BT_no_blend,
                                                                   true,
                                                                   false,
                                                                   m_ballRootNp,
                                                                   NodePath());

   PT(CLerpNodePathInterval) lerp2Ptr = new CLerpNodePathInterval("lerp2",
                                                                  0.1,
                                                                  CLerpInterval::BT_no_blend,
                                                                  true,
                                                                  false,
                                                                  m_ballRootNp,
                                                                  NodePath());

   PT(WaitInterval) waitPtr = new WaitInterval(1);

   PT(CMetaInterval) cMetaIntervalPtr = new CMetaInterval("sequence");

   if(lerp1Ptr         == NULL ||
      lerp2Ptr         == NULL ||
      waitPtr          == NULL ||
      cMetaIntervalPtr == NULL)
      {
      nout << "ERROR: out of memory" << endl;
      return;
      }

   float endPosZ = m_ballRootNp.get_pos().get_z() - 0.9;
   LVecBase3f midEndPos(toPos.get_x(),
                       toPos.get_y(),
                       0.5*(m_ballRootNp.get_pos().get_z()+endPosZ));
   lerp1Ptr->set_end_pos(midEndPos);
   LVecBase3f endPos(toPos.get_x(),
                     toPos.get_y(),
                     endPosZ);
   lerp2Ptr->set_end_pos(endPos);

   cMetaIntervalPtr->add_c_interval(lerp1Ptr, 0, CMetaInterval::RS_previous_end);
   cMetaIntervalPtr->add_c_interval(lerp2Ptr, 0, CMetaInterval::RS_previous_end);
   cMetaIntervalPtr->add_c_interval(waitPtr , 0, CMetaInterval::RS_previous_end);
   cMetaIntervalPtr->set_done_event("restartGame");
   cMetaIntervalPtr->start();

   EventHandler::get_global_event_handler()->add_hook("restartGame", call_start, this);

   PT(GenericAsyncTask) intervalManagerTaskPtr = static_cast<GenericAsyncTask*>(AsyncTaskManager::get_global_ptr()->find_task("intervalManagerTask"));
   if(intervalManagerTaskPtr == NULL)
      {
      intervalManagerTaskPtr = new GenericAsyncTask("intervalManagerTask", step_interval_manager, NULL);
      if(intervalManagerTaskPtr != NULL)
         {
         AsyncTaskManager::get_global_ptr()->add(intervalManagerTaskPtr);
         }
      }
   }

void World::sys_exit(const Event* eventPtr, void* dataPtr)
   {
   exit(0);
   }

AsyncTask::DoneStatus World::call_traverse(GenericAsyncTask *taskPtr, void *dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return AsyncTask::DS_done;
      }

   static_cast<World*>(dataPtr)->traverse();
   return AsyncTask::DS_cont;
   }

void World::traverse()
   {
   NodePath renderNp = m_windowFrameworkPtr->get_render();
   m_cTrav.traverse(renderNp);
   }

AsyncTask::DoneStatus World::call_roll(GenericAsyncTask *taskPtr, void *dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return AsyncTask::DS_done;
      }

   return static_cast<World*>(dataPtr)->roll(taskPtr);
   }

AsyncTask::DoneStatus World::step_interval_manager(GenericAsyncTask *taskPtr, void *dataPtr)
   {
   CIntervalManager::get_global_ptr()->step();
   return AsyncTask::DS_cont;
   }

void World::call_start(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<World*>(dataPtr)->start();
   }
