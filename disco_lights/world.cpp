/*
 * world.cpp
 *
 *  Created on: 2012-07-21
 *      Author: dri
 */

#include "../p3util/callbackUtil.h"
#include "pandaFramework.h"
#include "ambientLight.h"
#include "directionalLight.h"
#include "spotlight.h"
#include "pointLight.h"
#include "cIntervalManager.h"
#include "world.h"

// Note: these macros lighten calls to the template functions.
#define WORLD_ADD_TASK(name, func) \
   add_task_t<World, &World::func>(name, this)

#define WORLD_DEFINE_KEY(event_name, description, func) \
   define_key_t<World, &World::func>(m_windowFramework, \
                                     event_name,        \
                                     description,       \
                                     this)

#define ADD_BRIGHTNESS(light, amount) \
   add_brightness<NodePath World::*, &World::light, amount>

#define TOGGLE_LIGHTS(light) \
   toggle_lights<NodePath World::*, &World::light>

// Simple function to keep a value in a given range (by default 0 to 1)
float World::restrain(float i,
                      float mn,       // = 0
                      float mx) const // = 1
   {
   return min(max(i, mn), mx);
   }

// Macro-like function to reduce the amount of code needed to create the
// onscreen instructions
void World::make_status_label(float i, COnscreenText* label)
   {
   // precondition
   if(label == NULL)
      {
      nout << "ERROR: parameter label cannot be NULL." << endl;
      }

   *label = COnscreenText("makeStatusLabel", COnscreenText::TS_plain);
   label->set_fg(Colorf(1,1,0,1));
   label->set_pos(-1.3, 0.95 - (.05 * i));
   label->set_align(TextNode::A_left);
   label->set_scale(0.05);
   NodePath aspect2d = m_windowFramework->get_aspect_2d();
   label->reparent_to(aspect2d);
   }

