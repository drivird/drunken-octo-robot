/*
 * cCommonFilters.cpp
 *
 *  Created on: 2012-06-20
 *      Author: dri
 *
 */

#include "cCommonFilters.h"
#include "shaderPool.h"
#include "auxBitplaneAttrib.h"
#include "texturePool.h"

const char* CCommonFilters::CARTOON_BODY = "\n"
                           "float4 cartoondelta = k_cartoonseparation * texpix_txaux.xwyw;\n"
                           "float4 cartoon_p0 = l_texcoordN + cartoondelta.xyzw;\n"
                           "float4 cartoon_c0 = tex2D(k_txaux, cartoon_p0.xy);\n"
                           "float4 cartoon_p1 = l_texcoordN - cartoondelta.xyzw;\n"
                           "float4 cartoon_c1 = tex2D(k_txaux, cartoon_p1.xy);\n"
                           "float4 cartoon_p2 = l_texcoordN + cartoondelta.wzyx;\n"
                           "float4 cartoon_c2 = tex2D(k_txaux, cartoon_p2.xy);\n"
                           "float4 cartoon_p3 = l_texcoordN - cartoondelta.wzyx;\n"
                           "float4 cartoon_c3 = tex2D(k_txaux, cartoon_p3.xy);\n"
                           "float4 cartoon_mx = max(cartoon_c0,max(cartoon_c1,max(cartoon_c2,cartoon_c3)));\n"
                           "float4 cartoon_mn = min(cartoon_c0,min(cartoon_c1,min(cartoon_c2,cartoon_c3)));\n"
                           "float cartoon_thresh = saturate(dot(cartoon_mx - cartoon_mn, float4(3,3,0,0)) - 0.5);\n"
                           "o_color = lerp(o_color, float4(0,0,0,1), cartoon_thresh);\n";

// Class CommonFilters implements certain common image postprocessing
// filters.  The constructor requires a filter builder as a parameter.
CCommonFilters::CCommonFilters(GraphicsOutput* win, NodePath cam)
   : m_manager(CFilterManager(win, cam)),
     m_configuration(),
     m_finalQuad(),
     m_bloom(4),
     m_blur(2),
     m_ssao(3),
     m_textures(),
     m_task(new GenericAsyncTask("common-filters-update", call_update, this))
   {
   m_configuration["CartoonInk"] = NULL;
   m_configuration["Bloom"] = NULL;
   m_configuration["HalfPixelShift"] = NULL;
   m_configuration["ViewGlow"] = NULL;
   m_configuration["Inverted"] = NULL;
   m_configuration["VolumetricLighting"] = NULL;
   m_configuration["BlurSharpen"] = NULL;
   m_configuration["AmbientOcclusion"] = NULL;

   cleanup();
   }

const Shader* CCommonFilters::load_shader(const string& name) const
   {
   // TODO go get the shaders in their install directory
   string fn("../../p3util/");
   fn += name;
   return ShaderPool::load_shader(fn);
   }

void CCommonFilters::cleanup()
   {
   m_manager.cleanup();
   m_textures.clear();
   m_finalQuad = NodePath(); // remove_node() ?
   m_bloom.clear();
   m_blur.clear();
   m_ssao.clear();

   PT(AsyncTask) task = AsyncTaskManager::
      get_global_ptr()->find_task("common-filters-update");
   if(task != NULL)
      {
      AsyncTaskManager::get_global_ptr()->remove(task);
      }
   }

