/*
 * sleepWhenMinimized.cpp
 *
 *  Created on: 2012-07-18
 *      Author: dri
 */

#include "sleepWhenMinimized.h"

int SleepWhenMinimized::m_cnt = 0;

SleepWhenMinimized::SleepWhenMinimized(WindowFramework* wf)
   : m_task(NULL)
   {
   ostringstream name("SleepTask");
   name << m_cnt++;
   m_task = new GenericAsyncTask(name.str(), &sleepTask, wf);
   AsyncTaskManager::get_global_ptr()->add(m_task);
   }

AsyncTask::DoneStatus SleepWhenMinimized::sleepTask(GenericAsyncTask* task, void* data)
   {
   PT(WindowFramework) wf = static_cast<WindowFramework*>(data);

   if(wf->get_graphics_window()->get_properties().get_minimized())
      {
      Thread::sleep(0.01);
      }

   return AsyncTask::DS_cont;
   }