// The main initialization of our class
World::World(WindowFramework* windowFramework)
   : m_windowFramework(windowFramework),
     m_title(),
     m_ambientText("text"),
     m_directionalText("text"),
     m_spotlightText("text"),
     m_pointLightText("text"),
     m_spinningText("text"),
     m_ambientBrightnessText("text"),
     m_directionalBrightnessText("text"),
     m_spotlightBrightnessText("text"),
     m_spotlightExponentText("text"),
     m_lightingPerPixelText("text"),
     m_lightingShadowsText("text"),
     m_disco(),
     m_ambientLight(),
     m_directionalLight(),
     m_spotlight(),
     m_redHelper(),
     m_greenHelper(),
     m_blueHelper(),
     m_redPointLight(),
     m_greenPointLight(),
     m_bluePointLight(),
     m_pointLightHelper(),
     m_pointLightsSpin(NULL),
     m_arePointLightsSpinning(true),
     m_perPixelEnabled(false), // Per-pixel lighting and ...
     m_shadowsEnabled(false)   // shadows are initially off.
   {
   // precondition
   if(windowFramework == NULL)
      {
      nout << "ERROR: parameter windowFramework cannot be NULL." << endl;
      return;
      }

   // This creates the on screen title that is in every tutorial
   COnscreenText title("title", COnscreenText::TS_plain);
   title.set_text("Panda3D: Tutorial - Lighting");
   title.set_fg(Colorf(1,1,0,1));
   title.set_pos(0.87,-0.95);
   title.set_scale(0.07);
   m_title = title.generate();
   NodePath aspect2d = m_windowFramework->get_aspect_2d();
   m_title.reparent_to(aspect2d);

   // Creates labels used for onscreen instructions
   make_status_label(0, &m_ambientText);
   make_status_label(1, &m_directionalText);
   make_status_label(2, &m_spotlightText);
   make_status_label(3, &m_pointLightText);
   make_status_label(4, &m_spinningText);
   make_status_label(5, &m_ambientBrightnessText);
   make_status_label(6, &m_directionalBrightnessText);
   make_status_label(7, &m_spotlightBrightnessText);
   make_status_label(8, &m_spotlightExponentText);
   make_status_label(9, &m_lightingPerPixelText);
   make_status_label(10, &m_lightingShadowsText);

   NodePath models = m_windowFramework->get_panda_framework()->get_models();
   m_disco = m_windowFramework->load_model(models, "../models/disco_hall");
   NodePath render = m_windowFramework->get_render();
   m_disco.reparent_to(render);
   m_disco.set_pos_hpr(0, 50, -4, 90, 0, 0);

   // First we create an ambient light. All objects are affected by ambient
   // light equally
   // Create and name the ambient light
   PT(AmbientLight) aLight = new AmbientLight("ambientLight");
   m_ambientLight = render.attach_new_node(aLight);
   // Set the color of the ambient light
   aLight->set_color(Colorf(0.1, 0.1, 0.1, 1));
   // add the newly created light to the lightAttrib

   // Now we create a directional light. Directional lights add shading from a
   // given angle. This is good for far away sources like the sun
   PT(DirectionalLight) dLight = new DirectionalLight("directionalLight");
   m_directionalLight = render.attach_new_node(dLight);
   dLight->set_color(Colorf(0.35, 0.35, 0.35, 1));
   // The direction of a directional light is set as a 3D vector
   dLight->set_direction(LVector3f(1, 1, -2 ));
   // These settings are necessary for shadows to work correctly
   m_directionalLight.set_z(6);
   PT(Lens) dlens = DCAST(DirectionalLight,
      m_directionalLight.node())->get_lens();
   dlens->set_film_size(41, 21);
   dlens->set_near_far(50, 75);
   //DCAST(DirectionalLight, m_directionalLight.node())->show_frustum();

   // Now we create a spotlight. Spotlights light objects in a given cone
   // They are good for simulating things like flashlights
   NodePath camera = m_windowFramework->get_camera_group();
   PT(Spotlight) sLight = new Spotlight("spotlight");
   m_spotlight = camera.attach_new_node(sLight);
   sLight->set_color(Colorf(0.45, 0.45, 0.45, 1));
   //The cone of a spotlight is controlled by it's lens. This creates the lens
   sLight->set_lens(new PerspectiveLens());
   //This sets the Field of View (fov) of the lens, in degrees for width and
   //height. The lower the numbers, the tighter the spotlight.
   sLight->get_lens()->set_fov(16, 16);
   // Attenuation controls how the light fades with distance. The numbers are
   // The three values represent the three constants (constant, linear, and
   // quadratic) in the internal lighting equation. The higher the numbers the
   // shorter the light goes.
   sLight->set_attenuation(LVecBase3f(1, 0.0, 0.0));
   // This exponent value sets how soft the edge of the spotlight is. 0 means a
   // hard edge. 128 means a very soft edge.
   sLight->set_exponent(60.0);

   // Now we create three colored Point lights. Point lights are lights that
   // radiate from a single point, like a light bulb. Like spotlights, they
   // are given position by attaching them to NodePaths in the world
   m_redHelper = m_windowFramework->load_model(models, "../models/sphere");
   m_redHelper.set_color(Colorf(1, 0, 0, 1));
   m_redHelper.set_pos(-6.5, -3.75, 0);
   m_redHelper.set_scale(.25);
   PT(PointLight) redPLight = new PointLight("redPointLight");
   m_redPointLight = m_redHelper.attach_new_node(redPLight);
   redPLight->set_color(Colorf(0.35, 0, 0, 1));
   redPLight->set_attenuation(LVecBase3f(0.1, 0.04, 0.0));

   // The green point light and helper
   m_greenHelper = m_windowFramework->load_model(models, "../models/sphere");
   m_greenHelper.set_color(Colorf(0, 1, 0, 1));
   m_greenHelper.set_pos(0, 7.5, 0);
   m_greenHelper.set_scale(.25);
   PT(PointLight) greenPLight = new PointLight("greenPointLight");
   m_greenPointLight = m_greenHelper.attach_new_node(greenPLight);
   greenPLight->set_attenuation(LVecBase3f(0.1, 0.04, 0.0));
   greenPLight->set_color(Colorf(0, 0.35, 0, 1));

   // The blue point light and helper
   m_blueHelper = m_windowFramework->load_model(models, "../models/sphere");
   m_blueHelper.set_color(Colorf(0, 0, 1, 1));
   m_blueHelper.set_pos(6.5, -3.75, 0);
   m_blueHelper.set_scale(.25);
   PT(PointLight) bluePLight = new PointLight("bluePointLight");
   m_bluePointLight = m_blueHelper.attach_new_node(bluePLight);
   bluePLight->set_attenuation(LVecBase3f(0.1, 0.04, 0.0));
   bluePLight->set_color(Colorf(0, 0, 0.35, 1));
   bluePLight->set_specular_color(Colorf(1));

   // Create a dummy node so the lights can be spun with one command
   m_pointLightHelper = render.attach_new_node("pointLightHelper");
   m_pointLightHelper.set_pos(0, 50, 11);
   m_redHelper.reparent_to(m_pointLightHelper);
   m_greenHelper.reparent_to(m_pointLightHelper);
   m_blueHelper.reparent_to(m_pointLightHelper);

   // Finally we store the lights on the root of the scene graph.
   // This will cause them to affect everything in the scene.

   render.set_light(m_ambientLight);
   render.set_light(m_directionalLight);
   render.set_light(m_spotlight);
   render.set_light(m_redPointLight);
   render.set_light(m_greenPointLight);
   render.set_light(m_bluePointLight);

   // Create and start interval to spin the lights, and a variable to
   // manage them.
   m_pointLightsSpin = new CLerpNodePathInterval("pointLightsSpin",
                                                 6,
                                                 CLerpInterval::BT_no_blend,
                                                 true,
                                                 false,
                                                 m_pointLightHelper,
                                                 NodePath());
   m_pointLightsSpin->set_start_hpr(LVecBase3f(0, 0, 0));
   m_pointLightsSpin->set_end_hpr(LVecBase3f(360, 0, 0));
   m_pointLightsSpin->loop();
   // Note: setup a task to step the interval manager
   WORLD_ADD_TASK("stepIntervalManager", step_interval_manager);

   // listen to keys for controlling the lights
   WORLD_DEFINE_KEY("escape", "quit", quit);
   WORLD_DEFINE_KEY("a", "toggleAmbientLight",
      TOGGLE_LIGHTS(m_ambientLight));
   WORLD_DEFINE_KEY("d", "toggleDirectionalLight",
      TOGGLE_LIGHTS(m_directionalLight));
   WORLD_DEFINE_KEY("s", "toggleSpotLight",
      TOGGLE_LIGHTS(m_spotlight));
   // Note: instead of passing the three PointLight in an array, three events
   //       are defined using the same key.
   WORLD_DEFINE_KEY("p", "toggleRedPointLight",
      TOGGLE_LIGHTS(m_redPointLight));
   WORLD_DEFINE_KEY("p", "toggleGreenPointLight",
      TOGGLE_LIGHTS(m_greenPointLight));
   WORLD_DEFINE_KEY("p", "toggleBluePointLight",
      TOGGLE_LIGHTS(m_bluePointLight));

   WORLD_DEFINE_KEY("r", "toggleSpinningPointLights",
      toggle_spinning_point_lights);
   WORLD_DEFINE_KEY("l", "togglePerPixelLighting", toggle_per_pixel_lighting);
   WORLD_DEFINE_KEY("e", "toggleShadows", toggle_shadows);
   WORLD_DEFINE_KEY("z", "decSpotBrightness",
      ADD_BRIGHTNESS(m_ambientLight, -5));
   WORLD_DEFINE_KEY("x", "incSpotBrightness",
      ADD_BRIGHTNESS(m_ambientLight, 5));
   WORLD_DEFINE_KEY("c", "decDirectionalBrightness",
      ADD_BRIGHTNESS(m_directionalLight, -5));
   WORLD_DEFINE_KEY("v", "incDirectionalBrightness",
      ADD_BRIGHTNESS(m_directionalLight, 5));
   WORLD_DEFINE_KEY("b", "decSpotBrightness",
      ADD_BRIGHTNESS(m_spotlight, -5));
   WORLD_DEFINE_KEY("n", "incSpotBrightness",
      ADD_BRIGHTNESS(m_spotlight, 5));
   WORLD_DEFINE_KEY("q", "decSpotlightExponent", adjust_spotlight_exponent<-1>);
   WORLD_DEFINE_KEY("w", "incSpotlightExponent", adjust_spotlight_exponent<1>);

   // Finally call the function that builds the instruction texts
   update_status_label();
   }


