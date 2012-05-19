/*
 * world.cpp
 *
 *  Created on: 2012-05-17
 *      Author: dri
 */

#include "pandaFramework.h"
#include "texturePool.h"
#include "world.h"

// This helps reduce the amount of code used by loading objects, since all of the
// objects are pretty much the same.
NodePath World::load_object(const string& tex   /* = "" */,
                            const LPoint2f& pos /* = LPoint2f(0,0) */,
                            float depth         /* = SPRITE_POS */,
                            float scale         /* = 1 */,
                            bool transparency   /* = true*/) const
   {
   NodePath modelsNp = m_windowFrameworkPtr->get_panda_framework()->get_models();
   NodePath objNp = m_windowFrameworkPtr->load_model(modelsNp, "../models/plane");

   // Everything is parented to the camera so
   // that it faces the screen
   NodePath cameraNp = m_windowFrameworkPtr->get_camera_group();
   objNp.reparent_to(cameraNp);

   // Set initial position
   objNp.set_pos(LPoint3f(pos.get_x(), depth, pos.get_y()));

   // Set initial scale
   objNp.set_scale(scale);

   // This tells Panda not to worry about the
   // order this is drawn in. (it prevents an
   // effect known as z-fighting)
   objNp.set_bin("unsorted", 0);

   // Tells panda not to check if something
   // has already drawn in front of it
   // (Everything in this game is at the same
   // depth anyway)
   objNp.set_depth_test(false);

   // All of our objects are transparent
   if(transparency)
      {
      objNp.set_transparency( transparency ? TransparencyAttrib::M_alpha
                                           : TransparencyAttrib::M_none);
      }

   if(tex != "")
      {
      // Load the texture
      PT(Texture) texturePtr = TexturePool::load_texture("../textures/"+tex+".png");
      // Set the texture
      objNp.set_texture(texturePtr, 1);
      }

   return objNp;
   }

