/*
 * toonMakerAdvanced.h
 *
 *  Created on: 2012-06-01
 *      Author: dri
 */

#ifndef TOONMAKERADVANCED_H_
#define TOONMAKERADVANCED_H_

#include "../p3util/cActor.h"
#include "../p3util/cBufferViewer.h"
#include "../p3util/cOnscreenText.h"

class ToonMakerAdvanced
   {
   public:

   ToonMakerAdvanced(WindowFramework* windowFrameworkPtr);

   private:

   NodePath add_instructions(float pos, const string& msg);
   NodePath add_title(const string& text);
   void increase_separation();
   void decrease_separation();
   void increase_cutoff();
   void decrease_cutoff();

   ToonMakerAdvanced(); // to prevent the use of the default constructor
   static AsyncTask::DoneStatus step_interval_manager(GenericAsyncTask* taskPtr, void* dataPtr);
   static void sys_exit(const Event* eventPtr, void* dataPtr);
   static void call_buffer_viewer_toggle_enable(const Event* eventPtr, void* dataPtr);
   static void call_increase_separation(const Event* eventPtr, void* dataPtr);
   static void call_decrease_separation(const Event* eventPtr, void* dataPtr);
   static void call_increase_cutoff(const Event* eventPtr, void* dataPtr);
   static void call_decrease_cutoff(const Event* eventPtr, void* dataPtr);

   PT(WindowFramework) m_windowFrameworkPtr;
   PT(GraphicsOutput) m_normalsBufferPtr;
   PT(DisplayRegion) m_displayRegionPtr;
   NodePath m_titleNp;
   NodePath m_inst1Np;
   NodePath m_inst2Np;
   NodePath m_inst3Np;
   NodePath m_inst4Np;
   NodePath m_drawnSceneNp;
   float m_separation;
   float m_cutoff;
   CActor m_character;
   CBufferViewer m_bufferViewer;
   };

#endif /* TOONMAKERADVANCED_H_ */