// This function takes a list of lights and toggles their state. It takes in a
// list so that more than one light can be toggled in a single command
template<typename T, T nontype_param>
void World::toggle_lights(const Event* event)
   {
   const NodePath& light = this->*nontype_param;

   NodePath render = m_windowFramework->get_render();
   // If the given light is in our lightAttrib, remove it.
   // This has the effect of turning off the light
   if(render.has_light(light))
      {
      render.clear_light(light);
      }
   // Otherwise, add it back. This has the effect of turning the light on
   else
      {
      render.set_light(light);
      }

   update_status_label();
   }

// This function toggles the spinning of the point intervals by pausing and
// resuming the interval
void World::toggle_spinning_point_lights(const Event* event)
   {
   if(m_arePointLightsSpinning)
      {
      m_pointLightsSpin->pause();
      }
   else
      {
      m_pointLightsSpin->resume();
      }
   m_arePointLightsSpinning = !m_arePointLightsSpinning;
   update_status_label();
   }

// This function turns per-pixel lighting on or off.
void World::toggle_per_pixel_lighting(const Event* event)
   {
   NodePath render = m_windowFramework->get_render();
   if(m_perPixelEnabled)
      {
      m_perPixelEnabled = false;
      render.clear_shader();
      }
   else
      {
      m_perPixelEnabled = true;
      render.set_shader_auto();
      }
   update_status_label();
   }

