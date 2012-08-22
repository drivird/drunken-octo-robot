/*
 * world.cpp
 *
 *  Created on: 2012-07-26
 *      Author: dri
 */

#include "../p3util/cOnscreenText.h"
#include "../p3util/callbackUtil.h"
#include "cIntervalManager.h"
#include "world.h"

// Note: These macros lighten the call to template functions
#define WORLD_DEFINE_KEY(event_name, description, func) \
   define_key_t<World, &World::func>(m_windowFramework, \
                                     event_name,        \
                                     description,       \
                                     this)

#define WORLD_ADD_HOOK(event_name, func) \
   add_hook_t<World, &World::func>(event_name, this)

#define TOGGLE_FOG(r, g, b) \
   set_fog_color<r, g, b>

#define SET_BACKGROUND_COLOR(r, g, b) \
   set_background_color<r, g, b>

// Global variables for the tunnel dimensions and speed of travel
const int World::TUNNEL_SEGMENT_LENGTH = 50;
const int World::TUNNEL_TIME = 2;   // Amount of time for one segment to travel
                                    // the distance of TUNNEL_SEGMENT_LENGTH

// Macro-like function used to reduce the amount to code needed to create the
// on screen instructions
// Function used to reduce the amount to code needed to create the
// on screen instructions
NodePath World::gen_label_text(const string& text, int i) const
   {
   COnscreenText label("label");
   label.set_text(text);
   label.set_pos(LVecBase2f(-1.3, 0.95-0.06*i));
   label.set_fg(Colorf(1, 1, 1, 1));
   label.set_align(TextNode::A_left);
   label.set_scale(0.05);
   label.reparent_to(m_windowFramework->get_aspect_2d());
   return label.generate();
   }

