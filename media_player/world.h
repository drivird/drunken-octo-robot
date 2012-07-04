/*
 * cWorld.h
 *
 *  Created on: 2012-07-03
 *      Author: dri
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "pandaFramework.h"
#include "movieTexture.h"
#include "audioManager.h"

class World
   {
   public:

   World(WindowFramework* windowFrameworkPtr);
   ~World();

   private:

   NodePath add_instructions(float pos, const string& msg);
   NodePath add_title(const string& text);
   void stop_sound();
   void fast_forward();
   void play_pause();
   void audio_loop();

   static void call_stop_sound(const Event* eventPtr, void* dataPtr);
   static void call_fast_forward(const Event* eventPtr, void* dataPtr);
   static void call_play_pause(const Event* eventPtr, void* dataPtr);
   static AsyncTask::DoneStatus call_audio_loop(GenericAsyncTask *task, void *user_data);

   static const char* MEDIAFILE;

   PT(WindowFramework) m_windowFrameworkPtr;
   NodePath m_title;
   NodePath m_inst1;
   NodePath m_inst2;
   NodePath m_inst3;
   PT(MovieTexture) m_tex;
   PT(AudioManager) m_audioManager;
   PT(AudioSound) m_sound;
   PT(GenericAsyncTask) m_audioLoop;
   };

inline World::~World()
   {
   m_audioManager->shutdown();
   }

#endif /* WORLD_H_ */
