/*
 * world.cpp
 *
 *  Created on: 2012-05-22
 *      Author: dri
 */

#include "../p3util/cOnscreenText.h"
#include "../p3util/cActorInterval.h"
#include "../p3util/genericFunctionInterval.h"
#include "directionalLight.h"
#include "ambientLight.h"
#include "cIntervalManager.h"
#include "waitInterval.h"
#include "world.h"

// Macro-like function used to reduce the amount to code needed to create the
// on screen instructions
NodePath World::gen_label_text(const string& text, int i) const
   {
   COnscreenText label("label");
   label.set_text(text);
   label.set_pos(LVecBase2f(-1.3, 0.95-0.05*i));
   label.set_fg(Colorf(1,1,1,1));
   label.set_align(TextNode::A_left);
   label.set_scale(0.05);
   label.reparent_to(m_windowFrameworkPtr->get_aspect_2d());
   return label.generate();
   }

World::World(WindowFramework* windowFrameworkPtr)
   : m_windowFrameworkPtr(windowFrameworkPtr)
   {
   // preconditions
   if(m_windowFrameworkPtr == NULL)
      {
      nout << "ERROR: parameter windowFrameworkPtr cannot be NULL." << endl;
      return;
      }

   // This code puts the standard title and instruction text on screen
   COnscreenText title("title", COnscreenText::TS_plain);
   title.set_text("Panda3D: Tutorial - Actors");
   title.set_fg(Colorf(0,0,0,1));
   title.set_pos(LVecBase2f(0.8,-0.95));
   title.set_scale(0.07);
   title.reparent_to(m_windowFrameworkPtr->get_aspect_2d());
   m_titleNp = title.generate();
   m_escapeEventTextNp = gen_label_text("ESC: Quit", 0);
   m_aKeyEventTextNp = gen_label_text("[A]: Robot 1 Left Punch" , 1);
   m_sKeyEventTextNp = gen_label_text("[S]: Robot 1 Right Punch", 2);
   m_kKeyEventTextNp = gen_label_text("[K]: Robot 2 Left Punch" , 3);
   m_lKeyEventTextNp = gen_label_text("[L]: Robot 2 Right Punch", 4);

   // Set the camera in a fixed position
   // Note: no need to disable the mouse in C++
   NodePath cameraNp = m_windowFrameworkPtr->get_camera_group();
   cameraNp.set_pos_hpr(14.5, -15.4, 14, 45, -14, 0);
   m_windowFrameworkPtr->set_background_type(WindowFramework::BT_black);

   // Add lighting so that the objects are not drawn flat
   setup_lights();

   // Load the ring
   NodePath modelsNp = m_windowFrameworkPtr->get_panda_framework()->get_models();
   m_ringNp = m_windowFrameworkPtr->load_model(modelsNp, "../models/ring");
   NodePath renderNp = m_windowFrameworkPtr->get_render();
   m_ringNp.reparent_to(renderNp);

   // Models that use skeletal animation are known as Actors instead of models
   // Instead of just one file, the have one file for the main model, and an
   // additional file for each playable animation.
   // They are loaded using Actor.Actor instead of loader.LoadModel.
   // The constructor takes the location of the main object as with a normal model
   // and a dictionary (A fancy python structure that is like a lookup table)
   // that contains names for animations, and paths to the appropriate files
   CActor::AnimMap robotAnims;
   robotAnims["../models/robot_left_punch" ].push_back("leftPunch" );
   robotAnims["../models/robot_right_punch"].push_back("rightPunch");
   robotAnims["../models/robot_head_up"    ].push_back("headUp"    );
   robotAnims["../models/robot_head_down"  ].push_back("headDown"  );
   m_robot1.load_actor(m_windowFrameworkPtr,
                       "../models/robot",
                       &robotAnims,
                       PartGroup::HMF_ok_part_extra |
                       PartGroup::HMF_ok_anim_extra |
                       PartGroup::HMF_ok_wrong_root_name);

   // Actors need to be positioned and parented like normal objects
   m_robot1.set_pos_hpr_scale(  -1, -2.5,    4,
                                45,    0,    0,
                              1.25, 1.25, 1.25);
   m_robot1.reparent_to(renderNp);

   // We'll repeat the process for the second robot. The only thing that changes
   // here is the robot's color and position
   m_robot2.load_actor(m_windowFrameworkPtr,
                       "../models/robot",
                       &robotAnims,
                       PartGroup::HMF_ok_part_extra |
                       PartGroup::HMF_ok_anim_extra |
                       PartGroup::HMF_ok_wrong_root_name);

   // Set the properties of this robot
   m_robot2.set_pos_hpr_scale(   1,  1.5,    4,
                               225,    0,    0,
                              1.25, 1.25, 1.25);
   m_robot2.set_color(Colorf(0.7, 0, 0, 1));
   m_robot2.reparent_to(renderNp);

   // Now we define how the animated models will move. Animations are played
   // through special intervals. In this case we use actor intervals in a
   // sequence to play the part of the punch animation where the arm extends,
   // call a function to check if the punch landed, and then play the part of the
   // animation where the arm retracts

   // Punch sequence for robot 1's left arm
   m_robot1.m_punchLeftPtr = new CMetaInterval("robot1PunchLeft");
   if(m_robot1.m_punchLeftPtr != NULL)
      {
      // Interval for the outstreched animation
      m_robot1.m_punchLeftPtr->add_c_interval(new CActorInterval("robot1PunchLeftStretch", &m_robot1, "leftPunch", 1, 10));
      // Function to check if the punch was successful
      m_robot1.m_punchLeftPtr->add_c_interval(new GenericFunctionInterval("robot1PunchLeftCheck", call_check_punch<R_robot2>, this, true));
      // Interval for the retract animation
      m_robot1.m_punchLeftPtr->add_c_interval(new CActorInterval("robot1PunchLeftRetract", &m_robot1, "leftPunch", 11, 32));
      }

   // Punch sequence for robot 1's right arm
   m_robot1.m_punchRightPtr = new CMetaInterval("robot1PunchRight");
   if(m_robot1.m_punchRightPtr != NULL)
      {
      // Interval for the outstreched animation
      m_robot1.m_punchRightPtr->add_c_interval(new CActorInterval("robot1PunchRightStretch", &m_robot1, "rightPunch", 1, 10));
      // Function to check if the punch was successful
      m_robot1.m_punchRightPtr->add_c_interval(new GenericFunctionInterval("robot1PunchRightCheck", call_check_punch<R_robot2>, this, true));
      // Interval for the retract animation
      m_robot1.m_punchRightPtr->add_c_interval(new CActorInterval("robot1PunchRightRetract", &m_robot1, "rightPunch", 11, 32));
      }

   // Punch sequence for robot 2's left arm
   m_robot2.m_punchLeftPtr = new CMetaInterval("robot2PunchLeft");
   if(m_robot2.m_punchLeftPtr != NULL)
      {
      // Interval for the outstreched animation
      m_robot2.m_punchLeftPtr->add_c_interval(new CActorInterval("robot2PunchLeftStretch", &m_robot2, "leftPunch", 1, 10));
      // Function to check if the punch was successful
      m_robot2.m_punchLeftPtr->add_c_interval(new GenericFunctionInterval("robot2PunchLeftCheck", call_check_punch<R_robot1>, this, true));
      // Interval for the retract animation
      m_robot2.m_punchLeftPtr->add_c_interval(new CActorInterval("robot2PunchLeftRetract", &m_robot2, "leftPunch", 11, 32));
      }

   // Punch sequence for robot 2's right arm
   m_robot2.m_punchRightPtr = new CMetaInterval("robot2PunchRight");
   if(m_robot2.m_punchRightPtr != NULL)
      {
      // Interval for the outstreched animation
      m_robot2.m_punchRightPtr->add_c_interval(new CActorInterval("robot2PunchRightStretch", &m_robot2, "rightPunch", 1, 10));
      // Function to check if the punch was successful
      m_robot2.m_punchRightPtr->add_c_interval(new GenericFunctionInterval("robot2PunchRightCheck", call_check_punch<R_robot1>, this, true));
      // Interval for the retract animation
      m_robot2.m_punchRightPtr->add_c_interval(new CActorInterval("robot2PunchRightRetract", &m_robot2, "rightPunch", 11, 32));
      }

   // We use the same technique to create a sequence for when a robot is knocked
   // out where the head pops up, waits a while, and then resets

   // Head animation for robot 1
   m_robot1.m_resetHeadPtr = new CMetaInterval("robot1ResetHead");
   if(m_robot1.m_resetHeadPtr != NULL)
      {
      // Interval for the head going up. Since no start or end frames were given,
      // the entire animation is played.
      m_robot1.m_resetHeadPtr->add_c_interval(new CActorInterval("robot1ResetHeadUp", &m_robot1, "headUp"));
      m_robot1.m_resetHeadPtr->add_c_interval(new WaitInterval(3));
      // The head down animation was animated a little too quickly, so this will
      // play it at 75% of it's normal speed
      m_robot1.m_resetHeadPtr->add_c_interval(new CActorInterval("robot1ResetHeadDown", &m_robot1, "headDown", 0.75));
      }

   // Head animation for robot 2
   m_robot2.m_resetHeadPtr = new CMetaInterval("robot2ResetHead");
   if(m_robot2.m_resetHeadPtr != NULL)
      {
      // Interval for the head going up. Since no start or end frames were given,
      // the entire animation is played.
      m_robot2.m_resetHeadPtr->add_c_interval(new CActorInterval("robot2ResetHeadUp", &m_robot2, "headUp"));
      m_robot2.m_resetHeadPtr->add_c_interval(new WaitInterval(3));
      // The head down animation was animated a little too quickly, so this will
      // play it at 75% of it's normal speed
      m_robot2.m_resetHeadPtr->add_c_interval(new CActorInterval("robot2ResetHeadDown", &m_robot2, "headDown", 0.75));
      }

   // Note: setup a task to call CIntervalManager::step() at each frame to run the intervals
   AsyncTaskManager::get_global_ptr()->add(new GenericAsyncTask("intervalManagerTask", step_interval_manager, NULL));


   // Now that we have defined the motion, we can define our key input.
   // Each fist is bound to a key. When a key is pressed, self.tryPunch checks to
   // make sure that the both robots have their heads down, and if they do it
   // plays the given interval
   m_windowFrameworkPtr->enable_keyboard();
   m_windowFrameworkPtr->get_panda_framework()->define_key("escape", "sysExit"            , sys_exit                               , NULL);
   m_windowFrameworkPtr->get_panda_framework()->define_key("a"     , "robot1TryLeftPunch" , call_try_punch<R_robot1, P_left_punch >, this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("s"     , "robot1TryRightPunch", call_try_punch<R_robot1, P_right_punch>, this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("k"     , "robot2TryLeftPunch" , call_try_punch<R_robot2, P_left_punch >, this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("l"     , "robot2TryRightPunch", call_try_punch<R_robot2, P_right_punch>, this);

   // initialize RNG
   srand(time(NULL));
   }

// tryPunch will play the interval passed to it only if
// -neither robot has 'resetHead' playing (a head is up) AND
// -the punch interval passed to it is not already playing
void World::try_punch(RobotId robotId, PunchId punchId)
   {
   if(m_robot1.m_resetHeadPtr->is_playing() ||
      m_robot2.m_resetHeadPtr->is_playing() )
      {
      return;
      }

   Robot* robotPtr = NULL;
   switch(robotId)
      {
      case R_robot1:
         robotPtr = &m_robot1;
         break;
      case R_robot2:
         robotPtr = &m_robot2;
         break;
      default:
         nout << "ERROR: missing a RobotId switch case?" << endl;
         return;
      }

   PT(CMetaInterval) punchPtr = NULL;
   switch(punchId)
      {
      case P_left_punch:
         punchPtr = robotPtr->m_punchLeftPtr;
         break;
      case P_right_punch:
         punchPtr = robotPtr->m_punchRightPtr;
         break;
      default:
         nout << "ERROR: missing a PunchId switch case?" << endl;
         return;
      }

   if(!punchPtr->is_playing())
      {
      punchPtr->start();
      }
   }

// checkPunch will determine if a successful punch has been thrown
void World::check_punch(RobotId robotId)
   {
   if(robotId == R_robot1)
      {
      // punch is directed to robot 1
      // if robot 1 is playing'resetHead', do nothing
      if(m_robot1.m_resetHeadPtr->is_playing())
         {
         return;
         }
      // if robot 1 is not punching...
      if(!m_robot1.m_punchLeftPtr->is_playing()  &&
         !m_robot1.m_punchRightPtr->is_playing() )
         {
         // ...15% chance of successful hit
         if((double)rand()/(double)RAND_MAX > 0.85)
            {
            m_robot1.m_resetHeadPtr->start();
            }
         // Otherwise, only 5% chance of successful hit
         else if((double)rand()/(double)RAND_MAX > 0.95)
            {
            m_robot1.m_resetHeadPtr->start();
            }
         }
      }
   else
      {
      // punch is directed to robot 2, same as above
      if(m_robot2.m_resetHeadPtr->is_playing())
         {
         return;
         }
      if(!m_robot2.m_punchLeftPtr->is_playing()  &&
         !m_robot2.m_punchRightPtr->is_playing() )
         {
         if((double)rand()/(double)RAND_MAX > 0.85)
            {
            m_robot2.m_resetHeadPtr->start();
            }
         else if((double)rand()/(double)RAND_MAX > 0.95)
            {
            m_robot2.m_resetHeadPtr->start();
            }
         }
      }
   }

// This function sets up the lighting
void World::setup_lights()
   {
   NodePath renderNp = m_windowFrameworkPtr->get_render();
   PT(AmbientLight) ambientLightPtr = new AmbientLight("ambientLight");
   if(ambientLightPtr != NULL)
      {
      ambientLightPtr->set_color(Colorf(0.8, 0.8, 0.75, 1));
      renderNp.set_light(renderNp.attach_new_node(ambientLightPtr));
      }
   PT(DirectionalLight) directionalLightPtr = new DirectionalLight("directionalLight");
   if(directionalLightPtr != NULL)
      {
      directionalLightPtr->set_direction(LVecBase3f( 0, 0, -2.5 ));
      directionalLightPtr->set_color(Colorf( 0.9, 0.8, 0.9, 1 ));
      renderNp.set_light(renderNp.attach_new_node(directionalLightPtr));
      }
   }

AsyncTask::DoneStatus World::step_interval_manager(GenericAsyncTask *taskPtr, void *dataPtr)
   {
   CIntervalManager::get_global_ptr()->step();
   return AsyncTask::DS_cont;
   }

void World::sys_exit(const Event* eventPtr, void* dataPtr)
   {
   exit(0);
   }

template<int robotId, int punchId>
void World::call_try_punch(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<World*>(dataPtr)->try_punch((RobotId)robotId, (PunchId)punchId);
   }

template<int robotId>
void World::call_check_punch(void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<World*>(dataPtr)->check_punch((RobotId)robotId);
   }
