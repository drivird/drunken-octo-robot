/*
 * world.cpp
 *
 *  Created on: 2012-07-31
 *      Author: dri
 */

#include "../p3util/cOnscreenText.h"
#include "../p3util/callbackUtil.h"
#include "cIntervalManager.h"
#include "mouseButton.h"
#include "world.h"

// Note: These macros lighten the call to template functions
#define WORLD_DEFINE_KEY(event_name, description, func) \
   define_key_t<World, &World::func>(m_windowFramework, \
                                     event_name,        \
                                     description,       \
                                     this)

#define WORLD_ADD_TASK(name, func) \
   add_task_t<World, &World::func>(name, this)

World::World(WindowFramework* windowFramework)
   : m_windowFramework(windowFramework),
     m_title(),
     m_escapeEventText(),
     m_musicManager(AudioManager::create_AudioManager()),
     m_musicBoxSound(m_musicManager->get_sound("../music/musicbox.ogg")),
     m_musicTime(0),
     m_sfxManager(AudioManager::create_AudioManager()),
     m_lidSfx(m_sfxManager->get_sound("../music/openclose.ogg")),
     m_lidOpenSfx(NULL),
     m_lidCloseSfx(NULL),
     m_sliderText(),
     m_slider(),
     m_buttonOpen(),
     m_boxOpen(false),
     m_musicBox(),
     m_Lid(),
     m_Panda(),
     m_HingeNode(),
     m_lidClose(NULL),
     m_lidOpen(NULL),
     m_PandaTurn(NULL)
   {
   // Our standard title and instructions text
   COnscreenText title("title", COnscreenText::TS_plain);
   title.set_text("Panda3D: Tutorial - Musicbox(sounds)");
   title.set_fg(Colorf(1, 1, 1, 1));
   title.set_pos(0.7, -0.95);
   title.set_scale(0.07);
   m_title = title.generate();
   const NodePath& aspect2d = m_windowFramework->get_aspect_2d();
   m_title.reparent_to(aspect2d);
   COnscreenText escapeEventText("escape", COnscreenText::TS_plain);
   escapeEventText.set_text("ESC: Quit");
   escapeEventText.set_fg(Colorf(1, 1, 1, 1));
   escapeEventText.set_pos(-1.3, 0.95);
   escapeEventText.set_align(TextNode::A_left);
   escapeEventText.set_scale(0.05);
   m_escapeEventText = escapeEventText.generate();
   m_escapeEventText.reparent_to(aspect2d);

   // Set up the key input
   WORLD_DEFINE_KEY("escape", "quit", quit);

   // Fix the camera position
   // base.disableMouse() // Note: no need to disable the mouse in C++

   // Loading sounds is done in a similar way to loading other things
   // Loading the main music box song
   // Note: function base.loadMusic is python only. In C++ we need to manage
   //       the AudioManager ourselves. Read the manual or look for
   //       self.sfxManagerList in file ShowBase.py for more info.
   //       The procedure is like this:
   //       *) configure which audio library to use (*optional)
   //       1) create the manager
   //       2) load the file
   //       3) update the manager using a task
   //       4) shutdown the manager when you are done
   // ensure only 1 midi song is playing at a time
   m_musicManager->set_concurrent_sound_limit(1);
   m_musicBoxSound->set_volume(0.5);   // Volume is a percentage from 0 to 1
   m_musicBoxSound->set_loop_count(0); // 0 means loop forever, 1 (default) means
                                       // play once. 2 or higher means play that
                                       // many times

   // Sound objects do not have a pause function, just play and stop. So we will
   // Use this variable to keep track of where the sound is at when it was stopped
   // to impliment pausing
   m_musicTime = 0;

   // Loading the open/close effect
   // loadSFX and loadMusic are identical. They are often used for organization
   // (loadMusic is used for background music, loadSfx is used for other effects)
   // The open/close file has both effects in it. Fortunatly we can use intervals
   // to easily define parts of a sound file to play

   m_lidOpenSfx = new CSoundInterval(
      CSoundInterval::Parameters("lidOpenSfx", m_lidSfx)
         .duration(2).start_time(0));

   m_lidCloseSfx = new CSoundInterval(
      CSoundInterval::Parameters("lidCloseSfx", m_lidSfx)
         .start_time(5));

   WORLD_ADD_TASK("worldAsyncTask", async_task);

    // For this tutorial, it seemed appropriate to have on screen controls. The
    // following code creates them
    // This is a label for a slider
    COnscreenText sliderText = COnscreenText("sliderText");
    sliderText.set_text("Volume");
    sliderText.set_fg(Colorf(1, 1, 1, 1));
    sliderText.set_pos(0, 0.8);
    sliderText.set_scale(0.07);
    m_sliderText = sliderText.generate();
    m_sliderText.reparent_to(aspect2d);

    // The slider itself. It calls self.set_music_box_volume when changed
    PT(PGSliderBar) slider = new PGSliderBar("volume");
    slider->set_value(0.50);
    // Note: these are the default values of python class DirectSlider
    const bool vertical = false;
    const float length = 2;
    const float width = 0.16;
    const float bevel = 0.1;
    slider->setup_slider(vertical, length, width, bevel);
    // Note: manually adjust thumb button to look like in the python tutorial
    //       but first disable pieces management or else the slider will
    //       override any changes to the thumb's frame.
    slider->set_manage_pieces(false);
    PT(PGButton) thumb = slider->get_thumb_button();
    thumb->set_frame(-length*0.025, length*0.025, -width*0.5, width*0.5);
    m_slider = aspect2d.attach_new_node(slider);
    m_slider.set_pos(0, 0, 0.7);

    const string& slider_event = slider->get_adjust_event();
    WORLD_DEFINE_KEY(slider_event, "setMusicBoxVolume", set_music_box_volume);

    // A button that calls self.toggleMusicBox when pressed
    // Note: python class DirectButton has a neat feature that allows us to
    //       change its label at run time. In the original python tutorial it
    //       used to rename the button from "Open Box" to "Close Box" and
    //       inversely. To duplicate this behavior, we create two distinct
    //       buttons and render them in turn.
    string button_event;
    m_buttonOpen = MakeButton("OpenBox", "Open Box", &button_event);
    WORLD_DEFINE_KEY(button_event, "toggleMusicBox", toggle_music_box);

    m_buttonClose = MakeButton("CloseBox", "Close Box", &button_event);
    m_buttonClose.detach_node();
    WORLD_DEFINE_KEY(button_event, "toggleMusicBox", toggle_music_box);

    // A variable to represent the state of the simulation. It starts closed
    m_boxOpen = false;

    // Here we load and set up the music box. It was modeled in a complex way, so
    // setting it up will be complicated
    const NodePath& models = m_windowFramework->get_panda_framework()->get_models();
    m_musicBox = m_windowFramework->load_model(models, "../models/MusicBox");
    m_musicBox.set_pos(0, 60, -10);
    const NodePath& render = m_windowFramework->get_render();
    m_musicBox.reparent_to(render);
    // Just like the scene graph contains hierarchies of nodes, so can
    // models. You can get the NodePath for the node using the find
    // function, and then you can animate the model by moving its parts
    // To see the hierarchy of a model, use, the ls function
    // m_musicBox.ls() prints out the entire hierarchy of the model

    // Finding pieces of the model
    m_Lid = m_musicBox.find("**/lid");
    m_Panda = m_musicBox.find("**/turningthing");

    // This model was made with the hinge in the wrong place
    // this is here so we have something to turn
    m_HingeNode = m_musicBox.find("**/box").attach_new_node("nHingeNode");
    m_HingeNode.set_pos(0.8659, 6.5, 5.4);
    // WRT - ie with respect to. Reparents the object without changing
    // its position, size, or orientation
    m_Lid.wrt_reparent_to(m_HingeNode);
    m_HingeNode.set_hpr(0, 90, 0);

    // This sets up an interval to play the close sound and actually close the box
    // at the same time.
    const bool bakeInStart = true;
    const bool fluid = false;
    PT(CLerpNodePathInterval) lerpLidClose = new CLerpNodePathInterval(
       "lerpLidClose", 2, CLerpInterval::BT_ease_in_out, bakeInStart, fluid,
       m_HingeNode, NodePath());
    lerpLidClose->set_start_hpr(
       LVecBase3f(m_HingeNode.get_h(), 0, m_HingeNode.get_r()));
    lerpLidClose->set_end_hpr(
       LVecBase3f(m_HingeNode.get_h(), 90, m_HingeNode.get_r()));

    m_lidClose = new CMetaInterval("lidClose");
    m_lidClose->add_c_interval(m_lidCloseSfx);
    m_lidClose->add_c_interval(
       lerpLidClose, 0, CMetaInterval::RS_level_begin);

    PT(CLerpNodePathInterval) lerpLidOpen = new CLerpNodePathInterval(
       "lerpLidOpen", 2, CLerpInterval::BT_ease_in_out, bakeInStart, fluid,
       m_HingeNode, NodePath());
    lerpLidOpen->set_start_hpr(
       LVecBase3f(m_HingeNode.get_h(), 90, m_HingeNode.get_r()));
    lerpLidOpen->set_end_hpr(
       LVecBase3f(m_HingeNode.get_h(), 0, m_HingeNode.get_r()));

    m_lidOpen = new CMetaInterval("lidOpen");
    m_lidOpen->add_c_interval(m_lidOpenSfx);
    m_lidOpen->add_c_interval(
       lerpLidOpen, 0, CMetaInterval::RS_level_begin);

    // The interval for turning the panda
    m_PandaTurn = new CLerpNodePathInterval("pandaTurn", 7,
       CLerpInterval::BT_no_blend, bakeInStart, fluid, m_Panda, NodePath());
    m_PandaTurn->set_start_hpr(m_Panda.get_hpr());
    m_PandaTurn->set_end_hpr(
       LVecBase3f(m_Panda.get_h()+360, m_Panda.get_p(), m_Panda.get_r()));
    // Do a quick loop and pause to set it as a looping interval so it can be
    // started with resume and loop properly
    m_PandaTurn->loop();
    m_PandaTurn->pause();
   }

