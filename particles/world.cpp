/*
 * world.cpp
 *
 *  Created on: 2012-08-23
 *      Author: dri
 */

#include "../p3util/cOnscreenText.h"
#include "../p3util/callbackUtil.h"
#include "ambientLight.h"
#include "directionalLight.h"
#include "pointParticleFactory.h"
#include "spriteParticleRenderer.h"
#include "pointParticleRenderer.h"
#include "discEmitter.h"
#include "sphereSurfaceEmitter.h"
#include "texturePool.h"
#include "linearNoiseForce.h"
#include "linearVectorForce.h"
#include "linearJitterForce.h"
#include "linearCylinderVortexForce.h"
#include "forceNode.h"
#include "linearEulerIntegrator.h"
#include "sphereVolumeEmitter.h"
#include "world.h"

#define WORLD_DEFINE_KEY(event_name, description, func) \
   define_key_t<World, &World::func>(m_windowFramework, \
                                     event_name,        \
                                     description,       \
                                     this)

#define WORLD_ADD_TASK(name, func) \
   add_task_t<World, &World::func>(name, this)

const char* World::HELPTEXT = "\n\
1: Load Steam\n\
2: Load Dust\n\
3: Load Fountain\n\
4: Load Smoke\n\
5: Load Smokering\n\
6: Load Fireish\n\
ESC: Quit\n\
";

World::World(WindowFramework* windowFramework)
   : m_windowFramework(windowFramework),
     m_title(),
     m_escapeEvent(),
     m_t(),
     m_p(),
     m_particleSysMgr(),
     m_physicsMgr()
   {
   // Standard title and instruction text
   COnscreenText title("title");
   title.set_text("Panda3D: Tutorial - Particles");
   title.set_fg(Colorf(1, 1, 1, 1));
   title.set_pos(0.8, -0.95);
   title.set_scale(0.07);
   const NodePath& aspect2d = m_windowFramework->get_aspect_2d();
   title.reparent_to(aspect2d);
   m_title = title.generate();

   COnscreenText escapeEvent("escapeEvent");
   escapeEvent.set_text(HELPTEXT);
   escapeEvent.set_fg(Colorf(1, 1, 1, 1));
   escapeEvent.set_pos(-1.3, 0.95);
   escapeEvent.set_align(TextNode::A_left);
   escapeEvent.set_scale(0.05);
   escapeEvent.reparent_to(aspect2d);
   m_escapeEvent = escapeEvent.generate();

   // More standard initialization
   WORLD_DEFINE_KEY("escape", "sysExit", quit);
   WORLD_DEFINE_KEY("1", "steam", load_particle_config<PC_steam>);
   WORLD_DEFINE_KEY("2", "dust", load_particle_config<PC_dust>);
   WORLD_DEFINE_KEY("3", "fountain", load_particle_config<PC_fountain>);
   WORLD_DEFINE_KEY("4", "smoke", load_particle_config<PC_smoke>);
   WORLD_DEFINE_KEY("5", "smokering", load_particle_config<PC_smokering>);
   WORLD_DEFINE_KEY("6", "fireish", load_particle_config<PC_fireish>);

   // base.disableMouse() // no need in C++
   NodePath camera = m_windowFramework->get_camera_group();
   camera.set_pos(0, -20, 2);
   m_windowFramework->set_background_type(WindowFramework::BT_black);

   // This command is required for Panda to render particles
   // Note: this command, i.e. base.enableParticles() does not exist in C++
   //       and has to be broken down into lower level calls
   WORLD_ADD_TASK("enableParticles", enable_particles);
   m_physicsMgr.attach_linear_integrator(new LinearEulerIntegrator());

   const NodePath&  models =
         m_windowFramework->get_panda_framework()->get_models();
   m_t = m_windowFramework->load_model(models, "../models/teapot");
   m_t.set_pos(0, 10, 0);
   const NodePath& render = m_windowFramework->get_render();
   m_t.reparent_to(render);
   setup_lights();
   m_p = m_t.attach_new_node("particleEffect");
   load_particle_config<PC_steam>();
   }

