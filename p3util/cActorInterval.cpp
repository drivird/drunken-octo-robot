/*
 * cActorInterval.cpp
 *
 *  Created on: 2012-05-25
 *      Author: dri
 */

#include "cActor.h"
#include "cActorInterval.h"

CActorInterval::CActorInterval(const string& name, CActor* actorPtr, const string& animName)
   : CInterval(name, 0, false)
   {
   int startFrame = 0;
   int endFrame = 0;
   if(actorPtr->find_anim(animName) != NULL)
      {
      startFrame = 0;
      endFrame = actorPtr->get_num_frames(animName)-1;
      }
   double playRate = 1.0;
   init(actorPtr, animName, startFrame, endFrame, playRate);
   }

CActorInterval::CActorInterval(const string& name, CActor* actorPtr, const string& animName, double playRate)
   : CInterval(name, 0, false)
   {
   int startFrame = 0;
   int endFrame = 0;
   if(actorPtr->find_anim(animName) != NULL)
      {
      startFrame = 0;
      endFrame = actorPtr->get_num_frames(animName)-1;
      }
   init(actorPtr, animName, startFrame, endFrame, playRate);
   }

CActorInterval::CActorInterval(const string& name,
                               CActor* actorPtr,
                               const string& animName,
                               int startFrame,
                               int endFrame)
   : CInterval(name, 0, false)
   {
   double playRate = 1.0;
   init(actorPtr, animName, startFrame, endFrame, playRate);
   }

CActorInterval::CActorInterval(const string& name,
                               CActor* actorPtr,
                               const string& animName,
                               int startFrame,
                               int endFrame,
                               double playRate)
   : CInterval(name, 0, false)
   {
   init(actorPtr, animName, startFrame, endFrame, playRate);
   }

CActorInterval::~CActorInterval()
   {
   ;
   }

void CActorInterval::init(CActor* actorPtr, const string& animName, int startFrame, int endFrame, double playRate)
   {
   m_actorPtr = actorPtr;
   m_animName = animName;
   m_startFrame = startFrame;
   m_endFrame = endFrame;
   m_implicitDuration = true;
   m_constrainedLoop = false;
   m_loopAnim = false;
   _play_rate = playRate;

   if(m_actorPtr->find_anim(m_animName) == NULL)
      {
      nout << "Unknown animation for actor: " << m_animName << endl;
      m_frameRate = 1.0;
      m_startFrame = 0;
      m_endFrame = 0;
      }
   else
      {
      m_frameRate = m_actorPtr->find_anim(m_animName)->get_frame_rate() * fabs(_play_rate);
      }

   // Must we play the animation backwards?  We play backwards if
   // either (or both) of the following is true: the playRate is
   // negative, or endFrame is before startFrame.
   m_reverse = (_play_rate < 0);
   if(m_endFrame < m_startFrame)
      {
      m_reverse = true;
      int t = m_endFrame;
      m_endFrame = m_startFrame;
      m_startFrame = t;
      }

   m_numFrames = m_endFrame - m_startFrame + 1;

   // Compute duration if no duration specified
   _duration = (double)m_numFrames / m_frameRate;
   }

void CActorInterval::priv_step(double t)
   {
   CInterval::priv_step(t);

   double frameCount = t * m_frameRate;
   if(m_constrainedLoop)
      {
      double numLoops = frameCount / m_numFrames;
      frameCount -= numLoops * m_numFrames;
      }

   double absFrame = (m_reverse ? m_endFrame   - frameCount :
                                  m_startFrame + frameCount);

   // Calc integer frame number
   int intFrame = (int)(floor(absFrame + 0.0001));

   // Pose anim

   int numFrames = m_actorPtr->get_num_frames(m_animName);
   int frame = (m_loopAnim ? (intFrame % numFrames) + (absFrame - intFrame) :
                             max(min(absFrame, numFrames-1.0), 0.0));

   m_actorPtr->pose(m_animName, frame);
   }


void CActorInterval::priv_finalize()
   {
   check_started(get_class_type(), "priv_finalize");

   if(m_implicitDuration && !m_loopAnim)
      {
      //  As a special case, we ensure we end up posed to the last
      //  frame of the animation if the original duration was
      //  implicit.  This is necessary only to guard against
      //  possible roundoff error in computing the final frame
      //  from the duration.  We don't do this in the case of a
      //  looping animation, however, because this would introduce
      //  a hitch in the animation when it plays back-to-back with
      //  the next cycle.
      if(m_reverse) { m_actorPtr->pose(m_animName, m_startFrame); }
      else          { m_actorPtr->pose(m_animName, m_endFrame);   }
      }
   else
      {
      // Otherwise, the user-specified duration determines which
      // is our final frame.
      priv_step(get_duration());
      }

   _state = S_final;
   interval_done();
   }
