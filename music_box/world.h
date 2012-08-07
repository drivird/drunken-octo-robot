/*
 * world.h
 *
 *  Created on: 2012-07-31
 *      Author: dri
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "audioManager.h"
#include "../p3util/cSoundInterval.h"
#include "cMetaInterval.h"
#include "cLerpNodePathInterval.h"

class World
   {
   public:

   World(WindowFramework* windowFramework);
   ~World();

   private:

   void quit(const Event* event);
   AsyncTask::DoneStatus async_task(GenericAsyncTask *task);
   void toggle_music_box(const Event* event);
   void set_music_box_volume(const Event* event);

   NodePath MakeButton(const string& name,
                       const string& label,
                       string* event_name) const;

   PT(WindowFramework) m_windowFramework;
   NodePath m_title;
   NodePath m_escapeEventText;
   PT(AudioManager) m_musicManager;
   PT(AudioSound) m_musicBoxSound;
   float m_musicTime;
   PT(AudioManager) m_sfxManager;
   PT(AudioSound) m_lidSfx;
   PT(CSoundInterval) m_lidOpenSfx;
   PT(CSoundInterval) m_lidCloseSfx;
   NodePath m_sliderText;
   NodePath m_slider;
   NodePath m_buttonOpen;
   NodePath m_buttonClose;
   bool m_boxOpen;
   NodePath m_musicBox;
   NodePath m_Lid;
   NodePath m_Panda;
   NodePath m_HingeNode;
   PT(CMetaInterval) m_lidClose;
   PT(CMetaInterval) m_lidOpen;
   PT(CLerpNodePathInterval) m_PandaTurn;
   };

inline World::~World()
   {
   m_musicManager->shutdown();
   m_sfxManager->shutdown();
   }

#endif /* WORLD_H_ */
