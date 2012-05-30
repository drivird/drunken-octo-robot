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

   typedef map<string, string> AnimMap;

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
   };

#endif /* CACTOR_H_ */
