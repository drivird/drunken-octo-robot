/*
 * cWorld.cpp
 *
 *  Created on: 2012-07-03
 *      Author: dri
 */

#include "../p3util/cOnscreenText.h"
#include "cardMaker.h"
#include "load_prc_file.h"
#include "world.h"

const char* World::MEDIAFILE = "../PandaSneezes.avi";

// Function to put instructions on the screen.
NodePath World::add_instructions(float pos, const string& msg)
   {
   COnscreenText instruction("instruction");
   instruction.set_text(msg);
   instruction.set_fg(Colorf(0, 0, 0, 1));
   instruction.set_pos(LVecBase2f(-1.3, pos));
   instruction.set_align(TextNode::A_left);
   instruction.set_scale(0.05);
   instruction.set_shadow(Colorf(1, 1, 1, 1));
   instruction.set_shadow_offset(LVecBase2f(0.1, 0.1));
   instruction.reparent_to(m_windowFrameworkPtr->get_aspect_2d());
   return instruction.generate();
   }

// Function to put title on the screen.
NodePath World::add_title(const string& text)
   {
   COnscreenText title("title");
   title.set_text(text);
   title.set_fg(Colorf(0, 0, 0, 1));
   title.set_pos(LVecBase2f(1.3, -0.95));
   title.set_align(TextNode::A_right);
   title.set_scale(0.07);
   title.set_shadow(Colorf(1, 1, 1, 1));
   title.set_shadow_offset(LVecBase2f(0.05, 0.05));
   title.reparent_to(m_windowFrameworkPtr->get_aspect_2d());
   return title.generate();
   }

World::World(WindowFramework* windowFrameworkPtr)
   : m_windowFrameworkPtr(windowFrameworkPtr),
     m_title(),
     m_inst1(),
     m_inst2(),
     m_inst3(),
     m_tex(NULL),
     m_audioManager(NULL),
     m_sound(NULL),
     m_audioLoop(NULL)
   {
   m_title = add_title("Panda3D: Tutorial - Media Player");
   m_inst1 = add_instructions(0.95,"P: Play/Pause");
   m_inst2 = add_instructions(0.90,"S: Stop and Rewind");
   m_inst3 = add_instructions(0.85,"M: Slow Motion / Normal Motion toggle");

   // Load the texture. We could use loader.loadTexture for this,
   // but we want to make sure we get a MovieTexture, since it
   // implements synchronizeTo.
   m_tex = new MovieTexture("name");
   if(!m_tex->read(MEDIAFILE))
      {
      nout << "Failed to load video!" << endl;
      }

   // Set up a fullscreen card to set the video texture on.
   CardMaker cm("My Fullscreen Card");
   cm.set_frame_fullscreen_quad();
   cm.set_uv_range(m_tex);
   NodePath card(cm.generate());
   NodePath render2d = m_windowFrameworkPtr->get_render_2d();
   card.reparent_to(render2d);
   card.set_texture(m_tex);
   card.set_tex_scale(TextureStage::get_default(), m_tex->get_tex_scale());

   // Note: function loader.loadSfx is python only. In C++ we need to manage
   //       the AudioManager ourselves. Look for self.sfxManagerList in file
   //       ShowBase.py for more info. The procedure is like that:
   //       *) configure which audio library to use
   //       1) create the manager
   //       2) load the file
   //       3) update the manage using a task
   //       4) shutdown the manager when you are done

   // Tell Panda3D to use OpenAL, not FMOD
   load_prc_file_data("", "audio-library-name p3openal_audio");

   m_audioManager = AudioManager::create_AudioManager();
   m_sound = m_audioManager->get_sound(MEDIAFILE);
   AsyncTaskManager::get_global_ptr()->add(new GenericAsyncTask("audioLoop",
                                                                call_audio_loop,
                                                                this));
   // Synchronize the video to the sound.
   m_tex->synchronize_to(m_sound);

   m_windowFrameworkPtr->enable_keyboard();
   m_windowFrameworkPtr->get_panda_framework()->
      define_key("p", "playpause", call_play_pause, this);
   m_windowFrameworkPtr->get_panda_framework()->
      define_key("s", "stopsound", call_stop_sound, this);
   m_windowFrameworkPtr->get_panda_framework()->
      define_key("m", "fastforward", call_fast_forward, this);
   }

void World::stop_sound()
   {
   m_sound->stop();
   m_sound->set_play_rate(1.0);
   }

void World::fast_forward()
   {
   nout << m_sound->status() << endl;
   if(m_sound->status() == AudioSound::PLAYING)
      {
      float t = m_sound->get_time();
      m_sound->stop();
      if(m_sound->get_play_rate() == 1.0)
         {
         m_sound->set_play_rate(0.5);
         }
      else
         {
         m_sound->set_play_rate(1.0);
         }
      m_sound->set_time(t);
      m_sound->play();
      }
   }

void World::play_pause()
   {
   if(m_sound->status() == AudioSound::PLAYING)
      {
      nout << "stop" << endl;
      float t = m_sound->get_time();
      m_sound->stop();
      m_sound->set_time(t);
      }
   else
      {
      nout << "play" << endl;
      m_sound->play();
      }
   }

void World::audio_loop()
   {
   m_audioManager->update();
   }

void World::call_stop_sound(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<World*>(dataPtr)->stop_sound();
   }

void World::call_fast_forward(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<World*>(dataPtr)->fast_forward();
   }

void World::call_play_pause(const Event* eventPtr, void* dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return;
      }

   static_cast<World*>(dataPtr)->play_pause();
   }

AsyncTask::DoneStatus World::call_audio_loop(GenericAsyncTask *taskPtr, void *dataPtr)
   {
   // preconditions
   if(dataPtr == NULL)
      {
      nout << "ERROR: parameter dataPtr cannot be NULL." << endl;
      return AsyncTask::DS_exit;
      }

   static_cast<World*>(dataPtr)->audio_loop();
   return AsyncTask::DS_cont;
   }
