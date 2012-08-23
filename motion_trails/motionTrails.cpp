/*
 * motionTrails.cpp
 *
 *  Created on: 2012-08-18
 *      Author: dri
 */

#include "../p3util/cOnscreenText.h"
#include "../p3util/callbackUtil.h"
#include "orthographicLens.h"
#include "cardMaker.h"
#include "motionTrails.h"

// Note: These macros lighten the call to template functions
#define MT_DEFINE_KEY(event_name, description, func) \
   define_key_t<MotionTrails, &MotionTrails::func>(  \
      m_windowFramework,                             \
      event_name,                                    \
      description,                                   \
      this)

#define MT_ADD_TASK(name, func) \
   add_task_t<MotionTrails, &MotionTrails::func>(name, this)

NodePath MotionTrails::add_instructions(float pos, const string& msg) const
   {
   COnscreenText instruction("instructions");
   instruction.set_text(msg);
   instruction.set_fg(Colorf(1, 1, 1, 1));
   instruction.set_pos(LVecBase2f(-1.3, pos));
   instruction.set_align(TextNode::A_left);
   instruction.set_scale(0.05);
   instruction.reparent_to(m_windowFramework->get_aspect_2d());
   return instruction.generate();
   }

MotionTrails::MotionTrails(WindowFramework* windowFramework)
   : m_windowFramework(windowFramework),
     m_tex(new Texture()),
     m_backcam(),
     m_background("background"),
     m_bcard(),
     m_fcard(),
     m_clickrate(0),
     m_nextclick(0),
     m_title(),
     m_instr0(),
     m_instr1(),
     m_instr2(),
     m_instr3(),
     m_instr4(),
     m_instr5()
   {
   // create a texture into which we can copy the main window.
   m_tex->set_minfilter(Texture::FT_linear);
   m_windowFramework->get_graphics_window()->add_render_texture(
         m_tex, GraphicsOutput::RTM_triggered_copy_texture);

   // Create another 2D camera. Tell it to render before the main camera.

   // Note: First, create a 2D camera using an orthographic lens and let its
   //       film dimension respect other 2D renderer like render2d and aspect2d
   //       Refer to WindowFramework::get_render_2d for more info.
   const float left = -1.0f;
   const float right = 1.0f;
   const float bottom = -1.0f;
   const float top = 1.0f;
   PT(Lens) lens = new OrthographicLens;
   lens->set_film_size(right - left, top - bottom);
   lens->set_film_offset((right + left) * 0.5, (top + bottom) * 0.5);
   lens->set_near_far(-1000, 1000);
   PT(Camera) backcamNode = new Camera("backcam", lens);

   // Note: make a new NodePath to root the background scene (in the spirit of
   //       render, render2d and the like) and set some standard properties for
   //       the 2-d display.
   m_background = NodePath("background");
   m_background.set_depth_write(false);
   m_background.set_depth_test(false);
   m_background.set_material_off(true);
   m_background.set_two_sided(true);
   m_backcam = m_background.attach_new_node(backcamNode);

   // Note: Create a display region that matches the size of the 3-d display
   //       region. Use a negative sort value to make it render before the main
   //       camera, associate the camera with the display region and let this
   //       display region do the clearing since it is the one at the back. To
   //       ensure proper working, disable any clear from display region in
   //       front of this one.
   float l, r, b, t;
   m_windowFramework->get_display_region_3d()->get_dimensions(l, r, b, t);
   PT(DisplayRegion) dr = m_windowFramework->
         get_graphics_window()->make_mono_display_region(l, r, b, t);
   dr->set_sort(-10);
   dr->set_camera(m_backcam);
   dr->set_clear_depth_active(true);
   dr->set_clear_color_active(true);
   dr->set_clear_color(Colorf(0,0,0,1));

   // Obtain two texture cards. One renders before the dragon, the other after.
   m_bcard = m_windowFramework->get_graphics_window()->get_texture_card();
   m_bcard.reparent_to(m_background);
   m_bcard.set_transparency(TransparencyAttrib::M_alpha);
   m_fcard = m_windowFramework->get_graphics_window()->get_texture_card();
   const NodePath& render2d = m_windowFramework->get_render_2d();
   m_fcard.reparent_to(render2d);
   m_fcard.set_transparency(TransparencyAttrib::M_alpha);

   // Initialize one of the nice effects.
   choose_effect_ghost();

   // Add the task that initiates the screenshots.
   MT_ADD_TASK("take_snap_shot", take_snap_shot);

   // Create some black squares on top of which we will
   // place the instructions.
   CardMaker blackmaker("blackmaker");
   blackmaker.set_color(0, 0, 0, 1);
   blackmaker.set_frame(-1.00, -0.50, 0.65, 1.00);
   NodePath instcard(blackmaker.generate());
   instcard.reparent_to(render2d);
   blackmaker.set_frame(-0.5, 0.5, -1.00, -0.85);
   NodePath titlecard(blackmaker.generate());
   titlecard.reparent_to(render2d);

   // Panda does its best to hide the differences between DirectX and
   // OpenGL.  But there are a few differences that it cannot hide.
   // One such difference is that when OpenGL copies from a
   // visible window to a texture, it gets it right-side-up.  When
   // DirectX does it, it gets it upside-down.  There is nothing panda
   // can do to compensate except to expose a flag and let the
   // application programmer deal with it.  You should only do this
   // in the rare event that you're copying from a visible window
   // to a texture.

   if(m_windowFramework->get_graphics_window()->get_gsg()->
         get_copy_texture_inverted())
      {
      nout << "Copy texture is inverted." << endl;
      m_bcard.set_scale(1, 1, -1);
      m_fcard.set_scale(1, 1, -1);
      }

   // Put up the instructions
   COnscreenText title("title");
   title.set_text("Panda3D: Tutorial - Motion Trails");
   title.set_fg(Colorf(1, 1, 1, 1));
   title.set_pos(0, -0.95);
   title.set_scale(0.07);
   const NodePath& aspect2d = m_windowFramework->get_aspect_2d();
   title.reparent_to(aspect2d);
   m_title = title.generate();

   m_instr0 = add_instructions(0.95, "Press ESC to exit");
   m_instr1 = add_instructions(0.90, "Press 1: Ghost effect");
   m_instr2 = add_instructions(0.85, "Press 2: PaintBrush effect");
   m_instr3 = add_instructions(0.80, "Press 3: Double Vision effect");
   m_instr4 = add_instructions(0.75, "Press 4: Wings of Blue effect");
   m_instr5 = add_instructions(0.70, "Press 5: Whirlpool effect");

   // enable the key events
   MT_DEFINE_KEY("escape", "sysExit", quit);
   MT_DEFINE_KEY("1", "chooseEffectGhost", choose_effect_ghost);
   MT_DEFINE_KEY("2", "chooseEffectPaintBrush", choose_effect_paint_brush);
   MT_DEFINE_KEY("3", "chooseEffectDoubleVision", choose_effect_double_vision);
   MT_DEFINE_KEY("4", "chooseEffectWingsOfBlue", choose_effect_wings_of_blue);
   MT_DEFINE_KEY("5", "chooseEffectWhirlpool", choose_effect_whirlpool);
   }