// Function used to reduce the amount to code needed to create the
// on screen instructions
NodePath World::gen_label_text(const string& text, int i) const
   {
   return onscreen_text(text, Colorf(1, 1, 0, 1), LPoint2f(-1.3, 0.95-0.05*i), A_left, 0.05);
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

   // Note: initialize random seed
   srand(time(NULL));

   // This code puts the standard title and instruction text on screen
   m_titleNp = onscreen_text("Panda3D: Tutorial - Tasks",
                             Colorf(1, 1, 0, 1),
                             LPoint2f(0.8, -0.95),
                             A_center,
                             0.07);
   m_escapeTextNp   = gen_label_text("ESC: Quit", 0);
   m_leftKeyTextNp  = gen_label_text("[Left Arrow]: Turn Left (CCW)", 1);
   m_rightKeyTextNp = gen_label_text("[Right Arrow]: Turn Right (CW)", 2);
   m_upKeyTextNp    = gen_label_text("[Up Arrow]: Accelerate", 3);
   m_spaceKeyTextNp = gen_label_text("[Space Bar]: Fire", 4);

   // Disable default mouse-based camera control
   // Note: no need to do it in C++

   // Load the background starfield
   m_bgNp = load_object("stars", LPoint2f(0,0), 200, 146, false);

   // Load the ship
   m_shipNp = load_object("ship");

   // Initial velocity
   set_velocity(m_shipNp, LVecBase3f(0,0,0));

   // A dictionary of what keys are currently being pressed
   // The key events update this list, and our task will query it as input
   m_keys["turnLeft" ] = false;
   m_keys["turnRight"] = false;
   m_keys["accel"    ] = false;
   m_keys["fire"     ] = false;

   m_windowFrameworkPtr->enable_keyboard();
   // Escape quits
   m_windowFrameworkPtr->get_panda_framework()->define_key("escape",         "sysExit",     sys_exit,         NULL);
   // Other keys events set the appropriate value in our key dictionary
   m_windowFrameworkPtr->get_panda_framework()->define_key("arrow_left",     "turnLeft",    set_turn_left,    this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("arrow_left-up",  "turnLeftUp",  unset_turn_left,  this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("arrow_right",    "turnRight",   set_turn_right,   this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("arrow_right-up", "turnRightUp", unset_turn_right, this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("arrow_up",       "accel",       set_accel,        this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("arrow_up-up",    "accelUp",     unset_accel,      this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("space",          "fire",        set_fire,         this);

   // Now we create the task. taskMgr is the task manager that actually calls
   // The function each frame. The add method creates a new task. The first
   // argument is the function to be called, and the second argument is the name
   // for the task. It returns a task object, that is passed to the function
   // each frame
   m_gameTaskPtr = new GenericAsyncTask("gameLoop", call_game_loop, this);
   if(m_gameTaskPtr != NULL)
      {
      AsyncTaskManager::get_global_ptr()->add(m_gameTaskPtr);
      }

   // The task object is a good place to put variables that should stay
   // persistent for the task function from frame to frame
   // Note: this is a python only thing, let's use the World object then
   //       to store this data.

   // Task time of the last frame
   m_last = 0;
   // Task time when the next bullet may be fired
   m_nextBullet = 0;

   // This empty list will contain fired bullets
   // Note: irrelevant in this C++ implementation

   // Complete initialization by spawning the asteroids
   spawn_asteroids();
   }

// As described earlier, this simply sets a key in the m_keys dictionary to
// the given value
void World::set_key(const string& key, bool val)
   {
   m_keys[key] = val;
   }

//////////////////////////////////////////////////////////////////////////////////////////
//
//  This version, using 'setTag', runs fine.
//
//////////////////////////////////////////////////////////////////////////////////////////

void World::set_velocity(NodePath objNp, const LVecBase3f& val)
   {
   ostringstream list;
   list << val.get_x() << " " << val.get_y() << " " << val.get_z();
   objNp.set_tag("velocity", list.str());
   }

LVecBase3f World::get_velocity(NodePath objNp) const
   {
   string velocity = objNp.get_tag("velocity");
   istringstream list(velocity);
   float x = 0;
   float y = 0;
   float z = 0;
   list >> x >> y >> z;
   return LVecBase3f(x, y, z);
   }

void World::set_expires(NodePath objNp, float val)
   {
   ostringstream expires;
   expires << val;
   objNp.set_tag("expires", expires.str());
   }

float World::get_expires(NodePath objNp) const
   {
   string value = objNp.get_tag("expires");
   istringstream expires(value);
   float val = 0;
   expires >> val;
   return val;
   }

//////////////////////////////////////////////////////////////////////////////////////////
//
// This version, using 'setPythonTag', crashes.
//
//////////////////////////////////////////////////////////////////////////////////////////
//
//  def setVelocity(self, obj, val):
//    obj.setPythonTag("velocity", val)
//
//  def getVelocity(self, obj):
//    return obj.getPythonTag("velocity")
//
//  def setExpires(self, obj, val):
//    obj.setPythonTag("expires", val)
//
//  def getExpires(self, obj):
//    return obj.getPythonTag("expires")
//

void World::spawn_asteroids()
   {
   // Control variable for if the ship is alive
   m_alive = true;
   // List that will contain our asteroids
   for(int i = 0; i < AST_INIT_NB; ++i)
      {
      // This loop loads an asteroid.
      // The texture chosen is random from "asteroid1" to "asteroid3"
      ostringstream asteroidFilename;
      asteroidFilename << "asteroid" << rand() % AST_TEX_NB + 1;
      // This is kind of a hack, but it keeps the asteroids from spawning near
      // the player. It creates the list (-20, -19 ... -5, 5, 6, 7, ... 20)
      // and chooses a value from it. Since the player starts at 0 and this list
      // doesn't contain anything from -4 to 4, it won't be close to the player
      vector<int> rangeX;
      rangeX.reserve( 2*(SCREEN_X - AST_MIN_DIST + 1) );
      for(int i = -SCREEN_X   ; i <= -AST_MIN_DIST; ++i) { rangeX.push_back(i); }
      for(int i = AST_MIN_DIST; i <= SCREEN_X     ; ++i) { rangeX.push_back(i); }
      float x = rangeX[rand() % rangeX.size()];
      // Same thing for Y, but from -15 to 15
      vector<int> rangeY;
      rangeY.reserve( 2*(SCREEN_Y - AST_MIN_DIST + 1) );
      for(int i = -SCREEN_Y   ; i <= -AST_MIN_DIST; ++i) { rangeY.push_back(i); }
      for(int i = AST_MIN_DIST; i <= SCREEN_Y     ; ++i) { rangeY.push_back(i); }
      float y = rangeY[rand() % rangeY.size()];
      NodePath asteroidNp = load_object(asteroidFilename.str(),
                                        LPoint2f(x,y),
                                        SPRITE_POS,
                                        AST_INIT_SCALE);
      // Heading is a random angle in radians
      double heading = double(rand()) / double(RAND_MAX) * 2*PI;
      // Converts the heading to a vector and multiplies it by speed to get a
      // velocity vector
      LVecBase3f v = LVecBase3f(sin(heading), 0, cos(heading)) * AST_INIT_VEL;
      set_velocity(asteroidNp, v);
      m_asteroids.push_back(asteroidNp);
      }
   }

// This is our main task function, which does all of the per-frame processing
// It takes in self like all functions in a class, and task, the task object
// returned by taskMgr
// Note: in this implementation, it is caller indirectly by its wrapper call_game_loop()
AsyncTask::DoneStatus World::game_loop(GenericAsyncTask* taskPtr)
   {
   // task contains a variable time, which is the time in seconds the task has
   // been running. By default, it does not have a delta time (or dt), which is
   // the amount of time elapsed from the last frame. A common way to do this is
   // to store the current time in task.last. This can be used to find dt
   // Note: AsyncTask::get_dt() do exists
   double time = taskPtr->get_elapsed_time();
   double dt = time - m_last;
   m_last = time;

   // If the ship is not alive, do nothing. Tasks return Task.cont to signify
   // that the task should continue running. If Task.done were returned instead,
   // the task would be removed and would no longer be called every frame
   if(!m_alive) { return AsyncTask::DS_cont; }

   // update ship position
   update_ship(dt);

   // check to see if the ship can fire
   if(m_keys["fire"] && time > m_nextBullet)
      {
      // If so, call the fire function
      fire(time);
      // And disable firing for a bit
      m_nextBullet = time + BULLET_REPEAT;
      }
   // Remove the fire flag until the next spacebar press
   m_keys["fire"] = false;

   // update asteroids
   list<NodePath>::iterator objPtr;
   for(objPtr = m_asteroids.begin(); objPtr != m_asteroids.end(); ++objPtr)
      {
      update_pos(*objPtr, dt);
      }

   // update bullets
   objPtr = m_bullets.begin();
   while(objPtr != m_bullets.end())
      {
      NodePath objNp = *objPtr;
      list<NodePath>::iterator objToErasePtr = objPtr++;

      // Update the bullet
      update_pos(objNp, dt);
      // Bullets have an expiration time (see definition of fire)
      // If a bullet has not expired, add it to the new bullet list so that it
      // will continue to exist
      if(get_expires(objNp) > time)
         {
         // Note: nothing to do, go verify the next bullet.
         }
      else
         {
         // Otherwise remove it from the scene
         objNp.remove_node();
         m_bullets.erase(objToErasePtr);
         }
      }
   // Set the bullet array to be the newly updated array
   // Note: irrelevant in this implementation

   // Check bullet collision with asteroids
   // In short, it checks every bullet against every asteroid. This is quite
   // slow. An big optimization would be to sort the objects left to right and
   // check only if they overlap. Framerate can go way down if there are too
   // many bullets on screen, but for the most part it's okay.
   for(list<NodePath>::iterator bulletPtr = m_bullets.begin(); bulletPtr != m_bullets.end(); ++bulletPtr)
      {
      NodePath bulletNp = *bulletPtr;
      // This range statement makes it step though the asteroid list backwards
      // This is because if an asteroid is removed, the elements after it
      // will change position in the list. If you go backwards, the
      // length stays constant
      list<NodePath>::iterator astPtr = m_asteroids.begin();
      while(astPtr != m_asteroids.end())
         {
         NodePath astNp = *astPtr;
         list<NodePath>::iterator astToHitPtr = astPtr++;

         // Panda's collision detection is more complicated than we need here.
         // This is the basic sphere collision check. If the distance between
         // the object centers is less than sum of the radii of the two objects,
         // then we have a collision. We use lengthSquared since it is a quicker
         // vector operation than length
         if(((bulletNp.get_pos() - astNp.get_pos()).length_squared() <
            pow(((bulletNp.get_scale().get_x() + astNp.get_scale().get_x())
              * .5 ), 2)))
            {
            // Schedule the bullet for removal
            set_expires(bulletNp, 0);
            // Handle the hit
            asteroid_hit(astToHitPtr);
            }
         }
      }

   // Now we do the same collision pass for the ship
   for(list<NodePath>::iterator astPtr = m_asteroids.begin(); astPtr != m_asteroids.end(); ++astPtr)
      {
      // Same sphere collision check for the ship vs. the asteroid
      if(((m_shipNp.get_pos() - astPtr->get_pos()).length_squared() <
         pow(((m_shipNp.get_scale().get_x() + astPtr->get_scale().get_x()) * .5), 2)))
         {
         // If there is a hit, clear the screen and schedule a restart
         // Ship is no longer alive
         m_alive = false;
         // Remove every object in asteroids and bullets from the scene
         list<NodePath>::iterator i;
         for(i = m_asteroids.begin(); i != m_asteroids.end(); ++i) { i->remove_node(); }
         for(i = m_bullets.begin(); i != m_bullets.end(); ++i) { i->remove_node(); }
         // Clear the bullet list
         m_bullets.clear();
         // Note: and the asteroid's
         m_asteroids.clear();
         // Hide the ship
         m_shipNp.hide();
         // Reset the velocity
         set_velocity(m_shipNp, LVecBase3f(0,0,0));

         // Note: Sequence is a python only class
         //       Replace it with a delayed task
         PT(GenericAsyncTask) taskPtr = new GenericAsyncTask("restart", call_restart, this);
         if(taskPtr != NULL)
            {
            // Wait 2 seconds
            taskPtr->set_delay(2);
            AsyncTaskManager::get_global_ptr()->add(taskPtr);
            }
         return AsyncTask::DS_cont;
         }
      }

   // If the player has successfully destroyed all asteroids, respawn them
   if(m_asteroids.size() == 0) { spawn_asteroids(); }

   // Since every return is Task.cont, the task will
   // continue indefinitely
   return AsyncTask::DS_cont;
   }

// Updates the positions of objects
void World::update_pos(NodePath objNp, double dt)
   {
   LVecBase3f vel = get_velocity(objNp);
   LPoint3f newPos = objNp.get_pos() + (vel*dt);

   // Check if the object is out of bounds. If so, wrap it
   float radius = 0.5 * objNp.get_scale().get_x();
   if(newPos.get_x() - radius      >  SCREEN_X) { newPos.set_x(-SCREEN_X); }
   else if(newPos.get_x() + radius < -SCREEN_X) { newPos.set_x(SCREEN_X) ; }
   if(newPos.get_z() - radius      >  SCREEN_Y) { newPos.set_z(-SCREEN_Y); }
   else if(newPos.get_z() + radius < -SCREEN_Y) { newPos.set_z(SCREEN_Y) ; }

   objNp.set_pos(newPos);
   }

  // The handler when an asteroid is hit by a bullet
void World::asteroid_hit(list<NodePath>::iterator astPtr)
   {
   // If the asteroid is small it is simply removed
   if(astPtr->get_scale().get_x() <= AST_MIN_SCALE)
      {
      astPtr->remove_node();
      // This uses a Python feature called slices. Basically it's saying
      // Make the list the current list up to index plus the rest of the list
      // after index
      // This has the effect of removing the object at index
      // Note: in this implementation, we just erase it
      m_asteroids.erase(astPtr);
      }
   else
      {
      // If it is big enough, split it instead
      // First we update the current asteroid
      float newScale = astPtr->get_scale().get_x() * AST_SIZE_SCALE;
      // Rescale it
      astPtr->set_scale(newScale);

      // The new direction is chosen as perpendicular to the old direction
      // This is determined using the cross product, which returns a vector
      // perpendicular to the two input vectors. By crossing velocity with a
      // vector that goes into the screen, we get a vector that is perpendicular
      // to the original velocity in the plane of the screen
      LVecBase3f vel = get_velocity(*astPtr);
      float speed = vel.length() * AST_VEL_SCALE;
      vel.normalize();
      vel = LVecBase3f(0,1,0).cross(vel);
      vel *= speed;
      set_velocity(*astPtr, vel);

      // Now we create a new asteroid identical to the current one
      LPoint2f pos(astPtr->get_pos().get_x(), astPtr->get_pos().get_z());
      NodePath newAstNp = load_object("", pos, SPRITE_POS, newScale);
      set_velocity(newAstNp, vel * -1);
      newAstNp.set_texture(astPtr->get_texture(), 1);
      m_asteroids.push_back(newAstNp);
      }
   }

  // This updates the ship's position. This is similar to the general update
  // but takes into account turn and thrust
void World::update_ship(double dt)
   {
   // Heading is the roll value for this model
   float heading = m_shipNp.get_r();
   // Change heading if left or right is being pressed
   if(m_keys["turnRight"])
      {
      heading += dt * TURN_RATE;
      while(heading > 360) { heading -= 360; }
      m_shipNp.set_r(heading);
      }
   else if(m_keys["turnLeft"])
      {
      heading -= dt * TURN_RATE;
      while(heading < 0) { heading += 360; }
      m_shipNp.set_r(heading);
      }

   // Thrust causes acceleration in the direction the ship is currently facing
   if(m_keys["accel"])
      {
      float heading_rad = DEG_TO_RAD * heading;
      // This builds a new velocity vector and adds it to the current one
      // Relative to the camera, the screen in Panda is the XZ plane.
      // Therefore all of our Y values in our velocities are 0 to signify no
      // change in that direction
      LVecBase3f newVel =
            LVecBase3f(sin(heading_rad), 0, cos(heading_rad)) * ACCELERATION * dt;
      newVel += get_velocity(m_shipNp);
      // Clamps the new velocity to the maximum speed. lengthSquared() is used
      // again since it is faster than length()
      if(newVel.length_squared() > MAX_VEL_SQ)
         {
         newVel.normalize();
         newVel *= MAX_VEL;
         }
      set_velocity(m_shipNp, newVel);
      }

   // Finally, update the position as with any other object
   update_pos(m_shipNp, dt);
   }

// Creates a bullet and adds it to the bullet list
void World::fire(double time)
   {
   float direction = DEG_TO_RAD * m_shipNp.get_r();
   LPoint2f pos(m_shipNp.get_pos().get_x(), m_shipNp.get_pos().get_z());
   // Create the object
   NodePath bulletNp = load_object("bullet", pos, SPRITE_POS, 0.2);
   // Velocity is in relation to the ship
   LVecBase3f vel = (get_velocity(m_shipNp) +
                     (LVecBase3f(sin(direction), 0, cos(direction)) *
                      BULLET_SPEED));
   set_velocity(bulletNp, vel);
   // Set the bullet expiration time to be a certain amount past the current time
   set_expires(bulletNp, time + BULLET_LIFE);

   // Finally, add the new bullet to the list
   m_bullets.push_back(bulletNp);
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

void World::sys_exit(const Event* eventPtr, void* dataPtr)
   {
   exit(0);
   }

void World::set_turn_left(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void World::set_turn_left(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->set_key("turnLeft", true);
   }

void World::unset_turn_left(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void World::unset_turn_left(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->set_key("turnLeft", false);
   }

void World::set_turn_right(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void World::set_turn_right(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->set_key("turnRight", true);
   }

void World::unset_turn_right(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void World::unset_turn_right(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->set_key("turnRight", false);
   }

void World::set_accel(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void World::set_accel(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->set_key("accel", true);
   }

void World::unset_accel(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void World::unset_accel(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->set_key("accel", false);
   }

void World::set_fire(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void World::set_fire(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->set_key("fire", true);
   }

AsyncTask::DoneStatus World::call_game_loop(GenericAsyncTask* taskPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: AsyncTask::DoneStatus World::call_game_loop(GenericAsyncTask* taskPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return AsyncTask::DS_done;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   return worldPtr->game_loop(taskPtr);
   }

AsyncTask::DoneStatus World::call_restart(GenericAsyncTask* taskPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: AsyncTask::DoneStatus World::call_restart(GenericAsyncTask* taskPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return AsyncTask::DS_done;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->restart();
   return AsyncTask::DS_done;
   }

void World::restart()
   {
   // Reset heading
   m_shipNp.set_r(0);
   // Reset position X
   m_shipNp.set_x(0);
   // Reset position Y (Z for Panda)
   m_shipNp.set_z(0);
   // Show the ship
   m_shipNp.show();
   // And respawn the asteroids
   spawn_asteroids();
   }