World::World(WindowFramework* windowFramework)
   : m_windowFramework(windowFramework),
     m_title(),
     m_escapeEventText(),
     m_pkeyEventText(),
     m_tkeyEventText(),
     m_dkeyEventText(),
     m_sdkeyEventText(),
     m_rkeyEventText(),
     m_srkeyEventText(),
     m_bkeyEventText(),
     m_sbkeyEventText(),
     m_gkeyEventText(),
     m_sgkeyEventText(),
     m_lkeyEventText(),
     m_slkeyEventText(),
     m_pluskeyEventText(),
     m_minuskeyEventText(),
     m_fog(NULL),
     // m_tunnel[4]
     m_tunnelMove(NULL)
   {
   // precondition
   if(windowFramework == NULL)
      {
      nout << "ERROR: parameter windowFramework cannot be NULL." << endl;
      return;
      }

   //// Standard initialization stuff
   // Standard title that's on screen in every tutorial
   COnscreenText title = COnscreenText("title", COnscreenText::TS_plain);
   title.set_text("Panda3D: Tutorial - Fog");
   title.set_fg(Colorf(1,1,1,1));
   title.set_pos(0.9,-0.95);
   title.set_scale(0.07);
   m_title = title.generate();
   NodePath aspect2d = m_windowFramework->get_aspect_2d();
   m_title.reparent_to(aspect2d);

   // Code to generate the on screen instructions
   m_escapeEventText = gen_label_text("ESC: Quit", 0);
   m_pkeyEventText = gen_label_text("[P]: Pause", 1);
   m_tkeyEventText = gen_label_text("[T]: Toggle Fog", 2);
   m_dkeyEventText = gen_label_text("[D]: Make fog color black", 3);
   m_sdkeyEventText = gen_label_text(
     "[SHIFT+D]: Make background color black", 4);
   m_rkeyEventText = gen_label_text("[R]: Make fog color red", 5);
   m_srkeyEventText = gen_label_text(
     "[SHIFT+R]: Make background color red", 6);
   m_bkeyEventText = gen_label_text("[B]: Make fog color blue", 7);
   m_sbkeyEventText = gen_label_text(
     "[SHIFT+B]: Make background color blue", 8);
   m_gkeyEventText = gen_label_text("[G]: Make fog color green", 9);
   m_sgkeyEventText = gen_label_text(
     "[SHIFT+G]: Make background color green", 10);
   m_lkeyEventText = gen_label_text(
     "[L]: Make fog color light grey", 11);
   m_slkeyEventText = gen_label_text(
     "[SHIFT+L]: Make background color light grey", 12);
   m_pluskeyEventText = gen_label_text("[+]: Increase fog density", 13);
   m_minuskeyEventText = gen_label_text("[-]: Decrease fog density", 14);

   // disable mouse control so that we can place the camera
   // base.disableMouse() // Note: not needed in C++
   NodePath camera = m_windowFramework->get_camera_group();
   camera.set_pos_hpr(0, 0, 10, 0, -90, 0);
   // set the background color to black
   set_background_color<0, 0, 0>(NULL);

   ////World specific-code

   // Create an instance of fog called 'distanceFog'.
   // 'distanceFog' is just a name for our fog, not a specific type of fog.
   m_fog = new Fog("distanceFog");
   // Set the initial color of our fog to black.
   m_fog->set_color(0, 0, 0);
   // Set the density/falloff of the fog.  The range is 0-1.
   // The higher the numer, the "bigger" the fog effect.
   m_fog->set_exp_density(0.08);
   // We will set fog on render which means that everything in our scene will
   // be affected by fog. Alternatively, you could only set fog on a specific
   // object/node and only it and the nodes below it would be affected by
   // the fog.
   NodePath render = m_windowFramework->get_render();
   render.set_fog(m_fog);

   // Define the keyboard input
   // Escape closes the demo
   WORLD_DEFINE_KEY("escape", "quit", quit);
   // Handle pausing the tunnel
   WORLD_DEFINE_KEY("p", "handlePause", handle_pause);
   // Handle turning the fog on and off
   WORLD_DEFINE_KEY("t", "ToggleFog", toggle_fog);
   // Sets keys to set the fog to various colors
   WORLD_DEFINE_KEY("r", "setFogColorRed", TOGGLE_FOG(10, 0, 0));
   WORLD_DEFINE_KEY("g", "setFogColorGreen", TOGGLE_FOG(0, 10, 0));
   WORLD_DEFINE_KEY("b", "setFogColorBlue", TOGGLE_FOG(0, 0, 10));
   WORLD_DEFINE_KEY("l", "setFogColorGray", TOGGLE_FOG(7, 7, 7));
   WORLD_DEFINE_KEY("d", "setFogColorBlack", TOGGLE_FOG(0, 0, 0));
   // Sets keys to change the background colors
   WORLD_DEFINE_KEY("shift-r", "setBackgroundRed",
      SET_BACKGROUND_COLOR(10, 0, 0));
   WORLD_DEFINE_KEY("shift-g", "setBackgroundGreen",
      SET_BACKGROUND_COLOR(0, 10, 0));
   WORLD_DEFINE_KEY("shift-b", "setBackgroundBlue",
      SET_BACKGROUND_COLOR(0, 0, 10));
   WORLD_DEFINE_KEY("shift-l", "setBackgroundGray",
      SET_BACKGROUND_COLOR(7, 7, 7));
   WORLD_DEFINE_KEY("shift-d", "setBackgroundBlack",
      SET_BACKGROUND_COLOR(0, 0, 0));
   // Increases the fog density when "+" key is pressed
   WORLD_DEFINE_KEY("+", "incFogDensity1", add_fog_density<1>);
   // This is to handle the other "+" key (it"s over = on the keyboard);
   WORLD_DEFINE_KEY("=", "incFogDensity2", add_fog_density<1>);
   WORLD_DEFINE_KEY("shift-=", "incFogDensity3", add_fog_density<1>);
   // Decreases the fog density when the "-" key is pressed
   WORLD_DEFINE_KEY("-", "decFogDensity", add_fog_density<-1>);

   // Load the tunel and start the tunnel
   init_tunnel();
   cont_tunnel(NULL);

   // Note: setup a task to step the interval manager
   AsyncTaskManager::get_global_ptr()->add(
      new GenericAsyncTask("intervalManagerTask", step_interval_manager, this));

   // Note: setup an event handler to catch the "contTunnel" event
   WORLD_ADD_HOOK("contTunnel", cont_tunnel);
   }

// This function will change the fog density by the amount passed into it
// This function is needed so that it can look up the current value and
// change it when the key is pressed. If you wanted to bind a key to set it
// at a given value you could call m_fog.setExpDensity directly
template<int value>
void World::add_fog_density(const Event* event)
   {
   // The min() statement makes sure the density is never over 1
   // The max() statement makes sure the density is never below 0
   const float change = 0.01*value;
   m_fog->set_exp_density(
      min(1.0f, max(0.0f, m_fog->get_exp_density() + change)));
   }