AsyncTask::DoneStatus MotionTrails::take_snap_shot(GenericAsyncTask *task)
   {
   if(task->get_elapsed_time() > m_nextclick)
      {
      m_nextclick += 1.0 / m_clickrate;
      if(m_nextclick < task->get_elapsed_time())
         {
         m_nextclick = task->get_elapsed_time();
         }
      m_windowFramework->get_graphics_window()->trigger_copy();
      }
   return AsyncTask::DS_cont;
   }

void MotionTrails::choose_effect_ghost(const Event* event)
   {
   m_bcard.hide();
   m_fcard.show();
   m_fcard.set_color(1.0, 1.0, 1.0, 0.99);
   m_fcard.set_scale(1.00);
   m_fcard.set_pos(0, 0, 0);
   m_fcard.set_r(0);
   m_clickrate = 30;
   m_nextclick = 0;
   }

void MotionTrails::choose_effect_paint_brush(const Event* event)
   {
   m_bcard.show();
   m_fcard.hide();
   m_bcard.set_color(1, 1, 1, 1);
   m_bcard.set_scale(1.0);
   m_bcard.set_pos(0, 0, 0);
   m_bcard.set_r(0);
   m_clickrate = 10000;
   m_nextclick = 0;
   }

void MotionTrails::choose_effect_double_vision(const Event* event)
   {
   m_bcard.show();
   m_bcard.set_color(1, 1, 1, 1);
   m_bcard.set_scale(1.0);
   m_bcard.set_pos(-0.05, 0,0);
   m_bcard.set_r(0);
   m_fcard.show();
   m_fcard.set_color(1, 1, 1, 0.60);
   m_fcard.set_scale(1.0);
   m_fcard.set_pos(0.05, 0,0);
   m_fcard.set_r(0);
   m_clickrate = 10000;
   m_nextclick = 0;
   }

void MotionTrails::choose_effect_wings_of_blue(const Event* event)
   {
   m_fcard.hide();
   m_bcard.show();
   m_bcard.set_color(1.0, 0.90, 1.0, 254.0/255.0);
   m_bcard.set_scale(1.1, 1, 0.95);
   m_bcard.set_pos(0, 0, 0.05);
   m_bcard.set_r(0);
   m_clickrate = 30;
   m_nextclick = 0;
   }

void MotionTrails::choose_effect_whirlpool(const Event* event)
   {
   m_bcard.show();
   m_fcard.hide();
   m_bcard.set_color(1, 1, 1, 1);
   m_bcard.set_scale(0.999);
   m_bcard.set_pos(0, 0, 0);
   m_bcard.set_r(1);
   m_clickrate = 10000;
   m_nextclick = 0;
   }

void MotionTrails::quit(const Event* event)
   {
   m_windowFramework->get_panda_framework()->set_exit_flag();
   }