template<int particleConfig>
void World::load_particle_config(const Event* event)
   {
   m_particleSysMgr.clear();
   m_physicsMgr.clear_angular_forces();
   m_physicsMgr.clear_linear_forces();
   m_physicsMgr.clear_physicals();
   m_p.remove_node();

   PT(ParticleSystem) system = new ParticleSystem();
   PT(ForceNode) forceNode = new ForceNode("force");

   switch(particleConfig)
      {
      case PC_steam:
         load_steam_config(system, forceNode);
         break;
      case PC_dust:
         load_dust_config(system, forceNode);
         break;
      case PC_fountain:
         load_fountain_config(system, forceNode);
         break;
      case PC_smoke:
         load_smoke_config(system, forceNode);
         break;
      case PC_smokering:
         load_smokering_config(system, forceNode);
         break;
      case PC_fireish:
         load_fireish_config(system, forceNode);
         break;
      default:
         nout << "ERROR: unknown particleConfig" << endl;
      }

   // Set up the physical node
   PT(PhysicalNode) physical = new PhysicalNode("particleEffect");
   // let the particle system affect the physicalNode node
   physical->add_physical(system);
   // render the particles at the physicalNode node
   system->set_render_parent(physical);
   // attach the physical node to the teapot
   m_p = m_t.attach_new_node(physical);
   // position it at its spout
   m_p.set_pos(3.000, 0.000, 2.250);
   // attach the force node to the physics node so that it affects only the
   // later
   m_p.attach_new_node(forceNode);

   // register the particle system with the managers
   m_particleSysMgr.attach_particlesystem(system);
   m_physicsMgr.attach_physical(system);
   }

void World::load_steam_config(ParticleSystem* system, ForceNode* forceNode)
   {
   // Settings come from file steam.ptf
   // Particles parameters
   system->set_pool_size(10000);
   system->set_birth_rate(0.2500);
   system->set_litter_size(20);
   system->set_litter_spread(0);
   system->set_system_lifespan(0.0000);
   system->set_local_velocity_flag(true);
   system->set_system_grows_older_flag(false);

   // Factory parameters
   PT(PointParticleFactory) factory = new PointParticleFactory();
   factory->set_lifespan_base(8);
   factory->set_lifespan_spread(0);
   factory->set_mass_base(1);
   factory->set_mass_spread(0);
   factory->set_terminal_velocity_base(400);
   factory->set_terminal_velocity_spread(0);
   system->set_factory(factory);

   // Renderer parameters
   PT(SpriteParticleRenderer) renderer = new SpriteParticleRenderer();
   renderer->set_alpha_mode(BaseParticleRenderer::PR_ALPHA_OUT);
   renderer->set_user_alpha(0.10);
   // Sprite parameters
   renderer->set_texture(TexturePool::load_texture("../steam.png"));
   renderer->set_color(Colorf(1.00, 1.00, 1.00, 1.00));
   renderer->set_x_scale_flag(true);
   renderer->set_y_scale_flag(true);
   renderer->set_anim_angle_flag(false);
   renderer->set_initial_x_scale(0.0025);
   renderer->set_final_x_scale(0.0400);
   renderer->set_initial_y_scale(0.0025);
   renderer->set_final_y_scale(0.0400);
   renderer->set_nonanimated_theta(0.0000);
   renderer->set_alpha_blend_method(BaseParticleRenderer::PP_NO_BLEND);
   renderer->set_alpha_disable(false);
   system->set_renderer(renderer);

   // Emitter parameters
   PT(DiscEmitter) emitter = new DiscEmitter();
   emitter->set_emission_type(BaseParticleEmitter::ET_EXPLICIT);
   emitter->set_amplitude(1.0000);
   emitter->set_amplitude_spread(0.7500);
   emitter->set_offset_force(LVector3f(0.0000, 0.0000, 1.0000));
   emitter->set_explicit_launch_vector(LVector3f(0.0000, 0.0000, 0.0000));
   emitter->set_radiate_origin(LPoint3f(0.0000, 0.0000, 0.0000));
   // Disc parameters
   emitter->set_radius(0.5000);
   system->set_emitter(emitter);

   // Force parameters
   PT(LinearNoiseForce) force = new LinearNoiseForce(0.1500, false);
   force->set_active(true);
   forceNode->add_force(force);
   system->add_linear_force(force);
   }