void World::set_music_box_volume(const Event* event)
   {
   // Simply reads the current value from the slider and sets it in the sound
   const float newVol = DCAST(PGSliderBar, m_slider.node())->get_value();
   m_musicBoxSound->set_volume(newVol);
   }

void World::toggle_music_box(const Event* event)
   {
   const NodePath& aspect2d = m_windowFramework->get_aspect_2d();
   if(m_boxOpen)
      {
      // close the box
      m_lidClose->start();              // Start the close box interval
      m_PandaTurn->pause();             // Pause the figurine turning

      // Save the current time of the music box song
      m_musicTime = m_musicBoxSound->get_time();
      m_musicBoxSound->stop();          // Stop the music box song

      // Note: show the `Open Box' button.
      m_buttonOpen.reparent_to(aspect2d);
      m_buttonClose.detach_node();
      }
   else
      {
      // open the box
      m_lidOpen->start();               // Start the open box interval
      m_PandaTurn->resume();            // Resume the figuring turning

      // Reset the time of the music box song so it starts where it left off
      m_musicBoxSound->set_time(m_musicTime);
      m_musicBoxSound->play();          // Play the music box song

      // Note: show the `Close Box' button.
      m_buttonOpen.detach_node();
      m_buttonClose.reparent_to(aspect2d);
      }

   m_boxOpen = !m_boxOpen;              // Set our state to opposite what it was
                                        // (closed to open or open to closed)
   }

void World::quit(const Event* event)
   {
   m_windowFramework->get_panda_framework()->set_exit_flag();
   }

AsyncTask::DoneStatus World::async_task(GenericAsyncTask *task)
   {
   m_musicManager->update();
   m_sfxManager->update();
   CIntervalManager::get_global_ptr()->step();

   return AsyncTask::DS_cont;
   }

NodePath World::MakeButton(const string& name,
                           const string& label,
                           string* event_name) const
   {
   PT(PGButton) button = new PGButton(name);
   const float bevel = 0.1;
   button->setup(label, bevel);
   const NodePath& aspect2d = m_windowFramework->get_aspect_2d();
   NodePath buttonNp = aspect2d.attach_new_node(button);
   buttonNp.set_scale(0.1);
   // Note: PGButton's setup is quite different from DirectButton's so just
   //       tweak PGButton's position and frame size until it looks like
   //       the button of the original python tutorial.
   buttonNp.set_pos(0.48, 0, 0.7);
   LVecBase4f frame = button->get_frame();
   button->set_frame(frame[0]-0.15,
                     frame[1]+0.35,
                     frame[2]-0.63,
                     frame[3]+0.37);

   *event_name = button->get_click_event(MouseButton::one());
   return buttonNp;
   }
