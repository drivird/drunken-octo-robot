/*
 * genericFunctionInterval.h
 *
 *  Created on: 2012-05-26
 *      Author: dri
 *
 * This class is meant to emulate the capabilities of python FunctionInterval class. It is by no
 * mean a full implementation. It only contains what is needed to translate the Panda3D tutorials.
 */

#ifndef GENERICFUNCTIONINTERVAL_H_
#define GENERICFUNCTIONINTERVAL_H_

#include "cInterval.h"

class GenericFunctionInterval : public CInterval
   {
   public:

   typedef void IntervalFunc(void *dataPtr);

   GenericFunctionInterval(const string& name, IntervalFunc* functionPtr, void* dataPtr, bool openEnded);
   virtual ~GenericFunctionInterval();

   protected:

   virtual void priv_instant();

   private:

   static AsyncTask::DoneStatus wrapper(GenericAsyncTask* taskPtr, void* dataPtr);

   static int InstanceCounter;
   IntervalFunc* m_functionPtr;
   void* m_dataPtr;
   };

#endif /* GENERICFUNCTIONINTERVAL_H_ */