void World::load_dust_config(ParticleSystem* system, ForceNode* forceNode)
   {
   // Settings come from file dust.ptf
   // Particles parameters
   system->set_pool_size(10000);
   system->set_birth_rate(0.0500);
   system->set_litter_size(10);
   system->set_litter_spread(0);
   system->set_system_lifespan(0.0000);
   system->set_local_velocity_flag(true);
   system->set_system_grows_older_flag(false);

   // Factory parameters
   PT(PointParticleFactory) factory = new PointParticleFactory();
   factory->set_lifespan_base(2.0000);
   factory->set_lifespan_spread(0.2500);
   factory->set_mass_base(2.0000);
   factory->set_mass_spread(0.0100);
   factory->set_terminal_velocity_base(400.0000);
   factory->set_terminal_velocity_spread(0.0000);
   system->set_factory(factory);

   // Renderer parameters
   PT(SpriteParticleRenderer) renderer = new SpriteParticleRenderer();
   renderer->set_alpha_mode(BaseParticleRenderer::PR_ALPHA_OUT);
   renderer->set_user_alpha(0.07);
   // Sprite parameters
   renderer->set_texture(TexturePool::load_texture("../dust.png"));
   renderer->set_color(Colorf(1.00, 1.00, 1.00, 1.00));
   renderer->set_x_scale_flag(false);
   renderer->set_y_scale_flag(false);
   renderer->set_anim_angle_flag(false);
   renderer->set_initial_x_scale(0.0100);
   renderer->set_final_x_scale(0.0200);
   renderer->set_initial_y_scale(0.0100);
   renderer->set_final_y_scale(0.0200);
   renderer->set_nonanimated_theta(0.0000);
   renderer->set_alpha_blend_method(BaseParticleRenderer::PP_BLEND_LINEAR);
   renderer->set_alpha_disable(false);
   system->set_renderer(renderer);

   // Emitter parameters
   PT(SphereVolumeEmitter) emitter = new SphereVolumeEmitter();
   emitter->set_emission_type(BaseParticleEmitter::ET_RADIATE);
   emitter->set_amplitude(1.0000);
   emitter->set_amplitude_spread(0.0000);
   emitter->set_offset_force(LVector3f(0.0000, 0.0000, 0.0000));
   emitter->set_explicit_launch_vector(LVector3f(1.0000, 0.0000, 0.0000));
   emitter->set_radiate_origin(LPoint3f(0.0000, 0.0000, 0.0000));
   // Sphere Volume parameters
   emitter->set_radius(0.1000);
   system->set_emitter(emitter);
   }

