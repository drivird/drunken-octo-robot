/*
 * callbackUtil.h
 *
 *  Created on: 2012-07-17
 *      Author: dri
 *
 * This file contains utilities to help when working with the different callback
 * functions of Panda3d. The fact is I got tired of mindlessly writing static
 * stub functions each time I wanted to trap a keyboard event or a to create a
 * new task. So, thanks to my friend mister VViZard, I now have these template
 * functions that will create these stubs compile time. Perfect job for a
 * machine :)
 *
 * Templatized callbacks are:
 * PandaFramework::define_key --> define_key_t
 * AsyncTaskManager::add --> add_task_t
 */

#ifndef CALLBACKUTIL_H_
#define CALLBACKUTIL_H_

#include "pandaFramework.h"

// forward declarations
template<class T, void (T::*func)(const Event* event)>
void EventCallbackStub(const Event* event, void* obj);

template<class T, AsyncTask::DoneStatus (T::*func)(GenericAsyncTask *task)>
AsyncTask::DoneStatus TaskStub(GenericAsyncTask *task, void* obj);

// Function: define_key_t
//
// *Brief*
// Templatized version of PandaFramework::define_key. Will create at compilation
// time the static stub function that calls the member function func of object
// obj.
//
// *Parameters*
// T           class of obj, see that parameter below.
// func        member function to callback, i.e. T::func, upon the key event.
//             The signature of the member function must be:
//             void T::foo(const Event* event)
// wf          the Panda3d window posting the events.
// event_name  same parameter as in PandaFramework::define_key.
// description same parameter as in PandaFramework::define_key.
// obj         the object affected by the key event.
//
// *Example*
// For class World with member function move_forward that is to be called each
// time key `w' is pressed:
//
// World::World(WindowFramework* wf)
//    {
//    ...
//    define_key_t<World, &World::move_forward>(wf, "w", "MoveForward", this);
//    ...
//    }
//
// void World::move_forward(const Event* event)
//    {
//    ...
//    }
//
// Can be simplified further using a macro to specify constant parameters
// (assuming World has a data member m_wf that points to the relevant
// WindowFramework*):
//
// #define WORLD_DEFINE_KEY(event_name, description, func)
// define_key_t<World, &World::func>(m_wf, event_name, description, this)
//
// Then the call above would be reduced to:
//
// WORLD_DEFINE_KEY("w", "MoveForward", move_forward);
//
template <class T, void (T::*func)(const Event* event)>
inline void define_key_t(WindowFramework* wf,
                         const string& event_name,
                         const string& description,
                         T* obj)
   {
   // preconditions
   if(wf == NULL)
      {
      nout << "ERROR: parameter wf cannot be null." << endl;
      return;
      }

   // keyboard needs to be enabled and since it does hurt to enable it
   // multiple times, just enable it at each call.
   wf->enable_keyboard();
   // at the compilation, an EventCallbackStub for T::func will be generated
   wf->get_panda_framework()->define_key(event_name,
                                         description,
                                         &EventCallbackStub<T, func>,
                                         obj);
   }


// Function: add_task_t
//
// *Brief*
// Templatized version of AsyncTaskManager::add. Will create at compilation
// time the static stub function that calls the member function func of object
// obj.
//
// *Parameters*
// T    class of obj, see that parameter below.
// func member function to callback, i.e. T::func, upon the key event.
//      The signature of the member function must be:
//      AsyncTask::DoneStatus T::foo(GenericAsyncTask *task)
// wf   the Panda3d window posting the events.
// name same parameter as in GenericAsyncTask::GenericAsyncTask.
// obj  the object affected by the key event.
//
// *Example*
// For class World with member function audio_loop that is to be called each
// frame:
//
// PT(GenericTask) task =
//    add_task_t<World, &World::audio_loop>("AudioLoop", this);
//
// Can be simplified further using a macro to specify constant parameters:
//
// #define WORLD_ADD_TASK(name, func)
// add_task_t<World, &World::func>(name, this)
//
// Then the call above would be reduced to:
//
// PT(GenericTask) task = WORLD_ADD_TASK("AudioLoop", audio_loop);
//
template<class T, AsyncTask::DoneStatus (T::*func)(GenericAsyncTask *task)>
inline GenericAsyncTask* add_task_t(const string &name, T* obj)
   {
   // create the new task. A TaskStub for T::func will be generated.
   PT(GenericAsyncTask) task = new GenericAsyncTask(name,
                                                    &TaskStub<T, func>,
                                                    obj);
   // register the new task with the global AsyncTaskManager
   AsyncTaskManager::get_global_ptr()->add(task);
   // return the task so the user can control it
   return task;
   }

// Templatized stub function with the signature of EventHandler::
// EventCallbackFunction. Used by define_key_t().
template<class T, void (T::*func)(const Event* event)>
void EventCallbackStub(const Event* event, void* obj)
   {
   // preconditions
   if(obj == NULL)
      {
      nout << "ERROR: parameter obj cannot be null." << endl;
      return;
      }

   (static_cast<T*>(obj)->*func)(event);
   }

// Templatized stub function with the signature of GenericAsyncTask::
// TaskFunc. Used by add_task_t().
template<class T, AsyncTask::DoneStatus (T::*func)(GenericAsyncTask *task)>
AsyncTask::DoneStatus TaskStub(GenericAsyncTask *task, void* obj)
   {
   // preconditions
   if(obj == NULL)
      {
      nout << "ERROR: parameter obj cannot be null." << endl;
      return AsyncTask::DS_done;
      }

   return (static_cast<T*>(obj)->*func)(task);
   }

#endif /* CALLBACKUTIL_H_ */