// Reconfigure is called whenever any configuration change is made.
bool CCommonFilters::reconfigure(bool fullrebuild, const string& changed)
   {
   Configuration& configuration = m_configuration;

   if(fullrebuild)
      {
      cleanup();

      if(m_configuration["CartoonInk"] == NULL &&
         m_configuration["Bloom"] == NULL &&
         m_configuration["HalfPixelShift"] == NULL &&
         m_configuration["ViewGlow"] == NULL &&
         m_configuration["Inverted"] == NULL &&
         m_configuration["VolumetricLighting"] == NULL &&
         m_configuration["BlurSharpen"] == NULL &&
         m_configuration["AmbientOcclusion"] == NULL)
         {
         return true;
         }

      int auxbits = 0;
      BoolMap needtex;
      needtex["color"] = true;
      if(configuration["CartoonInk"] != NULL)
         {
         needtex["aux"] = true;
         auxbits |= AuxBitplaneAttrib::ABO_aux_normal;
         }
      if(configuration["AmbientOcclusion"] != NULL)
         {
         needtex["depth"] = true;
         needtex["ssao0"] = true;
         needtex["ssao1"] = true;
         needtex["ssao2"] = true;
         needtex["aux"  ] = true;
         auxbits |= AuxBitplaneAttrib::ABO_aux_normal;
         }
      if(configuration["BlurSharpen"] != NULL)
         {
         needtex["blur0"] = true;
         needtex["blur1"] = true;
         }
      if (configuration["Bloom"] != NULL)
         {
         needtex["bloom0"] = true;
         needtex["bloom1"] = true;
         needtex["bloom2"] = true;
         needtex["bloom3"] = true;
         auxbits |= AuxBitplaneAttrib::ABO_glow;
         }
      if (configuration["ViewGlow"] != NULL)
         {
         auxbits |= AuxBitplaneAttrib::ABO_glow;
         }
      for(BoolMap::const_iterator i = needtex.begin(); i != needtex.end(); ++i)
         {
         const string& tex =  (*i).first;
         m_textures[tex] = new Texture("scene-" + tex);
         m_textures[tex]->set_wrap_u(Texture::WM_clamp);
         m_textures[tex]->set_wrap_v(Texture::WM_clamp);
         // bool neetexpix = true; // Note: not used
         }

      CFilterManager::RenderSceneIntoParameters params;
      params.textures = &m_textures;
      params.auxbits = auxbits;
      m_finalQuad = m_manager.render_scene_into(params);

      if(m_finalQuad.is_empty())
         {
         cleanup();
         return false;
         }

      if(configuration["BlurSharpen"] != NULL)
         {
         PT(Texture) blur0 = m_textures.find("blur0")->second;
         PT(Texture) blur1 = m_textures.find("blur1")->second;
         CFilterManager::RenderQuadIntoParameters params;
         params.colortex = blur0;
         params.div = 2;
         m_blur.push_back(m_manager.render_quad_into(params));
         params = CFilterManager::RenderQuadIntoParameters();
         params.colortex = blur1;
         m_blur.push_back(m_manager.render_quad_into(params));
         m_blur[0].set_shader_input("src", m_textures.find("color")->second);
         m_blur[0].set_shader(load_shader("filter-blurx.sha"));
         m_blur[1].set_shader_input("src", m_textures.find("blur0")->second);
         m_blur[1].set_shader(load_shader("filter-blury.sha"));
         }

      if(configuration["AmbientOcclusion"] != NULL)
         {
         PT(Texture) ssao0 = m_textures.find("ssao0")->second;
         PT(Texture) ssao1 = m_textures.find("ssao1")->second;
         PT(Texture) ssao2 = m_textures.find("ssao2")->second;
         CFilterManager::RenderQuadIntoParameters params;
         params.colortex = ssao0;
         m_ssao.push_back(m_manager.render_quad_into(params));
         params = CFilterManager::RenderQuadIntoParameters();
         params.colortex = ssao1;
         params.div = 2;
         m_ssao.push_back(m_manager.render_quad_into(params));
         params = CFilterManager::RenderQuadIntoParameters();
         params.colortex = ssao2;
         m_ssao.push_back(m_manager.render_quad_into(params));
         m_ssao[0].set_shader_input("depth", m_textures.find("depth")->second);
         m_ssao[0].set_shader_input("normal", m_textures.find("aux")->second);
         m_ssao[0].set_shader_input("random", TexturePool::
                                              load_texture("maps/random.rgb"));
         m_ssao[0].set_shader(load_shader("filter-ssao.sha"));
         m_ssao[1].set_shader_input("src", ssao0);
         m_ssao[1].set_shader(load_shader("filter-blurx.sha"));
         m_ssao[2].set_shader_input("src", ssao1);
         m_ssao[2].set_shader(load_shader("filter-blury.sha"));
         }

      if(configuration["Bloom"] != NULL)
         {
         const BloomConfiguration* bloomconf =
               static_cast<const BloomConfiguration*>(configuration["Bloom"]);
         PT(Texture) bloom0 = m_textures.find("bloom0")->second;
         PT(Texture) bloom1 = m_textures.find("bloom1")->second;
         PT(Texture) bloom2 = m_textures.find("bloom2")->second;
         PT(Texture) bloom3 = m_textures.find("bloom3")->second;
         int scale = 0;
         string downsampler;
         if(bloomconf->size == "large")
            {
            scale = 8;
            downsampler = "filter-down4.sha";
            }
         else if(bloomconf->size == "medium")
            {
            scale = 4;
            downsampler = "filter-copy.sha";
            }
         else
            {
            scale = 2;
            downsampler = "filter-copy.sha";
            }
         CFilterManager::RenderQuadIntoParameters params;
         params.colortex = bloom0;
         params.div = 2;
         params.align = scale;
         m_bloom.push_back(m_manager.render_quad_into(params));
         params = CFilterManager::RenderQuadIntoParameters();
         params.colortex = bloom1;
         params.div = scale;
         params.align = scale;
         m_bloom.push_back(m_manager.render_quad_into(params));
         params = CFilterManager::RenderQuadIntoParameters();
         params.colortex = bloom2;
         params.div = scale;
         params.align = scale;
         m_bloom.push_back(m_manager.render_quad_into(params));
         params = CFilterManager::RenderQuadIntoParameters();
         params.colortex = bloom3;
         params.div = scale;
         params.align = scale;
         m_bloom.push_back(m_manager.render_quad_into(params));
         m_bloom[0].set_shader_input("src", m_textures.find("color")->second);
         m_bloom[0].set_shader(load_shader("filter-bloomi.sha"));
         m_bloom[1].set_shader_input("src", m_textures.find("bloom0")->second);
         m_bloom[1].set_shader(load_shader(downsampler));
         m_bloom[2].set_shader_input("src", m_textures.find("bloom1")->second);
         m_bloom[2].set_shader(load_shader("filter-bloomx.sha"));
         m_bloom[3].set_shader_input("src", m_textures.find("bloom2")->second);
         m_bloom[3].set_shader(load_shader("filter-bloomy.sha"));
         }

      ostringstream text;
      text << "//Cg\n";
      text << "void vshader(float4 vtx_position : POSITION,\n";
      text << " out float4 l_position : POSITION,\n";
      text << " uniform float4 texpad_txcolor,\n";
      text << " uniform float4 texpix_txcolor,\n";
      text << " out float4 l_texcoordC : TEXCOORD0,\n";
      if(configuration["CartoonInk"] != NULL)
         {
         text << " uniform float4 texpad_txaux,\n";
         text << " uniform float4 texpix_txaux,\n";
         text << " out float4 l_texcoordN : TEXCOORD1,\n";
         }
      if(configuration["Bloom"] != NULL)
         {
         text << " uniform float4 texpad_txbloom3,\n";
         text << " out float4 l_texcoordB : TEXCOORD2,\n";
         }
      if(configuration["BlurSharpen"] != NULL)
         {
         text << " uniform float4 texpad_txblur1,\n";
         text << " out float4 l_texcoordBS : TEXCOORD3,\n";
         }
      if(configuration["AmbientOcclusion"] != NULL)
         {
         text << " uniform float4 texpad_txssao2,\n";
         text << " out float4 l_texcoordAO : TEXCOORD4,\n";
         }
      text << " uniform float4x4 mat_modelproj)\n";
      text << "{\n";
      text << " l_position=mul(mat_modelproj, vtx_position);\n";
      text << " l_texcoordC=(vtx_position.xzxz * texpad_txcolor) + texpad_txcolor;\n";
      if(configuration["CartoonInk"] != NULL)
         {
         text << " l_texcoordN=(vtx_position.xzxz * texpad_txaux) + texpad_txaux;\n";
         }
      if(configuration["Bloom"] != NULL)
         {
         text << " l_texcoordB=(vtx_position.xzxz * texpad_txbloom3) + texpad_txbloom3;\n";
         }
      if(configuration["BlurSharpen"] != NULL)
         {
         text << " l_texcoordBS=(vtx_position.xzxz * texpad_txblur1) + texpad_txblur1;\n";
         }
      if(configuration["AmbientOcclusion"] != NULL)
         {
         text << " l_texcoordAO=(vtx_position.xzxz * texpad_txssao2) + texpad_txssao2;\n";
         }
      if(configuration["HalfPixelShift"] != NULL)
         {
         text << " l_texcoordC+=texpix_txcolor*0.5;\n";
         if(configuration["CartoonInk"] != NULL)
            {
            text << " l_texcoordN+=texpix_txaux*0.5;\n";
            }
         }
      text << "}\n";

      text << "void fshader(\n";
      text << "float4 l_texcoordC : TEXCOORD0,\n";
      text << "uniform float4 texpix_txcolor,\n";
      if(configuration["CartoonInk"] != NULL)
         {
         text << "float4 l_texcoordN : TEXCOORD1,\n";
         text << "uniform float4 texpix_txaux,\n";
         }
      if(configuration["Bloom"] != NULL)
         {
         text << "float4 l_texcoordB : TEXCOORD2,\n";
         }
      if(configuration["BlurSharpen"] != NULL)
         {
         text << "float4 l_texcoordBS : TEXCOORD3,\n";
         text << "uniform float4 k_blurval,\n";
         }
      if(configuration["AmbientOcclusion"] != NULL)
         {
         text << "float4 l_texcoordAO : TEXCOORD4,\n";
         }
      for(CFilterManager::TexMap::const_iterator i = m_textures.begin();
            i != m_textures.end(); ++i)
         {
         const string& key = (*i).first;
         text << "uniform sampler2D k_tx" + key + ",\n";
         }
      if(configuration["CartoonInk"] != NULL)
         {
         text << "uniform float4 k_cartoonseparation,\n";
         }
      if(configuration["VolumetricLighting"] != NULL)
         {
         text << "uniform float4 k_casterpos,\n";
         text << "uniform float4 k_vlparams,\n";
         }
      text << "out float4 o_color : COLOR)\n";
      text << "{\n";
      text << " o_color = tex2D(k_txcolor, l_texcoordC.xy);\n";
      if(configuration["CartoonInk"] != NULL)
         {
         text << CARTOON_BODY;
         }
      if(configuration["AmbientOcclusion"] != NULL)
         {
         text << "o_color *= tex2D(k_txssao2, l_texcoordAO.xy).r;\n";
         }
      if(configuration["BlurSharpen"] != NULL)
         {
         text << " o_color = lerp(tex2D(k_txblur1, l_texcoordBS.xy), o_color, k_blurval.x);\n";
         }
      if (configuration["Bloom"] != NULL)
         {
         text << "o_color = saturate(o_color);\n";
         text << "float4 bloom = 0.5*tex2D(k_txbloom3, l_texcoordB.xy);\n";
         text << "o_color = 1-((1-bloom)*(1-o_color));\n";
         }
      if(configuration["ViewGlow"] != NULL)
         {
         text << "o_color.r = o_color.a;\n";
         }
      if(configuration["VolumetricLighting"] != NULL)
         {
         text << "float decay = 1.0f;\n";
         text << "float2 curcoord = l_texcoordC.xy;\n";
         text << "float2 lightdir = curcoord - k_casterpos.xy;\n";
         text << "lightdir *= k_vlparams.x;\n";
         text << "half4 sample = tex2D(k_txcolor, curcoord);\n";
         text << "float3 vlcolor = sample.rgb * sample.a;\n";
         text << "for (int i = 0; i < "
              << static_cast<const VolumetricLightingConfiguration*>
                 (configuration["VolumetricLighting"])->numsamples
              << "; i++) {\n";
         text << "  curcoord -= lightdir;\n";
         text << "  sample = tex2D(k_txcolor, curcoord);\n";
         text << "  sample *= sample.a * decay;//*weight\n";
         text << "  vlcolor += sample.rgb;\n";
         text << "  decay *= k_vlparams.y;\n";
         text << "}\n";
         text << "o_color += float4(vlcolor * k_vlparams.z, 1);\n";
         }
      if(configuration["Inverted"] != NULL)
         {
         text << "o_color = float4(1, 1, 1, 1) - o_color;\n";
         }
      text << "}\n";

      m_finalQuad.set_shader(Shader::make(text.str()));
      for(CFilterManager::TexMap::const_iterator i = m_textures.begin();
            i != m_textures.end(); ++i)
         {
         const string& tex = (*i).first;
         m_finalQuad.set_shader_input("tx"+tex, (*i).second);
         }

      if(AsyncTaskManager::get_global_ptr()->
         find_task("common-filters-update") == NULL)
         {
         AsyncTaskManager::get_global_ptr()->add(m_task);
         }
      }

   if(changed == "CartoonInk" || fullrebuild)
      {
      if(configuration["CartoonInk"] != NULL)
         {
         float separation = static_cast<const CartoonInkConfiguration*>
            (configuration["CartoonInk"])->separation;
         m_finalQuad.set_shader_input("cartoonseparation",
               LVector4f(separation, 0, separation, 0));
         }
      }

   if(changed == "BlurSharpen" || fullrebuild)
      {
      if(configuration["BlurSharpen"] != NULL)
         {
         float blurval = static_cast<const BlurSharpenConfiguration*>
            (configuration["BlurSharpen"])->amount;
         m_finalQuad.set_shader_input("blurval", LVecBase4f(blurval, blurval, blurval, blurval));
         }
      }

   if(changed == "Bloom" || fullrebuild)
      {
      if(configuration["Bloom"] != NULL)
         {
         const BloomConfiguration* bloomconf =
               static_cast<const BloomConfiguration*>(configuration["Bloom"]);
         float intensity = bloomconf->intensity * 3.0;
         m_bloom[0].set_shader_input("blend",
                                     bloomconf->blend[0],
                                     bloomconf->blend[1],
                                     bloomconf->blend[2],
                                     bloomconf->blend[3] * 2.0);
         if(bloomconf->maxtrigger - bloomconf->mintrigger == 0)
            {
            nout << "WARNING: division by 0 for set_shader_input `trigger'"
                 << endl;
            }
         m_bloom[0].set_shader_input("trigger", bloomconf->mintrigger,
               1.0/(bloomconf->maxtrigger-bloomconf->mintrigger), 0.0, 0.0);
         m_bloom[0].set_shader_input("desat", bloomconf->desat);
         m_bloom[3].set_shader_input("intensity", intensity, intensity, intensity, intensity);
         }
      }

   if(changed == "VolumetricLighting" || fullrebuild)
      {
      if(configuration["VolumetricLighting"] != NULL)
         {
         const VolumetricLightingConfiguration* config =
               static_cast<const VolumetricLightingConfiguration*>
               (configuration["VolumetricLighting"]);
         float tcparam = config->density / config->numsamples;
         m_finalQuad.set_shader_input("vlparams",
                                      tcparam,
                                      config->decay,
                                      config->exposure,
                                      0.0);
         }
      }

   if(changed == "AmbientOcclusion" || fullrebuild)
      {
      if(configuration["AmbientOcclusion"] != NULL)
         {
         const AmbientOcclusionConfiguration* config =
               static_cast<const AmbientOcclusionConfiguration*>
               (configuration["AmbientOcclusion"]);
         m_ssao[0].set_shader_input("params1", config->numsamples,
               -config->amount / config->numsamples, config->radius, 0);
         m_ssao[0].set_shader_input("params2", config->strength,
               config->falloff, 0, 0);
         }
      }

   update();
   return true;
   }

