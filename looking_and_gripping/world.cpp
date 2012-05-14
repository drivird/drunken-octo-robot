/*
 * world.cpp
 *
 *  Created on: 2012-05-08
 *      Author: dri
 *
 * This is a python to C++ translation of Panda3d 1.7 sample/tutorial:
 * Tut-Looking-and-Gripping.py
 *
 */

#include "world.h"
#include "pandaFramework.h"
#include "auto_bind.h"
#include "character.h"
#include "modelNode.h"
#include "mouseWatcher.h"
#include "ambientLight.h"
#include "directionalLight.h"

World::World(WindowFramework* windowFrameworkPtr)
   : m_windowFrameworkPtr(windowFrameworkPtr)
   {
   // preconditions
   if(m_windowFrameworkPtr == NULL)
      {
      nout << "ERROR: World::World(WindowFramework* windowFrameworkPtr) parameter windowFrameworkPtr cannot be NULL." << endl;
      return;
      }

   // This code puts the standard title and instruction text on screen
   m_titleNp = onscreen_text("Panda3D: Tutorial - Joint Manipulation", Colorf(1,1,1,1), LPoint2f(0.7, -0.95), A_center, 0.07);
   m_esckeyTextNp   = gen_label_text("ESC: Quit"      , 0);
   m_onekeyTextNp   = gen_label_text("[1]: Teapot"    , 1);
   m_twokeyTextNp   = gen_label_text("[2]: Candy cane", 2);
   m_threekeyTextNp = gen_label_text("[3]: Banana"    , 3);
   m_fourkeyTextNp  = gen_label_text("[4]: Sword"     , 4);

   // setup key input
   m_windowFrameworkPtr->enable_keyboard();
   m_windowFrameworkPtr->get_panda_framework()->define_key("escape", "Exit"     , sys_exit      , NULL);
   m_windowFrameworkPtr->get_panda_framework()->define_key("1"     , "Teapot"   , set_teapot    , this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("2"     , "CandyCane", set_candy_cane, this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("3"     , "Banana"   , set_banana    , this);
   m_windowFrameworkPtr->get_panda_framework()->define_key("4"     , "Sword"    , set_sword     , this);

   // Disable mouse-based camera-control
   // Note: disable by default in C++

   // Position the camera
   m_windowFrameworkPtr->get_camera_group().set_pos(0,-15, 2);

   // Load our animated character
   m_eveNp = m_windowFrameworkPtr->load_model(m_windowFrameworkPtr->get_panda_framework()->get_models(), "../models/eve");

   // Load the character's walk animation
   // Note: File eve_walk.egg is broken!
   //       The name of the animation is case sensitive and at line 13 of file
   //       eve_walk.egg you should read `Eve' instead of `eve'. You need to
   //       correct this in order to bind the animation automatically using
   //       auto_bind() or WindowFramework::loop_animations().
   m_windowFrameworkPtr->load_model(m_eveNp, "../models/eve_walk");
   auto_bind(m_eveNp.node(), m_animControlCollection);

   // Put it in the scene
   m_eveNp.reparent_to(m_windowFrameworkPtr->get_render());

   // Now we use control_joint to get a NodePath that's in control of her neck
   // This must be done before any animations are played
   bool foundJoint = false;
   PT(CharacterJoint) characterJointPtr = NULL;
   string jointName("Neck");
   m_eveNeckNp = m_eveNp.attach_new_node(jointName);
   NodePath characterNP = m_eveNp.find("**/+Character");
   PT(Character) characterPtr = DCAST(Character, characterNP.node());
   if(characterPtr != NULL)
      {
      characterJointPtr = characterPtr->find_joint(jointName);
      if(characterJointPtr != NULL)
         {
         for(int i = 0; !foundJoint && i < characterPtr->get_num_bundles(); ++i)
            {
            if(characterPtr->get_bundle(i)->control_joint(jointName, m_eveNeckNp.node()))
               {
               foundJoint = true;
               m_eveNeckNp.set_mat(characterJointPtr->get_default_value());
               }
            }
         }
      }
   if(!foundJoint)
      {
      nout << "ERROR: cannot control joint `" << jointName << "'." << endl;
      m_eveNeckNp.remove_node();
      }

   // We now play an animation. An animation must be played, or at least posed
   // for the nodepath we just got from control_joint to actually effect the model
   // Note: the AnimControl's name is the one from the <Bundle> tag in the egg file,
   //       in this case `Eve'.
   m_animControlCollection.get_anim(0)->set_play_rate(2);
   m_animControlCollection.loop("Eve", true);

   // Now we add a task that will take care of turning the head
   AsyncTaskManager::get_global_ptr()->add(new GenericAsyncTask("turnHead", World::turn_head, this));

   // Now we will expose the joint the hand joint. ExposeJoint allows us to
   // get the position of a joint while it is animating. This is different than
   // control_joint which stops that joint from animating but lets us move it.
   // This is particularly useful for putting an object (like a weapon) in an
   // actor's hand
   // Note: ExposeJoint is a python only function.
   foundJoint = false;
   jointName = "RightHand";
   m_eveRightHandNp = m_eveNp.attach_new_node(jointName);
   if(characterPtr != NULL)
      {
      characterJointPtr = characterPtr->find_joint(jointName);
      if(characterJointPtr != NULL)
         {
         foundJoint = true;
         characterJointPtr->add_net_transform(m_eveRightHandNp.node());
         }
      }
   if(!foundJoint)
      {
      nout << "ERROR: no joint named `" << jointName << "'." << endl;
      m_eveRightHandNp.remove_node();
      }

   // This is a table with models, positions, rotations, and scales of objects to
   // be attached to our exposed joint. These are stock models and so they needed
   // to be repositioned to look right.
   vector<ModelData> positions(M_models);
   positions[M_teapot]     = ModelData("../models/teapot"   , LVecBase3f(0.00,-0.66,-0.95), LVecBase3f(90,  0,90), 0.40);
   positions[M_candy_cane] = ModelData("../models/candycane", LVecBase3f(0.15,-0.99,-0.22), LVecBase3f(90,  0,90), 1.00);
   positions[M_banana]     = ModelData("../models/banana"   , LVecBase3f(0.08,-0.10, 0.09), LVecBase3f( 0,-90, 0), 1.75);
   positions[M_sword]      = ModelData("../models/sword"    , LVecBase3f(0.11, 0.19, 0.06), LVecBase3f( 0,  0,90), 1.00);
   // A list that will store our models objects
   m_modelsNp.reserve(M_models);
   for(vector<ModelData>::iterator i = positions.begin(); i < positions.end(); ++i)
      {
      // Load the model
      NodePath np = m_windowFrameworkPtr->load_model(m_windowFrameworkPtr->get_panda_framework()->get_models(), i->m_filename);
      // Position it
      np.set_pos(i->m_pos);
      // Rotate it
      np.set_hpr(i->m_hpr);
      // Scale it
      np.set_scale(i->m_scale);
      // Reparent the model to the exposed joint. That way when the joint moves,
      // the model we just loaded will move with it.
      np.reparent_to(m_eveRightHandNp);
      // Add it to our models list
      m_modelsNp.push_back(np);
      }

   // Make object 0 the first shown
   set_object(M_teapot);
   // Put in some default lighting
   setup_lights();
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

// Function used to reduce the amount to code needed to create the
// on screen instructions
NodePath World::gen_label_text(const string& text, int i) const
   {
   return onscreen_text(text, Colorf(1,1,1,1), LPoint2f(-1.3, 0.95-0.06*i), A_left, 0.05);
   }

// This is what we use to change which object it being held. It just hides all of
// the objects and then unhides the one that was selected
void World::set_object(int i)
   {
   // preconditions
   if(i < 0 || (unsigned int)i >= m_modelsNp.size())
      {
      nout << "ERROR: void World::set_object(int i) parameter i is out of range (i=" << i << ")" << endl;
      return;
      }

   for(vector<NodePath>::iterator np = m_modelsNp.begin(); np < m_modelsNp.end(); ++np)
      {
      np->hide();
      }
   m_modelsNp[i].show();
   }

void World::sys_exit(const Event* eventPtr, void* dataPtr)
   {
   exit(0);
   }

void World::set_teapot(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void World::set_teapot(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->set_object(M_teapot);
   }

void World::set_candy_cane(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void World::set_candy_cane(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->set_object(M_candy_cane);
   }

void World::set_banana(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void World::set_banana(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->set_object(M_banana);
   }

void World::set_sword(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: void World::set_sword(const Event* eventPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   worldPtr->set_object(M_sword);
   }

// This task gets the position of mouse each frame, and rotates the neck based
// on it.
AsyncTask::DoneStatus World::turn_head(GenericAsyncTask* taskPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: static AsyncTask::DoneStatus World::turnHead(GenericAsyncTask* taskPtr, void* dataPtr) parameter dataPtr cannot be NULL." << endl;
      return AsyncTask::DS_done;
      }

   World* worldPtr = static_cast<World*>(dataPtr);
   // Check to make sure the mouse is readable
   PT(MouseWatcher) mouseWatcherPtr = DCAST(MouseWatcher, worldPtr->m_windowFrameworkPtr->get_mouse().node());
   if(mouseWatcherPtr->has_mouse())
      {
      // get the mouse position as a Vec2. The values for each axis are from -1 to
      // 1. The top-left is (-1,-1), the bottom right is (1,1)
      const LPoint2f& mpos = mouseWatcherPtr->get_mouse();
      // Here we multiply the values to get the amount of degrees to turn
      // Restrain is used to make sure the values returned by getMouse are in the
      // valid range. If this particular model were to turn more than this,
      // significant tearing would be visible
      worldPtr->m_eveNeckNp.set_p(restrain(mpos.get_x()) * 50);
      worldPtr->m_eveNeckNp.set_h(restrain(mpos.get_y()) * 20);
      }
   // Task continues infinitely
   return AsyncTask::DS_cont;
   }

// A simple function to make sure a value is in a given range, -1 to 1 by default
float World::restrain(float i, float mn /*= -1*/, float mx /*= 1*/)
   {
   return min(max(i, mn), mx);
   }

//Sets up some default lighting
void World::setup_lights() const
   {
   PT(AmbientLight) ambientLightPtr = new AmbientLight("ambientLight");
   PT(DirectionalLight) directionalLightPtr = new DirectionalLight("directionalLight");
   if(ambientLightPtr == NULL || directionalLightPtr == NULL)
      {
      nout << "ERROR: out of memory." << endl;
      return;
      }

   ambientLightPtr->set_color(Colorf(.4,.4,.35,1));
   directionalLightPtr->set_direction(LVector3f(0,8,-2.5));
   directionalLightPtr->set_color(Colorf(0.9,0.8,0.9,1));
   NodePath renderNp = m_windowFrameworkPtr->get_render();
   renderNp.set_light(renderNp.attach_new_node(directionalLightPtr));
   renderNp.set_light(renderNp.attach_new_node(ambientLightPtr));
   }
