/*
 * cActorInterval.h
 *
 *  Created on: 2012-05-25
 *      Author: dri
 *
 * This class is meant to emulate the capabilities of python ActorInterval class. It is by no mean
 * a full implementation. It only contains what is needed to translate the Panda3D tutorials.
 */

#ifndef CACTORINTERVAL_H_
#define CACTORINTERVAL_H_

#include "cInterval.h"
#include "cActor.h"

class CActorInterval : public CInterval
   {
   public:

   CActorInterval(const string& name, CActor* actorPtr, const string& animName);
   CActorInterval(const string& name, CActor* actorPtr, const string& animName, double playRate);
   CActorInterval(const string& name, CActor* actorPtr, const string& animName, int startFrame, int endFrame);
   CActorInterval(const string& name, CActor* actorPtr, const string& animName, int startFrame, int endFrame, double playRate);
   virtual ~CActorInterval();

   protected:

   virtual void priv_step(double t);
   virtual void priv_finalize();

   private:

   void init(CActor* actorPtr, const string& animName, int startFrame, int endFrame, double playRate);

   CActor* m_actorPtr;
   string m_animName;
   double m_frameRate;
   int m_startFrame;
   int m_endFrame;
   bool m_reverse;
   int m_numFrames;
   bool m_implicitDuration;
   bool m_constrainedLoop;
   bool m_loopAnim;
   };

#endif /* CACTORINTERVAL_H_ */
