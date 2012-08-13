/*
 * world.h
 *
 *  Created on: 2012-08-10
 *      Author: dri
 *
 * Original header from Panda3d python tutorial:
 *
 * Author: Shao Zhang and Phil Saltzman
 * Last Updated: 4/18/2005
 *
 * This tutorial shows how to use a sequence of textures on an object to
 * achieve a specific effect. Popular uses of this technique are:
 * -Animated sprites
 * -Moving shadows
 *
 * This tutorial also demonstrates the billboard function which orients
 * an object to always face to the camera. This is useful for 2D sprites
 * in a 3D world.

 * The duck animation was created by Shane Liesegang and William Houng
 * for the Entertainment Technology Center class Building Virtual Worlds
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "pandaFramework.h"

class World
   {
   public:

   World(WindowFramework* windowFramework);

   private:

   typedef vector<PT(Texture)> TexVec;

   void quit(const Event* event);
   void load_texture_movie(int frames,
                           const string& name,
                           const string& suffix,
                           int padding,
                           vector<PT(Texture)>* texs);
   template<int fps, typename T1, T1 objT, typename T2, T2 texturesT>
      AsyncTask::DoneStatus texture_movie(GenericAsyncTask *task);
   void set_view_main(const Event* event);
   void set_view_billboard(const Event* event);

   PT(WindowFramework) m_windowFramework;
   NodePath m_title;
   NodePath m_escapeEventText;
   NodePath m_onekeyEventText;
   NodePath m_twokeyEventText;
   NodePath m_duckPlane;
   TexVec m_duckTexs;
   PT(GenericAsyncTask) m_duckTask;
   int m_fps;
   NodePath m_expPlane;
   TexVec m_expTexs;
   PT(GenericAsyncTask) m_expTask;
   NodePath m_orientPlane;
   PT(Texture) m_orientTex;
   NodePath m_trackball;
   };

#endif /* WORLD_H_ */
