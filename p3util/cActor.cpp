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

   // If there are no animations to bind to the actor, we are done here.
   if(animMapPtr == NULL) { return; }

   // load anims from the actor model first
   load_anims(animMapPtr, actorFilename, hierarchyMatchFlags);

   NodePathVec animNpVec;
   animNpVec.reserve(animMapPtr->size());

   // then for each animations specified by the user
   for(AnimMap::const_iterator animMapItr = animMapPtr->begin(); animMapItr != animMapPtr->end(); ++animMapItr)
      {
      const string& filename = (*animMapItr).first;
      if(filename != actorFilename)
         {
         // load the animation as a child of the actor
         NodePath animNp = windowFrameworkPtr->load_model(*this, filename);

         // load anims from that file
         load_anims(animMapPtr, filename, hierarchyMatchFlags);

         // detach the node so that it will not appear in a new call to auto_bind()
         animNp.detach_node();
         // keep it on the side
         animNpVec.push_back(animNp);
         }
      }

   // re-attach the animation nodes to the actor
   for(NodePathVec::iterator npItr = animNpVec.begin(); npItr < animNpVec.end(); ++npItr)
      {
      (*npItr).reparent_to(*this);
      }
   }

void CActor::load_anims(const AnimMap* animMapPtr,
                        const string& filename,
                        int hierarchyMatchFlags)
   {
   // precondition
   if(animMapPtr == NULL)
      {
      nout << "ERROR: parameter animMapPtr cannot be NULL." << endl;
      return;
      }

   // use auto_bind() to gather the anims
   AnimControlCollection tempCollection;
   auto_bind(this->node(), tempCollection, hierarchyMatchFlags);

   // get the anim names for the current file
   AnimMap::const_iterator animMapItr = animMapPtr->find(filename);
   if(animMapItr != animMapPtr->end())
      {
      // first, test the anim names
      for(NameVec::const_iterator nameItr = (*animMapItr).second.begin(); nameItr != (*animMapItr).second.end(); ++nameItr)
         {
         // make sure this name is not currently stored by the actor
         if(find_anim(*nameItr) == NULL)
            {
            // check if auto_bind() found an animation with the right name
            PT(AnimControl) animPtr = tempCollection.find_anim(*nameItr);
            if(animPtr != NULL)
               {
               store_anim(animPtr, *nameItr);
               tempCollection.unbind_anim(*nameItr);
               nout << "Stored animation `" << *nameItr << "' from file `" << (*animMapItr).first << "'" << endl;
               }
            }
         }

      // deal the remaining anims
      int animIdx = 0;
      for(NameVec::const_iterator nameItr = (*animMapItr).second.begin(); nameItr != (*animMapItr).second.end(); ++nameItr)
         {
         // make sure this name is not currently stored by the actor
         if(find_anim(*nameItr) == NULL)
            {
            // make sure there is at least one anim left to store
            PT(AnimControl) animPtr = tempCollection.get_anim(animIdx);
            if(animPtr != NULL)
               {
               store_anim(animPtr, *nameItr);
               ++animIdx;
               nout << "Stored animation `" << *nameItr << "' from file `" << (*animMapItr).first << "'" << endl;
               }
            }
         }
      }
   }

bool CActor::is_stored(const AnimControl* animPtr)
   {
   for(int i = 0; i < get_num_anims(); ++i)
      {
      PT(AnimControl) storedAnimPtr = get_anim(i);

      if(animPtr->get_anim() == storedAnimPtr->get_anim() &&
         animPtr->get_part() == storedAnimPtr->get_part() )
         {
         return true;
         }
      }
   return false;
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
