/*
 * cFunctionInterval.cpp
 *
 *  Created on: 2012-05-26
 *      Author: dri
 */

#include "asyncTaskManager.h"
#include "genericAsyncTask.h"
#include "genericFunctionInterval.h"

int GenericFunctionInterval::InstanceCounter = 0;

GenericFunctionInterval::GenericFunctionInterval(const string& name,
                                                 IntervalFunc* functionPtr,
                                                 void* dataPtr,
                                                 bool openEnded)
   : CInterval(name, 0, openEnded),
     m_functionPtr(functionPtr),
     m_dataPtr(dataPtr)
   {
   ++InstanceCounter;
   if(functionPtr == NULL)
      {
      nout << "ERROR: parameter functionPtr cannot be NULL." << endl;
      }
   }

GenericFunctionInterval::~GenericFunctionInterval()
   {
   ;
   }

void GenericFunctionInterval::priv_instant()
   {
   check_stopped(get_class_type(), "priv_instant");
   // Evaluate the function
   if(m_functionPtr != NULL)
      {
      // Note: the following code would deadlock when trying to access data from another
      //       running interval (such as in the Boxing-Robots tutorial, i.e. checkPunch
      //       accessing resetHead). I'm guessing it is because the running thread1 is
      //       asking data from thread2 who is waiting for thread1 to finish...
      //
      //       (*m_functionPtr)(m_dataPtr);
      //
      //       The truth is I did not try to figure it out, but I rather looked for a
      //       workaround to indirectly ask data to thread2 using an AsyncTask.
      string taskName("GenericFunctionIntervalTask");
      taskName += InstanceCounter;
      AsyncTaskManager::get_global_ptr()->add(new GenericAsyncTask(taskName, wrapper, this));
      }
   _state = S_final;
   }

AsyncTask::DoneStatus GenericFunctionInterval::wrapper(GenericAsyncTask* taskPtr, void* dataPtr)
   {
   if(dataPtr != NULL)
      {
      PT(GenericFunctionInterval) ptr = static_cast<GenericFunctionInterval*>(dataPtr);
      (*ptr->m_functionPtr)(ptr->m_dataPtr);
      }
   return AsyncTask::DS_done;
   }