void World::load_fountain_config(ParticleSystem* system, ForceNode* forceNode)
   {
   // Settings come from file fountain.ptf
   // Particles parameters
   system->set_pool_size(10000);
   system->set_birth_rate(0.0200);
   system->set_litter_size(300);
   system->set_litter_spread(100);
   system->set_system_lifespan(0.0000);
   system->set_local_velocity_flag(true);
   system->set_system_grows_older_flag(false);

   // Factory parameters
   PT(PointParticleFactory) factory = new PointParticleFactory();
   factory->set_lifespan_base(0.5000);
   factory->set_lifespan_spread(0.2500);
   factory->set_mass_base(2.0000);
   factory->set_mass_spread(0.0100);
   factory->set_terminal_velocity_base(400.0000);
   factory->set_terminal_velocity_spread(0.0000);
   system->set_factory(factory);

   // Renderer parameters
   PT(PointParticleRenderer) renderer = new PointParticleRenderer();
   renderer->set_alpha_mode(BaseParticleRenderer::PR_ALPHA_OUT);
   renderer->set_user_alpha(0.45);
   // Point parameters
   renderer->set_point_size(3.00);
   renderer->set_start_color(Colorf(0.25, 0.90, 1.00, 1.00));
   renderer->set_end_color(Colorf(1.00, 1.00, 1.00, 1.00));
   renderer->set_blend_type(PointParticleRenderer::PP_ONE_COLOR);
   renderer->set_blend_method(BaseParticleRenderer::PP_NO_BLEND);
   system->set_renderer(renderer);

   // Emitter parameters
   PT(DiscEmitter) emitter = new DiscEmitter();
   emitter->set_emission_type(BaseParticleEmitter::ET_CUSTOM);
   emitter->set_amplitude(1.0000);
   emitter->set_amplitude_spread(0.0000);
   emitter->set_offset_force(LVector3f(0.0000, 0.0000, 4.0000));
   emitter->set_explicit_launch_vector(LVector3f(1.0000, 0.0000, 0.0000));
   emitter->set_radiate_origin(LPoint3f(0.0000, 0.0000, 0.0000));
   // Disc parameters
   emitter->set_radius(0.0500);
   emitter->set_outer_angle(356.1859);
   emitter->set_inner_angle(0.0000);
   emitter->set_outer_magnitude(2.0000);
   emitter->set_inner_magnitude(1.0000);
   emitter->set_cubic_lerping(false);
   system->set_emitter(emitter);

   // Force parameters
   PT(LinearVectorForce) force0 = new LinearVectorForce(
         LVector3f(0.0000, 0.0000, -1.0000), 25.0000, 1);
   force0->set_active(true);
   forceNode->add_force(force0);
   system->add_linear_force(force0);

   PT(LinearJitterForce) force1 = new LinearJitterForce(3.0000, 1);
   force1->set_active(true);
   forceNode->add_force(force1);
   system->add_linear_force(force1);
   }

void World::load_smoke_config(ParticleSystem* system, ForceNode* forceNode)
   {
   // Settings come from file smoke.ptf
   // Particles parameters
   system->set_pool_size(10000);
   system->set_birth_rate(0.0500);
   system->set_litter_size(10);
   system->set_litter_spread(0);
   system->set_system_lifespan(0.0000);
   system->set_local_velocity_flag(true);
   system->set_system_grows_older_flag(false);

   // Factory parameters
   PT(PointParticleFactory) factory = new PointParticleFactory();
   factory->set_lifespan_base(2.0000);
   factory->set_lifespan_spread(0.2500);
   factory->set_mass_base(2.0000);
   factory->set_mass_spread(0.0100);
   factory->set_terminal_velocity_base(400.0000);
   factory->set_terminal_velocity_spread(0.0000);
   system->set_factory(factory);

   // Renderer parameters
   PT(SpriteParticleRenderer) renderer = new SpriteParticleRenderer();
   renderer->set_alpha_mode(BaseParticleRenderer::PR_ALPHA_OUT);
   renderer->set_user_alpha(0.13);
   // Sprite parameters
   renderer->set_texture(TexturePool::load_texture("../smoke.png"));
   renderer->set_color(Colorf(1.00, 1.00, 1.00, 1.00));
   renderer->set_x_scale_flag(false);
   renderer->set_y_scale_flag(false);
   renderer->set_anim_angle_flag(false);
   renderer->set_initial_x_scale(0.0100);
   renderer->set_final_x_scale(0.0200);
   renderer->set_initial_y_scale(0.0100);
   renderer->set_final_y_scale(0.0200);
   renderer->set_nonanimated_theta(0.0000);
   renderer->set_alpha_blend_method(BaseParticleRenderer::PP_BLEND_LINEAR);
   renderer->set_alpha_disable(false);
   system->set_renderer(renderer);

   // Emitter parameters
   PT(SphereSurfaceEmitter) emitter = new SphereSurfaceEmitter();
   emitter->set_emission_type(BaseParticleEmitter::ET_RADIATE);
   emitter->set_amplitude(1.0000);
   emitter->set_amplitude_spread(0.0000);
   emitter->set_offset_force(LVector3f(0.0000, 0.0000, 0.0000));
   emitter->set_explicit_launch_vector(LVector3f(1.0000, 0.0000, 0.0000));
   emitter->set_radiate_origin(LPoint3f(0.0000, 0.0000, 0.0000));
   // Sphere Surface parameters
   emitter->set_radius(0.0100);
   system->set_emitter(emitter);
   }

