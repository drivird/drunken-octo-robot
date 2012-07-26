/*
 * world.h
 *
 *  Created on: 2012-07-21
 *      Author: dri
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "../p3util/cOnscreenText.h"
#include "cLerpNodePathInterval.h"

class World
   {
   public:

   World(WindowFramework* windowFramework);

   private:

   float restrain(float i, float mn = 0, float mx = 1) const;
   void make_status_label(float i, COnscreenText* label);
   AsyncTask::DoneStatus step_interval_manager(GenericAsyncTask *task);
   void quit(const Event* event);
   template<typename T, T nontype_param> void toggle_lights(const Event* event);
   void update_label(COnscreenText* obj, const string& base, bool var);
   void update_status_label();
   void toggle_spinning_point_lights(const Event* event);
   void rgb_to_hsv(float r, float g, float b,
      float* h, float* s, float* v) const;
   void hsv_to_rgb(float h, float s, float v,
      float* r, float* g, float* b) const;
   string get_brightness_string(const Light& light) const;
   void toggle_per_pixel_lighting(const Event* event);
   void toggle_shadows(const Event* event);
   template<typename T, T nontype_param, int value>
      void add_brightness(const Event* event);
   template<int amount> void adjust_spotlight_exponent(const Event* event);

   PT(WindowFramework) m_windowFramework;
   NodePath m_title;
   COnscreenText m_ambientText;
   COnscreenText m_directionalText;
   COnscreenText m_spotlightText;
   COnscreenText m_pointLightText;
   COnscreenText m_spinningText;
   COnscreenText m_ambientBrightnessText;
   COnscreenText m_directionalBrightnessText;
   COnscreenText m_spotlightBrightnessText;
   COnscreenText m_spotlightExponentText;
   COnscreenText m_lightingPerPixelText;
   COnscreenText m_lightingShadowsText;
   NodePath m_disco;
   NodePath m_ambientLight;
   NodePath m_directionalLight;
   NodePath m_spotlight;
   NodePath m_redHelper;
   NodePath m_greenHelper;
   NodePath m_blueHelper;
   NodePath m_redPointLight;
   NodePath m_greenPointLight;
   NodePath m_bluePointLight;
   NodePath m_pointLightHelper;
   PT(CLerpNodePathInterval) m_pointLightsSpin;
   bool m_arePointLightsSpinning;
   bool m_perPixelEnabled;
   bool m_shadowsEnabled;
   };

#endif /* WORLD_H_ */
