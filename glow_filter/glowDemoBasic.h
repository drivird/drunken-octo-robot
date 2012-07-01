/*
 * glowDemoBasic.h
 *
 *  Created on: 2012-06-29
 *      Author: dri
 */

#ifndef GLOWDEMOBASIC_H_
#define GLOWDEMOBASIC_H_

#include "../p3util/cActor.h"
#include "../p3util/cBufferViewer.h"
#include "../p3util/cCommonFilters.h"
#include "../p3util/cOnscreenText.h"
#include "cLerpNodePathInterval.h"
#include "pandaFramework.h"

class GlowDemoBasic
   {
   public:

   GlowDemoBasic(WindowFramework* windowFrameworkPtr);

   private:

   NodePath add_instructions(float pos, const string& msg);
   NodePath add_title(const string& text);
   void toggle_glow();
   void toggle_display();

   GlowDemoBasic(); // to prevent use of the default constructor
   static AsyncTask::DoneStatus step_interval_manager(GenericAsyncTask* taskPtr,
                                                      void* dataPtr);
   static void sys_exit(const Event* eventPtr, void* dataPtr);
   static void call_buffer_viewer_toggle_enable(const Event* eventPtr,
                                                void* dataPtr);
   static void call_toggle_glow(const Event* eventPtr, void* dataPtr);
   static void call_toggle_display(const Event* eventPtr, void* dataPtr);

   PT(WindowFramework) m_windowFrameworkPtr;
   auto_ptr<CCommonFilters> m_filters;
   int m_glowSize;
   NodePath m_titleNp;
   NodePath m_inst1Np;
   NodePath m_inst2Np;
   NodePath m_inst3Np;
   NodePath m_inst4Np;
   CActor m_tron;
   bool m_isRunning;
   PT(CLerpNodePathInterval) m_hprIntervalPtr;
   CBufferViewer m_bufferViewer;
   };

#endif /* GLOWDEMOBASIC_H_ */