void World::load_smokering_config(ParticleSystem* system, ForceNode* forceNode)
   {
   // Settings come from file smokering.ptf
   // Particles parameters
   system->set_pool_size(1024);
   system->set_birth_rate(0.0200);
   system->set_litter_size(10);
   system->set_litter_spread(0);
   system->set_system_lifespan(1200.0000);
   system->set_local_velocity_flag(true);
   system->set_system_grows_older_flag(false);

   // Factory parameters
   PT(PointParticleFactory) factory = new PointParticleFactory();
   factory->set_lifespan_base(10.0000);
   factory->set_lifespan_spread(0.0000);
   factory->set_mass_base(1.0000);
   factory->set_mass_spread(0.0000);
   factory->set_terminal_velocity_base(400.0000);
   factory->set_terminal_velocity_spread(0.0000);
   system->set_factory(factory);

   // Renderer parameters
   PT(SpriteParticleRenderer) renderer = new SpriteParticleRenderer();
   renderer->set_alpha_mode(BaseParticleRenderer::PR_ALPHA_OUT);
   renderer->set_user_alpha(0.10);
   // Sprite parameters
   renderer->set_texture(TexturePool::load_texture("../smoke.png"));
   renderer->set_color(Colorf(1.00, 1.00, 1.00, 1.00));
   renderer->set_x_scale_flag(1);
   renderer->set_y_scale_flag(1);
   renderer->set_anim_angle_flag(false);
   renderer->set_initial_x_scale(0.0050);
   renderer->set_final_x_scale(0.0400);
   renderer->set_initial_y_scale(0.0100);
   renderer->set_final_y_scale(0.0400);
   renderer->set_nonanimated_theta(0.0000);
   renderer->set_alpha_blend_method(BaseParticleRenderer::PP_NO_BLEND);
   renderer->set_alpha_disable(false);
   system->set_renderer(renderer);

   // Emitter parameters
   PT(DiscEmitter) emitter = new DiscEmitter();
   emitter->set_emission_type(BaseParticleEmitter::ET_RADIATE);
   emitter->set_amplitude(1.0000);
   emitter->set_amplitude_spread(0.0000);
   emitter->set_offset_force(LVector3f(0.1000, 0.0000, 0.1000));
   emitter->set_explicit_launch_vector(LVector3f(1.0000, 0.0000, 0.0000));
   emitter->set_radiate_origin(LPoint3f(0.0000, 0.0000, 0.0000));
   // Disc parameters
   emitter->set_radius(0.5000);
   system->set_emitter(emitter);

   // Force parameters
   PT(LinearCylinderVortexForce) force0 = new LinearCylinderVortexForce(
         4.0000, 1.0000, 1.0000, 1.0000, 0);
   force0->set_active(true);
   forceNode->add_force(force0);
   system->add_linear_force(force0);

   PT(LinearVectorForce) force1 = new LinearVectorForce(
         LVector3f(0.0000, 0.0000, 1.0000), 0.0500, 0);
   force1->set_active(true);
   forceNode->add_force(force1);
   system->add_linear_force(force1);
   }