// Updates the shader inputs that need to be updated every frame.
// Normally, you shouldn't call this, it's being called in a task.
void CCommonFilters::update()
   {
   if(m_configuration["VolumetricLighting"] != NULL)
      {
      NodePath caster =
            static_cast<VolumetricLightingConfiguration*>
            (m_configuration["VolumetricLighting"])->caster;
      LPoint2f casterpos;
      NodePath cameraNp = m_manager.m_camera;
      PT(Camera) camera = DCAST(Camera, cameraNp.node());
      camera->get_lens()->project(caster.get_pos(cameraNp), casterpos);
      m_finalQuad.set_shader_input("casterpos",
                                   LVector4f(casterpos.get_x() * 0.5 + 0.5,
                                              casterpos.get_y() * 0.5 + 0.5,
                                              0,
                                              0));
      }
   }

bool CCommonFilters::set_cartoon_ink(float separation) // = 1
   {
   CartoonInkConfiguration* config =
         static_cast<CartoonInkConfiguration*>(m_configuration["CartoonInk"]);
   bool fullrebuild = (config == NULL);
   if(fullrebuild)
      {
      config = new CartoonInkConfiguration();
      }
   config->separation = separation;
   m_configuration["CartoonInk"] = config;
   return reconfigure(fullrebuild, "CartoonInk");
   }

