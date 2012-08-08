/*
 * cSoundInterval.cpp
 *
 *  Created on: 2012-08-02
 *      Author: dri
 */

#include "audioSound.h"
#include "cSoundInterval.h"

CSoundInterval::CSoundInterval(const Parameters& parameters)
   : CInterval(parameters.name(),
               parameters.duration(),
               parameters.open_ended()),
     _sound(parameters.sound()),
     _sound_duration(0),
     _loop(parameters.loop()),
     _volume(parameters.volume()),
     _start_time(parameters.start_time()),
     _seamless_loop(parameters.seamless_loop()),
     _sound_playing(false),
     _in_finish(false),
     _randomizer(Randomizer::get_next_seed())
   {
   if(_sound != NULL)
      {
      _sound_duration = _sound->length();
      }

   // If no duration given use sound's duration as interval's duration
   if(_duration == 0.0 && _sound != NULL)
      {
      _duration = max(_sound_duration - _start_time, 0.0);
      }
   }

void CSoundInterval::priv_initialize(double t)
   {
   // If it's within a 10th of a second of the start,
   // start at the beginning
   double t1 = t + _start_time;

   if(t1 < 0.1)
      {
      t1 = 0.0;
      }

   if(t1 < _sound_duration && !(_seamless_loop && _sound_playing))
      {
      play_sound(t1);
      _sound_playing = true;
      }

   CInterval::priv_initialize(t);
   }

void CSoundInterval::priv_step(double t)
   {
   if(_state == S_paused)
      {
      // Restarting from a pause.
      double t1 = t + _start_time;
      if(t1 < _sound_duration)
         {
         play_sound(t1);
         }
      }

   CInterval::priv_step(t);
   }

void CSoundInterval::priv_finalize()
   {
   // if we're just coming to the end of a seamless loop, leave the sound alone,
   // let the audio subsystem loop it
   if(_seamless_loop && _sound_playing && _do_loop && !_in_finish)
      {
      return;
      }
   else if(_sound != NULL)
      {
      _sound->stop();
      _sound_playing = false;
      }
   CInterval::priv_finalize();
   }

void CSoundInterval::priv_interrupt()
   {
   if(_sound != NULL)
      {
      _sound->stop();
      _sound_playing = false;
      }
   CInterval::priv_interrupt();
   }

void CSoundInterval::play_sound(double t1)
   {
   _sound->set_volume(_volume);
   _sound->set_time(t1);
   _sound->set_loop(_loop);
   _sound->play();
   }

void CSoundInterval::loop(double start_time, // = 0.0
                          double end_t,      // = -1.0
                          double play_rate,  // = 1.0
                          bool stagger)      // = false
   {
   loop(start_time, end_t, play_rate);
   if(stagger)
      {
      set_t(_randomizer.random_real(1) * get_duration());
      }
   }

void CSoundInterval::finish()
   {
   _in_finish = true;
   CInterval::finish();
   _in_finish = false;
   }

