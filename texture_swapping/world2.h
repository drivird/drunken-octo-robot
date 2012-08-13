/*
 * world2.h
 *
 *  Created on: 2012-08-12
 *      Author: dri
 *
 * Original header from Panda3d python tutorial:
 *
 * Author: Shao Zhang and Phil Saltzman
 * Last Updated: 4/18/2005
 *
 * textureMovie.py shows how to set up a texture movie effect. This tutorial
 * shows how to use that effect in a different context. Instead of setting the
 * texture based on time, this tutorial has an elevator that sets a texture on
 * its floor based on its height so that shadows cast by nearby lights line up
 * correctly.
 *
 * The elevator, shaft, and shadow movie were created by T.J. Jackson
 * for the Entertainment Technology Center class Building Virtual Worlds
 */

#ifndef WORLD2_H_
#define WORLD2_H_

#include "pandaFramework.h"
#include "cLerpNodePathInterval.h"

class World2
   {
   public:

   World2(WindowFramework* windowFramework);

   private:

   typedef vector<PT(Texture)> TexVec;
   typedef vector<NodePath> NodePathVec;

   void load_texture_movie(int frames,
                           const string& name,
                           const string& suffix,
                           int padding,
                           vector<PT(Texture)>* texs);
   AsyncTask::DoneStatus elevator_shadows(GenericAsyncTask *task);

   PT(WindowFramework) m_windowFramework;
   NodePath m_title;
   NodePath m_elevator;
   NodePath m_shadowPlane;
   TexVec m_shadowTexs;
   NodePathVec m_shaft;
   PT(CLerpNodePathInterval) m_elevatorInterval;
   };

#endif /* WORLD2_H_ */