bool CCommonFilters::del_cartoon_ink()
   {
   if(m_configuration["CartoonInk"] != NULL)
      {
      delete m_configuration["CartoonInk"];
      m_configuration["CartoonInk"] = NULL;
      return reconfigure(true, "CartoonInk");
      }
   return true;
   }

CCommonFilters::SetBloomParameters::SetBloomParameters()
   : blend(0.3, 0.4, 0.3, 0.0),
     mintrigger(0.6),
     maxtrigger(1),
     desat(0.6),
     intensity(1),
     size("medium")
   {
   // Empty
   }
bool CCommonFilters::set_bloom(const SetBloomParameters& params)
   {
   const LVector4f& blend = params.blend;
   float mintrigger = params.mintrigger;
   float maxtrigger = params.maxtrigger;
   float desat = params.desat;
   float intensity = params.intensity;
   const string& size = params.size;

   if(size == "off")
      {
      return del_bloom();
      }
   if(maxtrigger == 0) { maxtrigger = mintrigger + 0.8; }
   BloomConfiguration* config =
         static_cast<BloomConfiguration*>(m_configuration["Bloom"]);
   bool fullrebuild = true;
   if(config == NULL)
      {
      config = new BloomConfiguration();
      }
   else if(config->size == size)
      {
      fullrebuild = false;
      }
   config->blend = blend;
   config->maxtrigger = maxtrigger;
   config->mintrigger = mintrigger;
   config->desat = desat;
   config->intensity = intensity;
   config->size = size;
   m_configuration["Bloom"] = config;
   return reconfigure(fullrebuild, "Bloom");
   }