// This function turns shadows on or off.
void World::toggle_shadows(const Event* event)
   {
   if(m_shadowsEnabled)
      {
      m_shadowsEnabled = false;
      DCAST(DirectionalLight, m_directionalLight.node())->
         set_shadow_caster(false);
      }
   else
      {
      if(!m_perPixelEnabled)
         {
         toggle_per_pixel_lighting(NULL);
         }
      m_shadowsEnabled = true;
      DCAST(DirectionalLight, m_directionalLight.node())->
         set_shadow_caster(true, 512, 512);
      }
   update_status_label();
   }

// This function changes the spotlight's exponent. It is kept to the range
// 0 to 128. Going outside of this range causes an error
template<int amount>
void World::adjust_spotlight_exponent(const Event* event)
   {
   PT(Spotlight) spotlight = DCAST(Spotlight, m_spotlight.node());
   float e = restrain(spotlight->get_exponent() + amount, 0, 128);
   spotlight->set_exponent(e);
   update_status_label();
   }

// This function reads the color of the light, uses a built-in python function
// (from the library colorsys) to convert from RGB (red, green, blue) color
// representation to HSB (hue, saturation, brightness), so that we can get the
// brighteness of a light, change it, and then convert it back to rgb to chagne
// the light's color
template<typename T, T nontype_param, int value>
void World::add_brightness(const Event* event)
   {
   // Note: downcast to the right class using TypeHandle
   PT(Light) light = NULL;
   PT(PandaNode) node = (this->*nontype_param).node();
   if(node->is_of_type(AmbientLight::get_class_type()))
      {
      light = DCAST(AmbientLight, node);
      }
   else if(node->is_of_type(DirectionalLight::get_class_type()))
      {
      light = DCAST(DirectionalLight, node);
      }
   else if(node->is_of_type(Spotlight::get_class_type()))
      {
      light = DCAST(Spotlight, node);
      }
   else
      {
      nout << "ERROR: unknown light type." << endl;
      return;
      }

   float amount = value / 100.0;
   Colorf color = light->get_color();
   float h = 0;
   float s = 0;
   float v = 0;
   rgb_to_hsv(color[0], color[1], color[2], &h, &s, &v);
   float brightness = restrain(v + amount);
   float r = 0;
   float g = 0;
   float b = 0;
   hsv_to_rgb(h, s, brightness, &r, &g, &b);
   light->set_color(Colorf(r, g, b, 1));

   update_status_label();
   }