void World::load_fireish_config(ParticleSystem* system, ForceNode* forceNode)
   {
   // Settings come from file fireish.ptf
   // Particles parameters
   system->set_pool_size(1024);
   system->set_birth_rate(0.0200);
   system->set_litter_size(10);
   system->set_litter_spread(0);
   system->set_system_lifespan(1200.0000);
   system->set_local_velocity_flag(true);
   system->set_system_grows_older_flag(false);

   // Factory parameters
   PT(PointParticleFactory) factory = new PointParticleFactory();
   factory->set_lifespan_base(0.5000);
   factory->set_lifespan_spread(0.0000);
   factory->set_mass_base(1.0000);
   factory->set_mass_spread(0.0000);
   factory->set_terminal_velocity_base(400.0000);
   factory->set_terminal_velocity_spread(0.0000);
   system->set_factory(factory);

   // Renderer parameters
   PT(SpriteParticleRenderer) renderer = new SpriteParticleRenderer();
   renderer->set_alpha_mode(BaseParticleRenderer::PR_ALPHA_OUT);
   renderer->set_user_alpha(0.22);
   // Sprite parameters
   renderer->set_texture(TexturePool::load_texture("../sparkle.png"));
   renderer->set_color(Colorf(1.00, 1.00, 1.00, 1.00));
   renderer->set_x_scale_flag(true);
   renderer->set_y_scale_flag(true);
   renderer->set_anim_angle_flag(false);
   renderer->set_initial_x_scale(0.0050);
   renderer->set_final_x_scale(0.0200);
   renderer->set_initial_y_scale(0.0100);
   renderer->set_final_y_scale(0.0200);
   renderer->set_nonanimated_theta(0.0000);
   renderer->set_alpha_blend_method(BaseParticleRenderer::PP_NO_BLEND);
   renderer->set_alpha_disable(false);
   system->set_renderer(renderer);

   // Emitter parameters
   PT(DiscEmitter) emitter = new DiscEmitter();
   emitter->set_emission_type(BaseParticleEmitter::ET_RADIATE);
   emitter->set_amplitude(1.0000);
   emitter->set_amplitude_spread(0.0000);
   emitter->set_offset_force(LVector3f(0.0000, 0.0000, 3.0000));
   emitter->set_explicit_launch_vector(LVector3f(1.0000, 0.0000, 0.0000));
   emitter->set_radiate_origin(LPoint3f(0.0000, 0.0000, 0.0000));
   // Disc parameters
   emitter->set_radius(0.5000);
   system->set_emitter(emitter);
   }

// Setup lighting
void World::setup_lights()
   {
   PT(AmbientLight) ambientLight = new AmbientLight("ambientLight");
   ambientLight->set_color(Colorf(0.4, 0.4, 0.35, 1));
   PT(DirectionalLight) directionalLight =
         new DirectionalLight("directionalLight");
   directionalLight->set_direction(LVector3f(0, 8, -2.5));
   directionalLight->set_color(Colorf(0.9, 0.8, 0.9, 1));
   // Set lighting on teapot so steam doesn't get affected
   m_t.set_light(m_t.attach_new_node(directionalLight));
   m_t.set_light(m_t.attach_new_node(ambientLight));
   }

void World::quit(const Event* event)
   {
   m_windowFramework->get_panda_framework()->set_exit_flag();
   }

AsyncTask::DoneStatus World::enable_particles(GenericAsyncTask *task)
   {
   const double dt = ClockObject::get_global_clock()->get_dt();
   m_particleSysMgr.do_particles(dt);
   m_physicsMgr.do_physics(dt);

   return AsyncTask::DS_cont;
   }