bool CCommonFilters::del_bloom()
   {
   if(m_configuration["Bloom"] != NULL)
      {
      delete m_configuration["Bloom"];
      m_configuration["Bloom"] = NULL;
      return reconfigure(true, "Bloom");
      }
   return true;
   }

bool CCommonFilters::set_half_pixel_shift()
   {
   bool fullrebuild = (m_configuration["HalfPixelShift"] == NULL);
   if(fullrebuild)
      {
      m_configuration["HalfPixelShift"] = new BaseConfiguration;
      }
   m_configuration["HalfPixelShift"]->isSet = true;
   return reconfigure(fullrebuild, "HalfPixelShift");
   }

bool CCommonFilters::del_half_pixel_shift()
   {
   if(m_configuration["HalfPixelShift"] != NULL)
      {
      delete m_configuration["HalfPixelShift"];
      m_configuration["HalfPixelShift"] = NULL;
      return reconfigure(true, "HalfPixelShift");
      }
   return true;
   }

bool CCommonFilters::set_view_glow()
   {
   bool fullrebuild = (m_configuration["ViewGlow"] == NULL);
   if(fullrebuild)
      {
      m_configuration["ViewGlow"] = new BaseConfiguration();
      }
   m_configuration["ViewGlow"]->isSet = true;
   return reconfigure(fullrebuild, "ViewGlow");
   }

