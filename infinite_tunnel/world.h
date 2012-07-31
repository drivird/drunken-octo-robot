/*
 * world.h
 *
 *  Created on: 2012-07-26
 *      Author: dri
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "fog.h"
#include "cLerpNodePathInterval.h"

class World
   {
   public:

   World(WindowFramework* windowFramework);

   private:

   static const int TUNNEL_SEGMENT_LENGTH;
   static const int TUNNEL_TIME;

   NodePath gen_label_text(const string& text, int i) const;
   void quit(const Event* event);
   void toggle_fog(const Event* event);
   template<int r, int g, int b> void set_fog_color(const Event* event);
   template<int r, int g, int b> void set_background_color(const Event* event);
   template<int value> void add_fog_density(const Event* event);
   void init_tunnel();
   void cont_tunnel(const Event* event);
   static AsyncTask::DoneStatus step_interval_manager(
      GenericAsyncTask* taskPtr, void* dataPtr);
   void handle_pause(const Event* event);

   PT(WindowFramework) m_windowFramework;
   NodePath m_title;
   NodePath m_escapeEventText;
   NodePath m_pkeyEventText;
   NodePath m_tkeyEventText;
   NodePath m_dkeyEventText;
   NodePath m_sdkeyEventText;
   NodePath m_rkeyEventText;
   NodePath m_srkeyEventText;
   NodePath m_bkeyEventText;
   NodePath m_sbkeyEventText;
   NodePath m_gkeyEventText;
   NodePath m_sgkeyEventText;
   NodePath m_lkeyEventText;
   NodePath m_slkeyEventText;
   NodePath m_pluskeyEventText;
   NodePath m_minuskeyEventText;
   PT(Fog) m_fog;
   NodePath m_tunnel[4];
   PT(CLerpNodePathInterval) m_tunnelMove;
   };

#endif /* WORLD_H_ */
