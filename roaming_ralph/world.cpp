/*
 * world.cpp
 *
 *  Created on: 2012-05-14
 *      Author: dri
 */


#include "pandaFramework.h"
#include "ambientLight.h"
#include "directionalLight.h"
#include "auto_bind.h"
#include "world.h"

const float SPEED = 0.5; // Note: unused

// Function to put instructions on the screen.
NodePath World::add_instructions(float pos, const string& msg) const
   {
   return onscreen_text(msg, Colorf(1,1,1,1), LPoint2f(-1.3, pos), A_left, 0.05);
   }

// Function to put title on the screen.
NodePath World::add_title(const string& text) const
   {
   return onscreen_text(text, Colorf(1,1,1,1), LPoint2f(1.3,-0.95), A_right, 0.07);
   }

World::World(WindowFramework* windowFrameworkPtr)
   : m_windowFrameworkPtr(windowFrameworkPtr)
   {
   // preconditions
   if(m_windowFrameworkPtr == NULL)
      {
      nout << "ERROR: World::World(WindowFramework* windowFrameworkPtr) parameter windowFrameworkPtr cannot be NULL." << endl;
      return;
      }

   m_keyMap["left"     ] = false;
   m_keyMap["right"    ] = false;
   m_keyMap["forward"  ] = false;
   m_keyMap["cam-left" ] = false;
   m_keyMap["cam-right"] = false;
   m_windowFrameworkPtr->get_graphics_window()->get_active_display_region(0)->set_clear_color(Colorf(0,0,0,1));

   // Post the instructions

   m_titleNp = add_title("Panda3D Tutorial: Roaming Ralph (Walking on Uneven Terrain)");
   m_inst1Np = add_instructions(0.95, "[ESC]: Quit");
   m_inst2Np = add_instructions(0.90, "[Left Arrow]: Rotate Ralph Left");
   m_inst3Np = add_instructions(0.85, "[Right Arrow]: Rotate Ralph Right");
   m_inst4Np = add_instructions(0.80, "[Up Arrow]: Run Ralph Forward");
   m_inst6Np = add_instructions(0.70, "[A]: Rotate Camera Left");
   m_inst7Np = add_instructions(0.65, "[S]: Rotate Camera Right");

   // Set up the environment
   //
   // This environment model contains collision meshes.  If you look
   // in the egg file, you will see the following:
   //
   //    <Collide> { Polyset keep descend }
   //
   // This tag causes the following mesh to be converted to a collision
   // mesh -- a mesh which is optimized for collision, not rendering.
   // It also keeps the original mesh, so there are now two copies ---
   // one optimized for rendering, one for collisions.

   NodePath modelsNp = m_windowFrameworkPtr->get_panda_framework()->get_models();
   m_environNp = m_windowFrameworkPtr->load_model(modelsNp, "../models/world");
   NodePath renderNp = m_windowFrameworkPtr->get_render();
   m_environNp.reparent_to(renderNp);
   m_environNp.set_pos(0,0,0);

   // Create the main character, Ralph
   LPoint3f ralphStartPos = m_environNp.find("**/start_point").get_pos();
   m_ralphNp = m_windowFrameworkPtr->load_model(modelsNp, "../models/ralph");
   map<string, string> ralphAnims;
   ralphAnims["run"] = "../models/ralph-run";
   ralphAnims["walk"] = "../models/ralph-walk";
   auto_bind_named(m_ralphNp, m_animControlCollection, ralphAnims, PartGroup::HMF_ok_part_extra |
                                                                   PartGroup::HMF_ok_anim_extra |
                                                                   PartGroup::HMF_ok_wrong_root_name);
   m_ralphNp.reparent_to(renderNp);
   m_ralphNp.set_scale(0.2);
   m_ralphNp.set_pos(ralphStartPos);

   // Create a floater object.  We use the "floater" as a temporary
   // variable in a variety of calculations.

   m_floaterNp = NodePath("floater");
   m_floaterNp.reparent_to(renderNp);

   // Accept the control keys for movement and rotation
   m_windowFrameworkPtr->enable_keyboard();
   m_windowFrameworkPtr->get_panda_framework()->define_key("escape"        , "sysExit"    , sys_exit           , NULL);
   m_windowFrameworkPtr->get_panda_framework()->define_key("arrow_left"    , "left"       , set_key_left       , this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("arrow_right"   , "right"      , set_key_right      , this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("arrow_up"      , "forward"    , set_key_forward    , this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("a"             , "cam-left"   , set_key_cam_left   , this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("s"             , "cam-right"  , set_key_cam_right  , this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("arrow_left-up" , "leftUp"     , unset_key_left     , this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("arrow_right-up", "rightUp"    , unset_key_right    , this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("arrow_up-up"   , "forwardUp"  , unset_key_forward  , this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("a-up"          , "cam-leftUp" , unset_key_cam_left , this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("s-up"          , "cam-rightUp", unset_key_cam_right, this);

   PT(GenericAsyncTask) taskPtr = new GenericAsyncTask("moveTask", call_move, this);
   if(taskPtr != NULL)
      {
      AsyncTaskManager::get_global_ptr()->add(taskPtr);
      }

   // Game state variables
   m_isMoving = false;

   // Set up the camera

   // Note: no need to disable the mouse in C++
   NodePath cameraNp = m_windowFrameworkPtr->get_camera_group();
   cameraNp.set_pos(m_ralphNp.get_x(), m_ralphNp.get_y()+10, 2);

   // We will detect the height of the terrain by creating a collision
   // ray and casting it downward toward the terrain.  One ray will
   // start above ralph's head, and the other will start above the camera.
   // A ray may hit the terrain, or it may hit a rock or a tree.  If it
   // hits the terrain, we can detect the height.  If it hits anything
   // else, we rule that the move is illegal.

   NodePath ralphGroundColNp;
   m_ralphGroundRayPtr = new CollisionRay();
   if(m_ralphGroundRayPtr != NULL)
      {
      m_ralphGroundRayPtr->set_origin(0, 0, 1000);
      m_ralphGroundRayPtr->set_direction(0, 0, -1);
      m_ralphGroundColPtr = new CollisionNode("ralphRay");
      if(m_ralphGroundColPtr != NULL)
         {
         m_ralphGroundColPtr->add_solid(m_ralphGroundRayPtr);
         m_ralphGroundColPtr->set_from_collide_mask(BitMask32::bit(0));
         m_ralphGroundColPtr->set_into_collide_mask(BitMask32::all_off());
         ralphGroundColNp = m_ralphNp.attach_new_node(m_ralphGroundColPtr);
         m_ralphGroundHandlerPtr = new CollisionHandlerQueue();
         if(m_ralphGroundHandlerPtr != NULL)
            {
            m_collisionTraverser.add_collider(ralphGroundColNp, m_ralphGroundHandlerPtr);
            }
         }
      }

   NodePath camGroundColNp;
   m_camGroundRayPtr = new CollisionRay();
   if(m_camGroundRayPtr != NULL)
      {
      m_camGroundRayPtr->set_origin(0, 0, 1000);
      m_camGroundRayPtr->set_direction(0, 0, -1);
      m_camGroundColPtr = new CollisionNode("camRay");
      if(m_camGroundColPtr != NULL)
         {
         m_camGroundColPtr->add_solid(m_camGroundRayPtr);
         m_camGroundColPtr->set_from_collide_mask(BitMask32::bit(0));
         m_camGroundColPtr->set_into_collide_mask(BitMask32::all_off());
         camGroundColNp = cameraNp.attach_new_node(m_camGroundColPtr);
         m_camGroundHandlerPtr = new CollisionHandlerQueue();
         if(m_camGroundHandlerPtr != NULL)
            {
            m_collisionTraverser.add_collider(camGroundColNp, m_camGroundHandlerPtr);
            }
         }
      }

   // Uncomment this line to see the collision rays
   //ralphGroundColNp.show();
   //camGroundColNp.show();

   // Uncomment this line to show a visual representation of the
   // collisions occuring
   //m_collisionTraverser.show_collisions(renderNp);

   // Create some lighting
   PT(AmbientLight) ambientLightPtr = new AmbientLight("ambientLight");
   if(ambientLightPtr != NULL)
      {
      ambientLightPtr->set_color(Colorf(.3, .3, .3, 1));
      renderNp.set_light(renderNp.attach_new_node(ambientLightPtr));
      }
   PT(DirectionalLight) directionalLightPtr = new DirectionalLight("directionalLightPtr");
   if(directionalLightPtr != NULL)
      {
      directionalLightPtr->set_direction(LVecBase3f(-5, -5, -5));
      directionalLightPtr->set_color(Colorf(1, 1, 1, 1));
      directionalLightPtr->set_specular_color(Colorf(1, 1, 1, 1));
      renderNp.set_light(renderNp.attach_new_node(directionalLightPtr));
      }
   }

// Records the state of the arrow keys
void World::set_key(const string& key, bool value)
   {
   m_keyMap[key] = value;
   }

// Accepts arrow keys to move either the player or the menu cursor,
// Also deals with grid checking and collision detection
void World::move()
   {
   // If the camera-left key is pressed, move camera left.
   // If the camera-right key is pressed, move camera right.

   NodePath cameraNp = m_windowFrameworkPtr->get_camera_group();
   cameraNp.look_at(m_ralphNp);
   if(m_keyMap["cam-left"] != false)
      {
      cameraNp.set_x(cameraNp, -20 * ClockObject::get_global_clock()->get_dt());
      }
   if(m_keyMap["cam-right"] != false)
      {
      cameraNp.set_x(cameraNp, +20 * ClockObject::get_global_clock()->get_dt());
      }

   // save ralph's initial position so that we can restore it,
   // in case he falls off the map or runs into something.

   LPoint3f startPos = m_ralphNp.get_pos();

   // If a move-key is pressed, move ralph in the specified direction.

   if(m_keyMap["left"])
      {
      m_ralphNp.set_h(m_ralphNp.get_h() + 300 * ClockObject::get_global_clock()->get_dt());
      }
   if(m_keyMap["right"])
      {
      m_ralphNp.set_h(m_ralphNp.get_h() - 300 * ClockObject::get_global_clock()->get_dt());
      }
   if(m_keyMap["forward"])
      {
      m_ralphNp.set_y(m_ralphNp, -25 * ClockObject::get_global_clock()->get_dt());
      }

   // If ralph is moving, loop the run animation.
   // If he is standing still, stop the animation.

   if(m_keyMap["forward"] || m_keyMap["left"] || m_keyMap["right"])
      {
       if(!m_isMoving)
          {
          m_animControlCollection.loop("run", true);
          m_isMoving = true;
          }
      }
   else
      {
      if(m_isMoving)
         {
         m_animControlCollection.stop("run");
         m_animControlCollection.pose("walk", 5);
         m_isMoving = false;
         }
      }

   // If the camera is too far from ralph, move it closer.
   // If the camera is too close to ralph, move it farther.

   LPoint3f camVec = m_ralphNp.get_pos() - cameraNp.get_pos();
   camVec.set_z(0);
   float camDist = camVec.length();
   camVec.normalize();
   if(camDist > 10.0)
      {
      cameraNp.set_pos(cameraNp.get_pos() + camVec*(camDist-10));
      camDist = 10.0;
      }
   if(camDist < 5.0)
      {
      cameraNp.set_pos(cameraNp.get_pos() - camVec*(5-camDist));
      camDist = 5.0;
      }

   // Now check for collisions.

   NodePath renderNp = m_windowFrameworkPtr->get_render();
   m_collisionTraverser.traverse(renderNp);

   // Adjust ralph's Z coordinate.  If ralph's ray hit terrain,
   // update his Z. If it hit anything else, or didn't hit anything, put
   // him back where he was last frame.

   m_ralphGroundHandlerPtr->sort_entries();
   if(m_ralphGroundHandlerPtr->get_num_entries() > 0 &&
      m_ralphGroundHandlerPtr->get_entry(0)->get_into_node()->get_name() == "terrain")
      {
      m_ralphNp.set_z(m_ralphGroundHandlerPtr->get_entry(0)->get_surface_point(renderNp).get_z());
      }
   else
      {
      m_ralphNp.set_pos(startPos);
      }

   // Keep the camera at one foot above the terrain,
   // or two feet above ralph, whichever is greater.

   m_camGroundHandlerPtr->sort_entries();
   if(m_camGroundHandlerPtr->get_num_entries() > 0 &&
      m_camGroundHandlerPtr->get_entry(0)->get_into_node()->get_name() == "terrain")
      {
      cameraNp.set_z(m_camGroundHandlerPtr->get_entry(0)->get_surface_point(renderNp).get_z()+1.0);
      }
   if(cameraNp.get_z() < m_ralphNp.get_z() + 2.0)
      {
      cameraNp.set_z(m_ralphNp.get_z() + 2.0);
      }

   // The camera should look in ralph's direction,
   // but it should also try to stay horizontal, so look at
   // a floater which hovers above ralph's head.

   m_floaterNp.set_pos(m_ralphNp.get_pos());
   m_floaterNp.set_z(m_ralphNp.get_z() + 2.0);
   cameraNp.look_at(m_floaterNp);
   }

// Note: OnscreenText is a python only function. It's capabilities are emulated here
//       to simplify the translation to C++.
NodePath World::onscreen_text(const string& text, const Colorf& fg, const LPoint2f& pos, Alignment align, float scale) const
   {
   NodePath textNodeNp;

   if(m_windowFrameworkPtr != NULL)
      {
      PT(TextNode) textNodePtr = new TextNode("OnscreenText");
      if(textNodePtr != NULL)
         {
         textNodePtr->set_text(text);
         textNodePtr->set_text_color(fg);
         textNodePtr->set_align(static_cast<TextNode::Alignment>(align));
         textNodeNp = m_windowFrameworkPtr->get_aspect_2d().attach_new_node(textNodePtr);
         textNodeNp.set_pos(pos.get_x(), 0, pos.get_y());
         textNodeNp.set_scale(scale);
         }
      }

   return textNodeNp;
   }

// Note: this is a way to customize the names of the AnimControl(s) collected into an AnimControlCollection by auto_bind().
//       actorNp            : the actor that we wish to animate.
//       controls           : the collection of controls returned, just like auto_bind() would do.
//       animMap            : a map of the desired names (first) and their associated filenames (second).
//       hierarchyMatchFlags: idem as the same parameter from auto_bind().
//       This function should be called only once per actor.
void World::auto_bind_named(NodePath actorNp, AnimControlCollection &controls, const map<string,string>& animMap, int hierarchyMatchFlags /*= 0*/)
   {
   vector<NodePath> anims;
   anims.reserve(animMap.size());

   // for each animations we are asked to add to the actor
   for(map<string,string>::const_iterator i = animMap.begin(); i != animMap.end(); i++)
      {
      // load the animation as a child of the actor
      NodePath animNp = m_windowFrameworkPtr->load_model(actorNp, i->second);
      // collect the animation in a temporary collection
      AnimControlCollection tempCollection;
      auto_bind(actorNp.node(), tempCollection, hierarchyMatchFlags);
      // we should have collected a single animation
      if(tempCollection.get_num_anims() == 1)
         {
         // store the animation in the user's collection
         controls.store_anim(tempCollection.get_anim(0), i->first);
         // detach the node so that it will not appear in a new call to auto_bind()
         animNp.detach_node();
         // keep it on the side
         anims.push_back(animNp);
         }
      else
         {
         // something is wrong
         nout << "WARNING: could not bind animation `" << i->first << "' from file `" << i->second << "'." << endl;
         }
      }

   // re-attach the animation nodes to the actor
   for(vector<NodePath>::iterator i = anims.begin(); i < anims.end(); ++i)
      {
      i->reparent_to(actorNp);
      }
   }

void World::sys_exit(const Event* eventPtr, void* dataPtr)
   {
   exit(0);
   }

void World::set_key_left(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void World::set_key_left(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->set_key("left", true);
   }

void World::set_key_right(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void World::set_key_right(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->set_key("right", true);
   }

void World::set_key_forward(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void World::set_key_forward(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->set_key("forward", true);
   }

void World::set_key_cam_left(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void World::set_key_cam_left(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->set_key("cam-left", true);
   }

void World::set_key_cam_right(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void World::set_key_cam_right(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->set_key("cam-right", true);
   }

void World::unset_key_left(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void World::unset_key_left(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->set_key("left", false);
   }

void World::unset_key_right(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void World::unset_key_right(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->set_key("right", false);
   }

void World::unset_key_forward(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void World::unset_key_forward(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->set_key("forward", false);
   }

void World::unset_key_cam_left(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void World::unset_key_cam_left(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->set_key("cam-left", false);
   }

void World::unset_key_cam_right(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void World::unset_key_cam_right(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->set_key("cam-right", false);
   }

AsyncTask::DoneStatus World::call_move(GenericAsyncTask* taskPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: AsyncTask::DoneStatus World::call_move(GenericAsyncTask* taskPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return AsyncTask::DS_done;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->move();
   return AsyncTask::DS_cont;
   }