bool CCommonFilters::del_view_glow()
   {
   if(m_configuration["ViewGlow"] != NULL)
      {
      delete m_configuration["ViewGlow"];
      m_configuration["ViewGlow"] = NULL;
      return reconfigure(true, "ViewGlow");
      }
   return true;
   }

bool CCommonFilters::set_inverted()
   {
   bool fullrebuild = (m_configuration["Inverted"] == NULL);
   if(fullrebuild)
      {
      m_configuration["Inverted"] = new BaseConfiguration();
      }
   m_configuration["Inverted"]->isSet = true;
   return reconfigure(fullrebuild, "Inverted");
   }

bool CCommonFilters::del_inverted()
   {
   if(m_configuration["Inverted"] != NULL)
      {
      delete m_configuration["Inverted"];
      m_configuration["Inverted"] = NULL;
      return reconfigure(true, "Inverted");
      }
   return true;
   }

CCommonFilters::SetVolumetricLightingParameters::
SetVolumetricLightingParameters(NodePath caster)
   : caster(caster),
     numsamples(32),
     density(5),
     decay(0.1),
     exposure(0.1)
   {
   // Empty
   }
bool CCommonFilters::
set_volumetric_lighting(const SetVolumetricLightingParameters& params)
   {
   NodePath caster = params.caster;
   int numsamples = params.numsamples;
   float density = params.density;
   float decay = params.decay;
   float exposure = params.exposure;

   VolumetricLightingConfiguration* config =
         static_cast<VolumetricLightingConfiguration*>
         (m_configuration["VolumetricLighting"]);
   bool fullrebuild = true;
   if(config == NULL)
      {
      config = new VolumetricLightingConfiguration();
      }
   else if(config->caster == caster)
      {
      fullrebuild = false;
      }
   config->caster = caster;
   config->numsamples = numsamples;
   config->density = density;
   config->decay = decay;
   config->exposure = exposure;
   m_configuration["VolumetricLighting"] = config;
   return reconfigure(fullrebuild, "VolumetricLighting");
   }

