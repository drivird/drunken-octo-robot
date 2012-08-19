/*
 * distortionDemo.h
 *
 *  Created on: 2012-08-17
 *      Author: dri
 */

#ifndef DISTORTIONDEMO_H_
#define DISTORTIONDEMO_H_

#include "pandaFramework.h"
#include "cLerpNodePathInterval.h"
#include "../p3util/cBufferViewer.h"

NodePath add_title(WindowFramework* windowFramework, const string& text);

class DistortionDemo
   {
   public:

   DistortionDemo(WindowFramework* windowFramework);

   private:

   NodePath add_instructions(float pos, const string& msg) const;
   AsyncTask::DoneStatus update_scene(GenericAsyncTask *task);
   GraphicsOutput* make_FBO(const string& name) const;
   void toggle_buffer_viewer(const Event* event);
   void toggle_distortion(const Event* event);
   void quit(const Event* event);

   PT(WindowFramework) m_windowFramework;
   NodePath m_title;
   NodePath m_inst1;
   NodePath m_inst2;
   NodePath m_inst4;
   NodePath m_seascape;
   PT(GraphicsOutput) m_distortionBuffer;
   NodePath m_distortionObject;
   PT(CLerpNodePathInterval) m_hprInterval;
   PT(Texture) m_texDistortion;
   CBufferViewer m_bufferViewer;
   bool m_distortionOn;
   };

#endif /* DISTORTIONDEMO_H_ */
