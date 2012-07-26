/*
 * sleepWhenMinimized.h
 *
 *  Created on: 2012-07-12
 *      Author: dri
 *
 * This class, upon instantiation will start a task to query the state of the
 * Panda3d window. If it is minimized, it will make the thread sleep for 10 ms.
 * This way, Panda3d will take it slow on the CPU while minimized. Instantiate
 * this class in each thread you want to slow down.
 *
 * This code is mostly based on the code by atari314:
 * http://www.panda3d.org/forums/viewtopic.php?t=13375
 *
 */

#ifndef SLEEPWHENMINIMIZED_H_
#define SLEEPWHENMINIMIZED_H_

#include "pandaFramework.h"

class SleepWhenMinimized
   {
   public:
   SleepWhenMinimized(WindowFramework* wf);
   ~SleepWhenMinimized();

   static AsyncTask::DoneStatus sleepTask(GenericAsyncTask* task, void* data);

   private:
   static int m_cnt;
   PT(GenericAsyncTask) m_task;
   };

inline
SleepWhenMinimized::~SleepWhenMinimized()
   {
   AsyncTaskManager::get_global_ptr()->remove(m_task);
   --m_cnt;
   }

#endif /* SLEEPWHENMINIMIZED_H_ */