bool CCommonFilters::del_volumetric_lighting()
   {
   if(m_configuration["VolumetricLighting"] != NULL)
      {
      delete m_configuration["VolumetricLighting"];
      m_configuration["VolumetricLighting"] = NULL;
      return reconfigure(true, "VolumetricLighting");
      }
   return true;
   }

// Enables the blur/sharpen filter. If the 'amount' parameter is 1.0, it will not have effect.
// A value of 0.0 means fully blurred, and a value higher than 1.0 sharpens the image.
bool CCommonFilters::set_blur_sharpen(float amount) // = 0
   {
   BlurSharpenConfiguration* config =
         static_cast<BlurSharpenConfiguration*>(m_configuration["BlurSharpen"]);
   bool fullrebuild = (config == NULL);
   if(fullrebuild)
      {
      config = new BlurSharpenConfiguration();
      }
   config->amount = amount;
   m_configuration["BlurSharpen"] = config;
   return reconfigure(fullrebuild, "BlurSharpen");
   }

bool CCommonFilters::del_blur_sharpen()
   {
   if(m_configuration["BlurSharpen"] != NULL)
      {
      delete m_configuration["BlurSharpen"];
      m_configuration["BlurSharpen"] = NULL;
      return reconfigure(true, "BlurSharpen");
      }
   return true;
   }

CCommonFilters::SetAmbientOcclusionParameters::SetAmbientOcclusionParameters()
   : numsamples(16),
     radius(0.05),
     amount(2),
     strength(0.01),
     falloff(0.000002)
   {
   // Empty
   }
bool CCommonFilters::
set_ambient_occlusion(const SetAmbientOcclusionParameters& params)
   {
   int numsamples = params.numsamples;
   float radius = params.radius;
   float amount = params.amount;
   float strength = params.strength;
   float falloff = params.falloff;

   AmbientOcclusionConfiguration* config =
         static_cast<AmbientOcclusionConfiguration*>
         (m_configuration["AmbientOcclusion"]);
   bool fullrebuild = (config == NULL);
   if(fullrebuild)
      {
      config = new AmbientOcclusionConfiguration();
      }
   config->numsamples = numsamples;
   config->radius = radius;
   config->amount = amount;
   config->strength = strength;
   config->falloff = falloff;
   m_configuration["AmbientOcclusion"] = config;
   return reconfigure(fullrebuild, "AmbientOcclusion");
   }

bool CCommonFilters::del_ambient_occlusion()
   {
   if(m_configuration["AmbientOcclusion"] != NULL)
      {
      delete m_configuration["AmbientOcclusion"];
      m_configuration["AmbientOcclusion"] = NULL;
      return reconfigure(true, "AmbientOcclusion");
      }
   return true;
   }

AsyncTask::DoneStatus CCommonFilters::
call_update(GenericAsyncTask *task, void *user_data)
   {
   // precondition
   if(user_data == NULL)
      {
      nout << "ERROR: parameter user_data cannot be NULL" << endl;
      return AsyncTask::DS_done;
      }
   static_cast<CCommonFilters*>(user_data)->update();
   return AsyncTask::DS_cont;
   }
