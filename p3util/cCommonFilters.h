/*
 * cCommonFilters.h
 *
 *  Created on: 2012-06-20
 *      Author: dri
 *
 * This class implements in C++ Panda3D python CommonFilters. I have tried to
 * remain as close as possible to the python code. This is why structs are used
 * instead of classes. Also, light-weight structs are used to simulate python
 * positional parameters but the design could be further improved using the name
 * parameter idiom. -dri
 *
 * Original python header:
 *
 * Class CommonFilters implements certain common image
 * postprocessing filters.
 *
 * It is not ideal that these filters are all included in a single
 * monolithic module.  Unfortunately, when you want to apply two filters
 * at the same time, you have to compose them into a single shader, and
 * the composition process isn't simply a question of concatenating them:
 * you have to somehow make them work together.  I suspect that there
 * exists some fairly simple framework that would make this automatable.
 * However, until I write some more filters myself, I won't know what
 * that framework is.  Until then, I'll settle for this
 * clunky approach.  - Josh
 *
 */

#ifndef CCOMMONFILTERS_H_
#define CCOMMONFILTERS_H_

#include "pandaFramework.h"
#include "cFilterManager.h"

struct CCommonFilters
   {
   CCommonFilters(GraphicsOutput* win, NodePath cam);

   bool set_cartoon_ink(float separation = 1);
   bool del_cartoon_ink();

   struct SetBloomParameters
      {
      SetBloomParameters();
      LVector4f blend;
      float mintrigger;
      float maxtrigger;
      float desat;
      float intensity;
      string size;
      };
   bool set_bloom(const SetBloomParameters& params);
   bool del_bloom();

   bool set_half_pixel_shift();
   bool del_half_pixel_shift();
   bool set_view_glow();
   bool del_view_glow();
   bool set_inverted();
   bool del_inverted();

   struct SetVolumetricLightingParameters
      {
      SetVolumetricLightingParameters(NodePath caster);
      NodePath caster;
      int numsamples;
      float density;
      float decay;
      float exposure;
      };
   bool set_volumetric_lighting(const SetVolumetricLightingParameters& params);
   bool del_volumetric_lighting();

   bool set_blur_sharpen(float amount = 0);
   bool del_blur_sharpen();

   struct SetAmbientOcclusionParameters
      {
      SetAmbientOcclusionParameters();
      int numsamples;
      float radius;
      float amount;
      float strength;
      float falloff;
      };
   bool set_ambient_occlusion(const SetAmbientOcclusionParameters& params);
   bool del_ambient_occlusion();
   void resize_buffers();

   bool reconfigure(bool fullrebuild, const string& changed);
   void cleanup();

   struct BaseConfiguration
      {
      bool isSet;
      };
   struct CartoonInkConfiguration : public BaseConfiguration
      {
      float separation;
      };
   struct BloomConfiguration : public BaseConfiguration
      {
      LVector4f blend;
      float maxtrigger;
      float mintrigger;
      float desat;
      float intensity;
      string size;
      };
   struct VolumetricLightingConfiguration : public BaseConfiguration
      {
      NodePath caster;
      int numsamples;
      float density;
      float decay;
      float exposure;
      };
   struct BlurSharpenConfiguration : public BaseConfiguration
      {
      float amount;
      };
   struct AmbientOcclusionConfiguration : public BaseConfiguration
      {
      int numsamples;
      float radius;
      float amount;
      float strength;
      float falloff;
      };
   typedef map<string, BaseConfiguration*> Configuration;
   typedef vector<NodePath> NodePathVec;
   typedef map<string, bool> BoolMap;

   static const char* CARTOON_BODY;
   CFilterManager m_manager;
   Configuration m_configuration;
   NodePath m_finalQuad;
   NodePathVec m_bloom;
   NodePathVec m_blur;
   NodePathVec m_ssao;
   CFilterManager::TexMap m_textures;
   GenericAsyncTask* m_task;

   private:
   const Shader* load_shader(const string& name) const;
   void update();
   static AsyncTask::DoneStatus call_update(GenericAsyncTask *task,
                                            void *user_data);
   };

#endif /* CCOMMONFILTERS_H_ */
