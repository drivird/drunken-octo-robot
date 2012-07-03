/*
 * toonMakerBasic.h
 *
 *  Created on: 2012-06-23
 *      Author: dri
 */

#ifndef TOONMAKERBASIC_H_
#define TOONMAKERBASIC_H_

#include "../p3util/cActor.h"
#include "../p3util/cBufferViewer.h"
#include "../p3util/cCommonFilters.h"
#include "../p3util/cOnscreenText.h"
#include "pandaFramework.h"

class ToonMakerBasic
   {
   public:

   ToonMakerBasic(WindowFramework* windowFrameworkPtr);

   private:

   NodePath add_instructions(float pos, const string& msg);
   NodePath add_title(const string& text);
   void increase_separation();
   void decrease_separation();

   ToonMakerBasic(); // to prevent the use of the default constructor
   static AsyncTask::DoneStatus step_interval_manager(GenericAsyncTask* taskPtr,
                                                      void* dataPtr);
   static void sys_exit(const Event* eventPtr, void* dataPtr);
   static void call_buffer_viewer_toggle_enable(const Event* eventPtr,
                                                void* dataPtr);
   static void call_increase_separation(const Event* eventPtr, void* dataPtr);
   static void call_decrease_separation(const Event* eventPtr, void* dataPtr);
   static void call_filters_manager_resize_buffers(const Event* eventPtr,
                                                   void* dataPtr);

   PT(WindowFramework) m_windowFrameworkPtr;
   CBufferViewer m_bufferViewer;
   float m_separation;
   auto_ptr<CCommonFilters> m_filters;
   NodePath m_titleNp;
   NodePath m_inst1Np;
   NodePath m_inst2Np;
   NodePath m_inst3Np;
   CActor m_character;
   };

#endif /* TOONMAKERBASIC_H_ */
