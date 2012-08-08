/*
 * cSoundInterval.h
 *
 *  Created on: 2012-08-02
 *      Author: dri
 *
 * C++ implementation of SoundInterval, a Panda3d python class. Most of the
 * functionalities are there save the volume adjusted according to the distance
 * to a listenerNode as this feature depends on SfxPlayer, another Panda3d
 * python class. As a workaround, setup a task that computes the distance from
 * the sound source and adjust the volume at every frame using set_volume().
 */

#ifndef CSOUNDINTERVAL_H_
#define CSOUNDINTERVAL_H_

#include "cInterval.h"
#include "randomizer.h"

class CSoundInterval : public CInterval
   {
   public:

   class Parameters;

   CSoundInterval(const Parameters& parameters);

   void set_volume(float volume);
   void loop(double start_time = 0.0,
             double end_t = -1.0,
             double play_rate = 1.0,
             bool stagger = false);
   virtual void finish();

   virtual void priv_initialize(double t);
   virtual void priv_step(double t);
   virtual void priv_finalize();
   virtual void priv_interrupt();

   private:

   void play_sound(double t);

   PT(AudioSound) _sound;
   float _sound_duration;
   bool _loop;
   float _volume;
   double _start_time;
   bool _seamless_loop;
   bool _sound_playing;
   bool _in_finish;
   Randomizer _randomizer;
   };

inline void 
CSoundInterval::set_volume(float volume)
   {
   _volume = volume;
   }

class CSoundInterval::Parameters
   {
   public:

   Parameters(const string& name,
              AudioSound* sound);

   const string& name() const;
   double duration() const;
   bool open_ended() const;
   AudioSound* sound() const;
   float volume() const;
   double start_time() const;
   bool seamless_loop() const;
   bool loop() const;

   Parameters& loop(bool loop);
   Parameters& duration(double duration);
   Parameters& volume(float volume);
   Parameters& start_time(double start_time);
   Parameters& seamless_loop(bool seamless_loop);

   private:

   string _name;
   PT(AudioSound) _sound;
   bool _loop;
   double _duration;
   float _volume;
   double _start_time;
   bool _seamless_loop;
   bool _open_ended;
   };

inline
CSoundInterval::Parameters::Parameters(const string& name,
                                       AudioSound* sound)
   : _name(name),
     _sound(sound),
     _loop(false),
     _duration(0.0),
     _volume(1.0),
     _start_time(0.0),
     _seamless_loop(true),
     _open_ended(false)
   {
   // Empty
   }

inline const string&
CSoundInterval::Parameters::name() const
   {
   return _name;
   }

inline double
CSoundInterval::Parameters::duration() const
   {
   return _duration;
   }

inline bool
CSoundInterval::Parameters::open_ended() const
   {
   return _open_ended;
   }

inline AudioSound*
CSoundInterval::Parameters::sound() const
   {
   return _sound;
   }

inline float
CSoundInterval::Parameters::volume() const
   {
   return _volume;
   }

inline double
CSoundInterval::Parameters::start_time() const
   {
   return _start_time;
   }

inline bool
CSoundInterval::Parameters::seamless_loop() const
   {
   return _seamless_loop;
   }

inline bool
CSoundInterval::Parameters::loop() const
   {
   return _loop;
   }

inline CSoundInterval::Parameters&
CSoundInterval::Parameters::loop(bool loop)
   {
   _loop = loop;
   return *this;
   }

inline CSoundInterval::Parameters&
CSoundInterval::Parameters::duration(double duration)
   {
   _duration = duration;
   return *this;
   }

inline CSoundInterval::Parameters&
CSoundInterval::Parameters::volume(float volume)
   {
   _volume = volume;
   return *this;
   }

inline CSoundInterval::Parameters&
CSoundInterval::Parameters::start_time(double start_time)
   {
   _start_time = start_time;
   return *this;
   }

inline CSoundInterval::Parameters&
CSoundInterval::Parameters::seamless_loop(bool seamless_loop)
   {
   _seamless_loop = seamless_loop;
   return *this;
   }

#endif /* CSOUNDINTERVAL_H_ */