// Builds the onscreen instruction labels
void World::update_status_label()
   {
   NodePath render = m_windowFramework->get_render();
   update_label(&m_ambientText, "(a) ambient is",
      render.has_light(m_ambientLight));
   update_label(&m_directionalText, "(d) directional is",
      render.has_light(m_directionalLight));
   update_label(&m_spotlightText, "(s) spotlight is",
      render.has_light(m_spotlight));
   update_label(&m_pointLightText, "(p) point lights are",
      render.has_light(m_redPointLight));
   update_label(&m_spinningText, "(r) point light spinning is",
      m_arePointLightsSpinning);
   m_ambientBrightnessText.set_text("(z,x) Ambient Brightness: " +
      get_brightness_string(*DCAST(AmbientLight, m_ambientLight.node())));
   m_directionalBrightnessText.set_text("(c,v) Directional Brightness: " +
      get_brightness_string(*DCAST(DirectionalLight,
         m_directionalLight.node())));
   m_spotlightBrightnessText.set_text("(b,n) Spotlight Brightness: " +
      get_brightness_string(*DCAST(Spotlight, m_spotlight.node())));
   ostringstream exponent;
   exponent << DCAST(Spotlight, m_spotlight.node())->get_exponent();
   m_spotlightExponentText.set_text("(q,w) Spotlight Exponent: " +
      exponent.str());
   update_label(&m_lightingPerPixelText, "(l) Per-pixel lighting is",
      m_perPixelEnabled);
   update_label(&m_lightingShadowsText, "(e) Shadows are", m_shadowsEnabled);
   }

// Appends either (on) or (off) to the base string based on the bassed value
void World::update_label(COnscreenText* obj, const string& base, bool var)
   {
   string s(var ? " (on)" : " (off)");
   obj->set_text(base + s);
   }

// Returns the brightness of a light as a string to put it in the instruction
// labels
string World::get_brightness_string(const Light& light) const
   {
   Colorf color = light.get_color();
   float h = 0;
   float s = 0;
   float v = 0;
   rgb_to_hsv(color[0], color[1], color[2], &h, &s, &v);
   ostringstream value;
   value << setprecision(2) << v;
   return value.str();
   }

// Converts an RGB color value to HSV. Adapted from:
// http://mjijackson.com/2008/02/rgb-to-hsl-and-rgb-to-hsv-color-model-
// conversion-algorithms-in-javascript
void World::rgb_to_hsv(float r, float g, float b,
   float* h, float* s, float* v) const
   {
   float mx = max(max(r, g), b);
   float mn = min(min(r, g), b);
   *v = mx;

   float d = mx - mn;
   *s = (mx == 0 ? 0 : d / mx);

   if(mx == mn)
      {
      *h = 0; // achromatic
      }
   else
      {
      if(mx == r)
         {
         *h = (g - b) / d + (g < b ? 6 : 0);
         }
      else if(mx == g)
         {
         *h = (b - r) / d + 2;
         }
      else if(mx == b)
         {
         *h = (r - g) / d + 4;
         }
      else
         {
         nout << "ERROR: rgb_to_hsv." << endl;
         }
      *h /= 6;
      }
   }

// Converts an HSV color value to RGB. Adapted from:
// http://mjijackson.com/2008/02/rgb-to-hsl-and-rgb-to-hsv-color-model-
// conversion-algorithms-in-javascript
void World::hsv_to_rgb(float h, float s, float v,
   float* r, float* g, float* b) const
   {
   int i = h * 6;
   float f = h * 6 - i;
   float p = v * (1 - s);
   float q = v * (1 - f * s);
   float t = v * (1 - (1 - f) * s);

   switch(i % 6)
      {
      case 0: *r = v; *g = t; *b = p; break;
      case 1: *r = q; *g = v; *b = p; break;
      case 2: *r = p; *g = v; *b = t; break;
      case 3: *r = p; *g = q; *b = v; break;
      case 4: *r = t; *g = p; *b = v; break;
      case 5: *r = v; *g = p; *b = q; break;
      }
   }

AsyncTask::DoneStatus World::step_interval_manager(GenericAsyncTask *task)
   {
   CIntervalManager::get_global_ptr()->step();
   return AsyncTask::DS_cont;
   }

void World::quit(const Event* event)
   {
   m_windowFramework->get_panda_framework()->set_exit_flag();
   }
