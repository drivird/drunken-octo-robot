/*
 * cActor.cpp
 *
 *  Created on: 2012-05-24
 *      Author: dri
 */

#include "auto_bind.h"
#include "character.h"
#include "cActor.h"

CActor::CActor()
   {
   ;
   }

CActor::~CActor()
   {
   ;
   }


// This function loads an actor model and its animations, letting the user specify the name of each animation.
//
// Return value:
// The actor, parented to the framework's models node (to render the actor, reparent it to the render node)
//
// Parameters:
//    windowFrameworkPtr : the WindowFramework for this actor
//    actorFilename      : the actor's egg file
//    animMap            : a map of the desired name (first) of each animation and its associated file (second).
//                         Set this parameter to NULL if there are no animations to bind to the actor.
//    hierarchyMatchFlags: idem as the same parameter from auto_bind().
void CActor::load_actor(WindowFramework* windowFrameworkPtr,
                        const string& actorFilename,
                        const AnimMap* animMapPtr,
                        int hierarchyMatchFlags)
   {
   // preconditions
   if(windowFrameworkPtr == NULL)
      {
      nout << "ERROR: windowFrameworkPtr cannot be NULL." << endl;
      return;
      }
   if(!is_empty())
      {
      nout << "ERROR: cannot call load_actor() because this instance is not empty anymore." << endl;
      return;
      }

   // first load the actor model
   NodePath modelsNp = windowFrameworkPtr->get_panda_framework()->get_models();
   NodePath::operator=(windowFrameworkPtr->load_model(modelsNp, actorFilename));

   // Is there any animations to bind to the actor?
   if(animMapPtr == NULL) { return; }

   AnimControlCollection tempCollection;
   NodePathVec animsNp;
   animsNp.reserve(animMapPtr->size());

   // then for each animations specified by the user
   for(AnimMap::const_iterator i = animMapPtr->begin(); i != animMapPtr->end(); i++)
      {
      // load the animation as a child of the actor
      NodePath animNp = windowFrameworkPtr->load_model(*this, (*i).second);
      // collect the animation in a temporary collection
      auto_bind(this->node(), tempCollection, hierarchyMatchFlags);
      // we should have collected a single animation
      if(tempCollection.get_num_anims() == 1)
         {
         // store the animation in the user's collection
         store_anim(tempCollection.get_anim(0), (*i).first);
         // detach the node so that it will not appear in a new call to auto_bind()
         animNp.detach_node();
         // keep it on the side
         animsNp.push_back(animNp);
         }
      else
         {
         // something is wrong
         nout << "WARNING: could not bind animation `" << (*i).first << "' from file `" << (*i).second << "'." << endl;
         }
      tempCollection.clear_anims();
      }

   // re-attach the animation nodes to the actor
   for(NodePathVec::iterator np = animsNp.begin(); np < animsNp.end(); ++np)
      {
      (*np).reparent_to(*this);
      }
   }

// This function gives access to a joint that can be controlled via its NodePath handle.
// Think of it as a write to that joint interface.
//
// Return value:
// The joint's NodePath, parented to the actor.
//
// Parameter:
//    jointName: the joint's name as found in the model file.
NodePath CActor::control_joint(const string& jointName)
   {
   bool foundJoint = false;
   NodePath jointNp = attach_new_node(jointName);
   NodePath characterNP = find("**/+Character");
   PT(Character) characterPtr = DCAST(Character, characterNP.node());
   if(characterPtr != NULL)
      {
      PT(CharacterJoint) characterJointPtr = characterPtr->find_joint(jointName);
      if(characterJointPtr != NULL)
         {
         for(int i = 0; !foundJoint && i < characterPtr->get_num_bundles(); ++i)
            {
            if(characterPtr->get_bundle(i)->control_joint(jointName, jointNp.node()))
               {
               foundJoint = true;
               jointNp.set_mat(characterJointPtr->get_default_value());
               }
            }
         }
      }
   if(!foundJoint)
      {
      nout << "ERROR: cannot control joint `" << jointName << "'." << endl;
      jointNp.remove_node();
      }
   return jointNp;
   }

// This function exposes a joint via its NodePath handle.
// Think of it as a read from that joint interface.
//
// Return value:
// The joint's NodePath, parented to the actor.
//
// Parameter:
//    jointName: the joint's name as found in the model file.
NodePath CActor::expose_joint(const string& jointName)
   {
   bool foundJoint = false;
   NodePath jointNp = attach_new_node(jointName);
   NodePath characterNP = find("**/+Character");
   PT(Character) characterPtr = DCAST(Character, characterNP.node());
   if(characterPtr != NULL)
      {
      PT(CharacterJoint) characterJointPtr = characterPtr->find_joint(jointName);
      if(characterJointPtr != NULL)
         {
         foundJoint = true;
         characterJointPtr->add_net_transform(jointNp.node());
         }
      }
   if(!foundJoint)
      {
      nout << "ERROR: no joint named `" << jointName << "'." << endl;
      jointNp.remove_node();
      }
   return jointNp;
   }
