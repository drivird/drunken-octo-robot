/*
 * world.cpp
 *
 *  Created on: 2012-08-10
 *      Author: dri
 */

#include "../p3util/cOnscreenText.h"
#include "../p3util/callbackUtil.h"
#include "texturePool.h"
#include "billboardEffect.h"
#include "world.h"

// Note: These macros lighten the call to template functions
#define WORLD_DEFINE_KEY(event_name, description, func) \
   define_key_t<World, &World::func>(m_windowFramework, \
                                     event_name,        \
                                     description,       \
                                     this)

#define WORLD_ADD_TASK(name, func) \
   add_task_t<World, &World::func>(name, this)

#define TEXTURE_MOVIE(fps, obj, textures)             \
   texture_movie<fps, NodePath World::*, &World::obj, \
      TexVec World::*, &World::textures>

World::World(WindowFramework* windowFramework)
   : m_windowFramework(windowFramework),
     m_title(),
     m_escapeEventText(),
     m_onekeyEventText(),
     m_twokeyEventText(),
     m_duckPlane(),
     m_duckTexs(),
     m_duckTask(),
     m_fps(36),
     m_expPlane(),
     m_expTask(),
     m_orientPlane(),
     m_orientTex(),
     m_trackball()
   {
   // Standard initialization stuff
   // Standard title that's on screen in every tutorial
   COnscreenText title("title");
   title.set_text("Panda3D: Tutorial - Texture \"Movies\"");
   title.set_fg(Colorf(1, 1, 1, 1));
   title.set_pos(0.7, -0.95);
   title.set_scale(0.07);
   const NodePath& aspect2d = m_windowFramework->get_aspect_2d();
   title.reparent_to(aspect2d);
   m_title = title.generate();

   // Text to show the keyboard keys and their functions on screen
   COnscreenText escapeEventText("escapeEvent");
   escapeEventText.set_text("ESC: Quit");
   escapeEventText.set_fg(Colorf(1, 1, 1, 1));
   escapeEventText.set_pos(-1.3, 0.95);
   escapeEventText.set_align(TextNode::A_left);
   escapeEventText.set_scale(0.05);
   escapeEventText.reparent_to(aspect2d);
   m_escapeEventText = escapeEventText.generate();

   COnscreenText onekeyEventText("onekeyEvent");
   onekeyEventText.set_text("[1]: Freeview camera");
   onekeyEventText.set_fg(Colorf(1, 1, 1, 1));
   onekeyEventText.set_pos(-1.3, 0.90);
   onekeyEventText.set_align(TextNode::A_left);
   onekeyEventText.set_scale(0.05);
   onekeyEventText.reparent_to(aspect2d);
   m_onekeyEventText = onekeyEventText.generate();

   COnscreenText twokeyEventText("twokeyEvent");
   twokeyEventText.set_text(
      "[2]: Preset Camera Angle 2 (Verify billboard effect)");
   twokeyEventText.set_fg(Colorf(1, 1, 1, 1));
   twokeyEventText.set_pos(-1.3, 0.85);
   twokeyEventText.set_align(TextNode::A_left);
   twokeyEventText.set_scale(0.05);
   twokeyEventText.reparent_to(aspect2d);
   m_twokeyEventText = twokeyEventText.generate();

   // Set the background color
   m_windowFramework->set_background_type(WindowFramework::BT_black);

   // Set up the key input
   // Escape quits
   WORLD_DEFINE_KEY("escape", "exit", quit);
   //Free view
   WORLD_DEFINE_KEY("1", "setViewMain", set_view_main);
   // Billboard effect view
   WORLD_DEFINE_KEY("2", "setViewBillboard", set_view_billboard);

   // Initialization specific to this world
   // Load a polygon plane (4 sided square) to put an animated duck sprite on
   const NodePath& models =
      m_windowFramework->get_panda_framework()->get_models();
   m_duckPlane = m_windowFramework->load_model(models, "../models/plane");
   m_duckPlane.set_pos(-2, 8, 0);         // set its position
   const NodePath& render = m_windowFramework->get_render();
   m_duckPlane.reparent_to(render);       // reparent to render

   // Enable tranparency: this attribute needs to be set for Panda to render the
   // transparency in the duck's texture as transparent rather than opaque
   m_duckPlane.set_transparency(TransparencyAttrib::M_alpha);

   // Now we call our special 'loadTextureMovie' function that returns a list
   // containing all of the textures for the duck sprite.
   // Check the function definition later in this file for its parameters
   load_texture_movie(24, "../duck/duck_fly_left", "png", 2, &m_duckTexs);

   // Next we add a task to our task list that will animate the texture on the
   // duck plane according to the time elapsed.
   m_duckTask = WORLD_ADD_TASK("duckTask",
      TEXTURE_MOVIE(36, m_duckPlane, m_duckTexs));
   // The function texture_movie is set to run any texture movie that
   // animates and loops based on time (rather that some other value like
   // position). To do that, it is set up to expect a number of parameters set
   // in the task object. The following lines set those parameters

   /* Note: passed to the task as template parameters
   #Framerate: The texture will be changed 36 times per second
   self.duckTask.fps = 36
   #self.duckPlane is the object whose texture should be changed
   self.duckTask.obj = self.duckPlane
   #self.duckTexs (which we created earlier with self.oadTextureMovie)
   #contains the list of textures to animate from
   self.duckTask.textures = self.duckTexs
   */

   // Now, instead of a duck, we will put an animated explosion onto a polygon
   // This is the same as loading the duck animation, with the expection that
   // we will "billboard" the explosion so that it always faces the camera
   // load the object
   m_expPlane = m_windowFramework->load_model(models, "../models/plane");
   m_expPlane.set_pos(2, 8, 0);          // set its position
   m_expPlane.reparent_to(render);       // reparent to render
   // enable transparency
   m_expPlane.set_transparency(TransparencyAttrib::M_alpha);
   // load the texture movie
   load_texture_movie(51, "../explosion/explosion", "png", 4, &m_expTexs);

   // create the animation task
   m_expTask = WORLD_ADD_TASK("explosionTask",
      TEXTURE_MOVIE(30, m_expPlane, m_expTexs));
   /* Note: passed to the task as template parameters
   m_expTask.fps = 30                                 #set framerate
   m_expTask.obj = self.expPlane                      #set object
   m_expTask.textures = self.expTexs                  #set texture list
   */

   // This create the "billboard" effect that will rotate the object so that it
   // is always rendered as facing the eye (camera)
   m_expPlane.node()->set_effect(BillboardEffect::make_point_eye());

   // The code below generates the plane you see with the numbers and arrows.
   // This is just to give a sense of orientation as the camera is moved around.
   // Load the object
   m_orientPlane = m_windowFramework->load_model(models, "../models/plane");
   // load the texture
   m_orientTex = TexturePool::load_texture(
      "../models/textures/orientation.png");
   m_orientPlane.set_texture(m_orientTex, 1);        // Set the texture
   m_orientPlane.reparent_to(render);                  // Parent to render
   // Set the position, orientation, and scale
   m_orientPlane.set_pos_hpr_scale(0, 8, -1, 0, -90, 0, 10, 10, 10);

   // Note: mouse support must be activated. The basic method is to call
   //       WindowFramework::setup_trackball. In order to get the same viewpoint
   //       as in the original python tutorial, we get the trackball node and
   //       cancel the previous call to TrackBall::set_pos. Finally, we keep the
   //       trackball's NodePath; we'll use it to enable/disable the mouse.
   NodePath camera = m_windowFramework->get_camera_group();
   m_windowFramework->setup_trackball();
   m_trackball = m_windowFramework->get_mouse().find("**/trackball");
   DCAST(Trackball, m_trackball.node())->set_pos(0, 0, 0);
   }

