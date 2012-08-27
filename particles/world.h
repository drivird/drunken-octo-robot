/*
 * world.h
 *
 *  Created on: 2012-08-23
 *      Author: dri
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "pandaFramework.h"
#include "particleSystemManager.h"
#include "physicsManager.h"

class World
   {
   public:

   World(WindowFramework* windowFramework);

   private:

   enum ParticleConfig
      {
      PC_steam,
      PC_dust,
      PC_fountain,
      PC_smoke,
      PC_smokering,
      PC_fireish
      };

   void quit(const Event* event);
   void setup_lights();
   template<int particleConfig> void load_particle_config(const Event* event = NULL);
   void load_steam_config(ParticleSystem* system, ForceNode* forceNode);
   void load_dust_config(ParticleSystem* system, ForceNode* forceNode);
   void load_fountain_config(ParticleSystem* system, ForceNode* forceNode);
   void load_smoke_config(ParticleSystem* system, ForceNode* forceNode);
   void load_smokering_config(ParticleSystem* system, ForceNode* forceNode);
   void load_fireish_config(ParticleSystem* system, ForceNode* forceNode);
   AsyncTask::DoneStatus enable_particles(GenericAsyncTask *task);

   static const char* HELPTEXT;

   PT(WindowFramework) m_windowFramework;
   NodePath m_title;
   NodePath m_escapeEvent;
   NodePath m_t;
   NodePath m_p;
   ParticleSystemManager m_particleSysMgr;
   PhysicsManager m_physicsMgr;
   };

#endif /* WORLD_H_ */
