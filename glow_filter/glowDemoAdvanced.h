/*
 * glowDemoAdvanced.h
 *
 *  Created on: 2012-07-01
 *      Author: dri
 */

#ifndef GLOWDEMOADVANCED_H_
#define GLOWDEMOADVANCED_H_

#include "../p3util/cActor.h"
#include "../p3util/cBufferViewer.h"
#include "../p3util/cOnscreenText.h"
#include "cLerpNodePathInterval.h"
#include "orthographicLens.h"
#include "pandaFramework.h"

class GlowDemoAdvanced
   {
   public:

   GlowDemoAdvanced(WindowFramework* windowFrameworkPtr);

   private:

   NodePath add_instructions(float pos, const string& msg);
   NodePath add_title(const string& text);
   GraphicsOutput* make_filter_buffer(GraphicsOutput* srcbuffer,
                                      const string& name,
                                      int sort,
                                      const string& prog);

   struct MakeCamera2dParameters
      {
      MakeCamera2dParameters(GraphicsOutput* win);
      PT(GraphicsOutput) win;
      int sort;
      LVecBase4f displayRegion;
      LVecBase4f coords;
      PT(OrthographicLens) lens;
      string cameraName;
      private:
      MakeCamera2dParameters(); // to prevent its use
      };
   NodePath make_camera_2d(const MakeCamera2dParameters& params);

   void toggle_glow();
   void toggle_display();

   GlowDemoAdvanced(); // to prevent use of the default constructor
   static AsyncTask::DoneStatus step_interval_manager(GenericAsyncTask* taskPtr,
                                                      void* dataPtr);
   static void sys_exit(const Event* eventPtr, void* dataPtr);
   static void call_buffer_viewer_toggle_enable(const Event* eventPtr,
                                                void* dataPtr);
   static void call_toggle_glow(const Event* eventPtr, void* dataPtr);
   static void call_toggle_display(const Event* eventPtr, void* dataPtr);

   PT(WindowFramework) m_windowFrameworkPtr;
   NodePath m_titleNp;
   NodePath m_inst1Np;
   NodePath m_inst2Np;
   NodePath m_inst3Np;
   NodePath m_inst4Np;
   CActor m_tron;
   bool m_isRunning;
   NodePath m_finalcard;
   CBufferViewer m_bufferViewer;
   bool m_glowOn;
   PT(CLerpNodePathInterval) m_hprIntervalPtr;
   NodePath m_camera2d;
   };

#endif /* GLOWDEMOADVANCED_H_ */