// This function is run every frame by our tasks to animate the textures
template<int fps, typename T1, T1 objT, typename T2, T2 texturesT>
AsyncTask::DoneStatus World::texture_movie(GenericAsyncTask *task)
   {
   NodePath& obj = this->*objT;
   TexVec& textures = this->*texturesT;

   // Here we calculate the current frame number by multiplying the current time
   // (in seconds) by the frames per second variable we set earlier
   int currentFrame = task->get_elapsed_time() * fps;

   // Now we need to set the current texture on task.obj, which is the object
   // we specified earlier when we loaded the duck and explosion.
   // We will set its texture to one of the textures in the list we defined
   // earlier in task.textures.

   // Since we want the movie to loop, we need to reset the image index when it
   // reaches the end. We do this by performing a remainder operation (modulo,
   // "%" in python) on currentFrame with the number of frames, which is the
   // length of our frame list (len(task.textures)).
   // This is a common programming technique to achieve looping as it garuntees
   // a value in range of the list
   obj.set_texture(textures[currentFrame % textures.size()], 1);
   return AsyncTask::DS_cont;    //Continue the task indefinitely
   }

// Our custom load function to load the textures needed for a movie into a
// list. It assumes the the files are named
// "path/name<serial number>.extention"
// It takes the following arguments
// Frames: The number of frames to load
// name: The "path/name" part of the filename path
// suffix: The "extention" part of the path
// padding: The number of digit the serial number contains:
//          e.g. if the serial number is 0001 then padding is 4
void World::load_texture_movie(int frames,
                               const string& name,
                               const string& suffix,
                               int padding,
                               vector<PT(Texture)>* texs)
   {
   // The following line is very complicated but does a lot in one line
   // Here's the explanation from the inside out:
   // first, a string representing the filename is built an example is:
   // "path/name%04d.extention"
   // The % after the string is an operator that works like C's sprintf function
   // It tells python to put the next argument (i) in place of the %04d
   // For more string formatting information look in the python manual
   // That string is then passed to the loader.loadTexture function
   // The loader.loadTexture command gets done in a loop once for each frame,
   // And the result is returned as a list.
   // For more information on "list comprehensions" see the python manual
   for(int frameItr = 0; frameItr < frames; ++frameItr)
      {
      ostringstream filename;
      filename << name << setfill('0') << setw(padding)
         << frameItr << "." << suffix;
      PT(Texture) tex = TexturePool::load_texture(filename.str());
      (*texs).push_back(tex);
      }
   }

// Enables freelook
void World::set_view_main(const Event* event)
   {
   const NodePath& render = m_windowFramework->get_render();
   NodePath camera = m_windowFramework->get_camera_group();
   camera.reparent_to(render);
   // Note: reparent the trackball to the mouse to enable it
   const NodePath& mouse = m_windowFramework->get_mouse();
   m_trackball.reparent_to(mouse);
   }

// Disables freelook and places the camera in a good position to
// demonstrate the billboard effect
void World::set_view_billboard(const Event* event)
   {
   // Note: detach the trackball from the mouse to disable it
   m_trackball.detach_node();
   const NodePath& render = m_windowFramework->get_render();
   NodePath camera = m_windowFramework->get_camera_group();
   camera.reparent_to(render);
   camera.set_pos_hpr(-7, 7, 0, -90, 0, 0);
   }

void World::quit(const Event* event)
   {
   m_windowFramework->get_panda_framework()->set_exit_flag();
   }