// Code to initialize the tunnel
void World::init_tunnel()
   {
   NodePath models = m_windowFramework->get_panda_framework()->get_models();
   NodePath render = m_windowFramework->get_render();
   // Creates the list [None, None, None, None]
   for(int x = 0; x < 4; ++x)
      {
      // Load a copy of the tunnel
      m_tunnel[x] = m_windowFramework->load_model(models, "../models/tunnel");
      // The front segment needs to be attached to render
      if(x == 0)
         {
         m_tunnel[x].reparent_to(render);
         }
      // The rest of the segments parent to the previous one, so that by moving
      // the front segment, the entire tunnel is moved
      else
         {
         m_tunnel[x].reparent_to(m_tunnel[x-1]);
         }
      // We have to offset each segment by its length so that they stack onto
      // each other. Otherwise, they would all occupy the same space.
      m_tunnel[x].set_pos(0, 0, -TUNNEL_SEGMENT_LENGTH);
      // Now we have a tunnel consisting of 4 repeating segments with a
      // hierarchy like this:
      // render<-tunnel[0]<-tunnel[1]<-tunnel[2]<-tunnel[3]
      }
   }

// This function is called to snap the front of the tunnel to the back
// to simulate travelling through it
void World::cont_tunnel(const Event* event)
   {
   // This line uses slices to take the front of the list and put it on the
   // back. For more information on slices check the Python manual
   // Note: plain C++ loop will do the job
   NodePath tmp = m_tunnel[0];
   for(int i = 0; i < 3; ++i)
      {
      m_tunnel[i] = m_tunnel[i+1];
      }
   m_tunnel[3] = tmp;
   // Set the front segment (which was at TUNNEL_SEGMENT_LENGTH) to 0, which
   // is where the previous segment started
   m_tunnel[0].set_z(0);
   // Reparent the front to render to preserve the hierarchy outlined above
   NodePath render = m_windowFramework->get_render();
   m_tunnel[0].reparent_to(render);
   // Set the scale to be appropriate (since attributes like scale are
   // inherited, the rest of the segments have a scale of 1)
   m_tunnel[0].set_scale(0.155, 0.155, 0.305);
   // Set the new back to the values that the rest of teh segments have
   m_tunnel[3].reparent_to(m_tunnel[2]);
   m_tunnel[3].set_z(-TUNNEL_SEGMENT_LENGTH);
   m_tunnel[3].set_scale(1);

   // Set up the tunnel to move one segment and then call contTunnel again
   // to make the tunnel move infinitely
   m_tunnelMove = new CLerpNodePathInterval("tunnelMove",
                                            TUNNEL_TIME,
                                            CLerpInterval::BT_no_blend,
                                            false,
                                            true,
                                            m_tunnel[0],
                                            NodePath());
   m_tunnelMove->set_start_pos(LVecBase3f(0, 0, 0));
   m_tunnelMove->set_end_pos(LVecBase3f(0, 0, TUNNEL_SEGMENT_LENGTH*0.305));
   m_tunnelMove->set_done_event("contTunnel");
   m_tunnelMove->start();
   }

// This function calls toggle interval to pause or unpause the tunnel.
// Like addFogDensity, ToggleInterval could not be passed directly in the
// accept command since the value of self.tunnelMove changes whenever
// self.contTunnel is called
void World::handle_pause(const Event* event)
   {
   // Note: ToggleInterval seems redundant so it is ignored.
   if(m_tunnelMove->is_playing())
      {
      m_tunnelMove->pause();
      }
   else
      {
      m_tunnelMove->resume();
      }
   }

// This function will toggle any interval passed to it between playing and paused
// def ToggleInterval(interval):
//   if interval.isPlaying(): interval.pause()
//   else: interval.resume()

// This function will toggle fog on a node
void World::toggle_fog(const Event* event)
   {
   // If the fog attached to the node is equal to the one we passed in, then
   // fog is on and we should clear it
   NodePath render = m_windowFramework->get_render();
   if(render.get_fog() == m_fog)
      {
      render.clear_fog();
      }
   // Otherwise fog is not set so we should set it
   else
      {
      render.set_fog(m_fog);
      }
   }

template<int r, int g, int b>
void World::set_fog_color(const Event* event)
   {
   m_fog->set_color(0.1*r, 0.1*g, 0.1*b);
   }

template<int r, int g, int b>
void World::set_background_color(const Event* event)
   {
   m_windowFramework->get_display_region_3d()->
         set_clear_color(Colorf(0.1*r, 0.1*g, 0.1*b ,1));
   }

void World::quit(const Event* event)
   {
   m_windowFramework->get_panda_framework()->set_exit_flag();
   }

AsyncTask::DoneStatus World::step_interval_manager(GenericAsyncTask* taskPtr, void* dataPtr)
   {
   CIntervalManager::get_global_ptr()->step();
   return AsyncTask::DS_cont;
   }
