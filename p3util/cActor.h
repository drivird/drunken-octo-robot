/*
 * cActor.h
 *
 *  Created on: 2012-05-24
 *      Author: dri
 *
 * This class is meant to emulate the capabilities of python Actor class. It is by no mean a
 * full implementation. It only contains what is needed to translate the Panda3D tutorials.
 */

#ifndef CACTOR_H_
#define CACTOR_H_

#include "pandaFramework.h"

class CActor : public NodePath, public AnimControlCollection
   {
   public:

   typedef vector<string> NameVec;
   typedef map<string, NameVec> AnimMap;

   CActor();
   virtual ~CActor();

   void load_actor(WindowFramework* windowFrameworkPtr,
                   const string& actorFilename,
                   const AnimMap* animMapPtr,
                   int hierarchyMatchFlags);
   NodePath control_joint(const string& jointName);
   NodePath expose_joint(const string& jointName);

   private:

   typedef vector<NodePath> NodePathVec;

   void load_anims(const AnimMap* animMapPtr,
                   const string& filename,
                   int hierarchyMatchFlags);
   bool is_stored(const AnimControl* animPtr);
   };

#endif /* CACTOR_H_ */
