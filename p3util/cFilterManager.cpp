/*
 * cFilterManager.cpp
 *
 *  Created on: 2012-06-20
 *      Author: dri
 */

#include "cardMaker.h"
#include "auxBitplaneAttrib.h"
#include "orthographicLens.h"
#include "cFilterManager.h"

CFilterManager::ClearPair::ClearPair()
   : m_active(false),
     m_value(0, 0, 0, 0)
   {
   // Empty
   }

CFilterManager::ClearPair::ClearPair(bool active, const Colorf& value)
   : m_active(active),
     m_value(value)
   {
   // Empty
   }

// The FilterManager constructor requires you to provide
// a window which is rendering a scene, and the camera which is
// used by that window to render the scene.  These are henceforth
// called the 'original window' and the 'original camera.'
CFilterManager::CFilterManager(GraphicsOutput* win,
                               NodePath cam,
                               int forcex,   // = 0
                               int forcey)   // = 0
   : m_win(win),
     m_camera(cam),
     m_forcex(forcex),
     m_forcey(forcey),
     m_engine(NULL),
     m_region(NULL),
     m_wclears(GraphicsOutput::RTP_COUNT),
     m_rclears(GraphicsOutput::RTP_COUNT),
     m_caminit(DCAST(Camera, cam.node())->get_initial_state()),
     m_camstate(m_caminit),
     m_buffers(),
     m_sizes(),
     m_nextsort(0)
     //m_basex(false), // Note: not used
     //m_basey(false)  // Note: not used
   {
   // Precondition
   if(win == NULL)
      {
      nout << "ERROR: parameter win cannot be NULL." << endl;
      return;
      }

   // Note: Notifications are not supported
   // # Create the notify category
   //
   //if (FilterManager.notify == None):
   //    FilterManager.notify = directNotify.newCategory("FilterManager")

   // Find the appropriate display region.

   PT(DisplayRegion) region = NULL;
   for(int i = 0 ; i < win->get_num_active_display_regions(); ++i)
      {
      PT(DisplayRegion) dr = win->get_display_region(i);
      NodePath drcam = dr->get_camera();
      if(drcam == cam) { region = dr; }
      }

   if(region == NULL)
      {
      nout << "ERROR: Could not find appropriate DisplayRegion to filter"
           << endl;
      return;
      }

   // Instance Variables.
   m_engine = win->get_gsg()->get_engine();
   m_region = region;
   get_clears(*m_win, &m_wclears);
   get_clears(*m_region, &m_rclears);
   m_nextsort = m_win->get_sort() - 1000;

   EventHandler::get_global_event_handler()->add_hook("window-event",
                                                      call_resize_buffers,
                                                      this);
   }

void CFilterManager::get_clears(const DrawableRegion& region,
                                ClearVector* clears)
   {
   // Preconditions
   if(clears == NULL)
      {
      nout << "ERROR: parameter clears cannot be NULL." << endl;
      return;
      }

   clears->resize(0);
   for(int i = 0; i < GraphicsOutput::RTP_COUNT; ++i)
      {
      clears->push_back(ClearPair(region.get_clear_active(i),
                                  region.get_clear_value(i)));
      }
   }

void CFilterManager::set_clears(DrawableRegion* region,
                                const ClearVector& clears)
   {
   // Preconditions
   if(region == NULL)
      {
      nout << "ERROR: parameter region cannot be NULL." << endl;
      return;
      }

   for(int i = 0; i < GraphicsOutput::RTP_COUNT; ++i)
      {
      region->set_clear_active(i, clears[i].m_active);
      region->set_clear_value(i, clears[i].m_value);
      }
   }

void CFilterManager::set_stacked_clears(DrawableRegion* region,
                                        const ClearVector& clears0,
                                        const ClearVector& clears1)
   {
   // Preconditions
   if(region == NULL)
      {
      nout << "ERROR: parameter region cannot be NULL." << endl;
      return;
      }

   for(int i = 0; i < GraphicsOutput::RTP_COUNT; ++i)
      {
      bool active = clears0[i].m_active;
      const Colorf* value = &(clears0[i].m_value);
      if(!active)
         {
         active = clears1[i].m_active;
         value = &(clears1[i].m_value);
         }
      region->set_clear_active(i, active);
      region->set_clear_value(i, *value);
      }
   }

bool CFilterManager::is_fullscreen()
   {
   return ((m_region->get_left() == 0.0) &&
           (m_region->get_right() == 1.0) &&
           (m_region->get_bottom() == 0.0) &&
           (m_region->get_top() == 1.0));
   }

// Calculate the size of the desired window. Not public.
CFilterManager::LVecBase2n::LVecBase2n()
   : x(0),
     y(0)
   {
   // Empty
   }

CFilterManager::LVecBase2n::LVecBase2n(int x, int y)
   : x(x),
     y(y)
   {
   // Empty
   }

CFilterManager::LVecBase2n CFilterManager::get_scaled_size(int mul, int div, int align)
   {
   int winx = m_forcex;
   int winy = m_forcey;
   if(winx == 0) { winx = m_win->get_x_size(); }
   if(winy == 0) { winy = m_win->get_y_size(); }

   if(div != 1)
      {
      winx = ((winx+align-1) / align) * align;
      winy = ((winy+align-1) / align) * align;
      winx = (winx / div);
      winy = (winy / div);
      }

   if(mul != 1)
      {
      winx = winx * mul;
      winy = winy * mul;
      }

   return LVecBase2n(winx, winy);
   }


// Causes the scene to be rendered into the supplied textures
// instead of into the original window.  Puts a fullscreen quad
// into the original window to show the render-to-texture results.
// Returns the quad.  Normally, the caller would then apply a
// shader to the quad.
//
// To elaborate on how this all works:
//
// * An offscreen buffer is created.  It is set up to mimic
//   the original display region - it is the same size,
//   uses the same clear colors, and contains a DisplayRegion
//   that uses the original camera.
//
// * A fullscreen quad and an orthographic camera to render
//   that quad are both created.  The original camera is
//   removed from the original window, and in its place, the
//   orthographic quad-camera is installed.
//
// * The fullscreen quad is textured with the data from the
//  offscreen buffer.  A shader is applied that tints the
//  results pink.
//
// * Automatic shader generation NOT enabled.
//   If you have a filter that depends on a render target from
//   the auto-shader, you either need to set an auto-shader
//   attrib on the main camera or scene, or, you need to provide
//   these outputs in your own shader.
//
// * All clears are disabled on the original display region.
//   If the display region fills the whole window, then clears
//   are disabled on the original window as well.  It is
//   assumed that rendering the full-screen quad eliminates
//   the need to do clears.
//
// Hence, the original window which used to contain the actual
// scene, now contains a pink-tinted quad with a texture of the
// scene.  It is assumed that the user will replace the shader
// on the quad with a more interesting filter.
CFilterManager::RenderSceneIntoParameters::RenderSceneIntoParameters()
   : depthtex(NULL),
     colortex(NULL),
     auxtex(NULL),
     auxbits(0),
     textures(NULL)
   {
   // Empty
   }

NodePath CFilterManager::
render_scene_into(const RenderSceneIntoParameters& params)
   {
   PT(Texture) depthtex = params.depthtex;
   PT(Texture) colortex = params.colortex;
   PT(Texture) auxtex   = params.auxtex;
   int auxbits = params.auxbits;
   TexMap* textures = params.textures;

   if(textures != NULL)
      {
      TexMap::iterator tex;
      tex = textures->find("color");
      if(tex != textures->end()) { colortex = (*tex).second; }
      tex = textures->find("depth");
      if(tex != textures->end()) { depthtex = (*tex).second; }
      tex = textures->find("aux");
      if(tex != textures->end()) { auxtex = (*tex).second; }
      }

   if(colortex == NULL)
      {
      colortex = new Texture("filter-base-color");
      colortex->set_wrap_u(Texture::WM_clamp);
      colortex->set_wrap_v(Texture::WM_clamp);
      }

   TexGroup texgroup(depthtex, colortex, auxtex, NULL);

   // Choose the size of the offscreen buffer.

   LVecBase2n win = get_scaled_size(1, 1, 1);
   PT(GraphicsOutput) buffer = create_buffer("filter-base",
                                             win.x,
                                             win.y,
                                             texgroup);

   if(buffer == NULL)
      {
      return NodePath();
      }

   CardMaker cm("filter-base-quad");
   cm.set_frame_fullscreen_quad();
   NodePath quad(cm.generate());
   quad.set_depth_test(false);
   quad.set_depth_write(false);
   quad.set_texture(colortex);
   quad.set_color(Colorf(1, 0.5, 0.5, 1));

   NodePath cs("dummy");
   cs.set_state(m_camstate);
   // Do we really need to turn on the Shader Generator?
   //cs.setShaderAuto()
   if(auxbits != 0)
      {
      cs.set_attrib(AuxBitplaneAttrib::make(auxbits));
      }
   DCAST(Camera, m_camera.node())->set_initial_state(cs.get_state());

   PT(Camera) quadcamnode = new Camera("filter-quad-cam");
   PT(OrthographicLens) lens = new OrthographicLens();
   lens->set_film_size(2, 2);
   lens->set_film_offset(0, 0);
   lens->set_near_far(-1000, 1000);
   quadcamnode->set_lens(lens);
   NodePath quadcam = quad.attach_new_node(quadcamnode);

   m_region->set_camera(quadcam);

   PT(DisplayRegion) dr = buffer->get_display_region(0);
   set_stacked_clears(dr, m_rclears, m_wclears);

   if(auxtex != NULL)
      {
      dr->set_clear_active(GraphicsOutput::RTP_aux_rgba_0, true);
      dr->set_clear_value(GraphicsOutput::RTP_aux_rgba_0,
                          Colorf(0.5, 0.5, 1.0, 0.0));
      }
   m_region->disable_clears();
   if(is_fullscreen())
      {
      m_win->disable_clears();
      }
   dr->set_camera(m_camera);
   dr->set_active(true);

   m_buffers.push_back(buffer);
   m_sizes.push_back(LVecBase3f(1, 1, 1));

   return quad;
   }

// Creates an offscreen buffer for an intermediate
// computation. Installs a quad into the buffer.  Returns
// the fullscreen quad.  The size of the buffer is initially
// equal to the size of the main window.  The parameters 'mul',
// 'div', and 'align' can be used to adjust that size.
CFilterManager::RenderQuadIntoParameters::RenderQuadIntoParameters()
   : mul(1),
     div(1),
     align(1),
     depthtex(NULL),
     colortex(NULL),
     auxtex0(NULL),
     auxtex1(NULL)
   {
   // Empty
   }

NodePath CFilterManager::
render_quad_into(const RenderQuadIntoParameters& params)
   {
   int mul = params.mul;
   int div = params.div;
   int align = params.align;
   PT(Texture) depthtex = params.depthtex;
   PT(Texture) colortex = params.colortex;
   PT(Texture) auxtex0 = params.auxtex0;
   PT(Texture) auxtex1 = params.auxtex1;

   TexGroup texgroup(depthtex, colortex, auxtex0, auxtex1);

   LVecBase2n win = get_scaled_size(mul, div, align);

   bool depthbits = (depthtex != NULL);

   PT(GraphicsOutput) buffer = create_buffer("filter-stage",
                                             win.x,
                                             win.y,
                                             texgroup,
                                             depthbits);

   if(buffer == NULL)
      {
      return NULL;
      }

   CardMaker cm("filter-stage-quad");
   cm.set_frame_fullscreen_quad();
   NodePath quad(cm.generate());
   quad.set_depth_test(false);
   quad.set_depth_write(false);
   quad.set_color(Colorf(1, 0.5, 0.5, 1));

   PT(Camera) quadcamnode = new Camera("filter-quad-cam");
   PT(OrthographicLens) lens = new OrthographicLens();
   lens->set_film_size(2, 2);
   lens->set_film_offset(0, 0);
   lens->set_near_far(-1000, 1000);
   quadcamnode->set_lens(lens);
   NodePath quadcam = quad.attach_new_node(quadcamnode);

   buffer->get_display_region(0)->set_camera(quadcam);
   buffer->get_display_region(0)->set_active(true);

   m_buffers.push_back(buffer);
   m_sizes.push_back(LVecBase3f(mul, div, align));

   return quad;
   }

// Low-level buffer creation.  Not intended for public use.
CFilterManager::TexGroup::TexGroup()
   : depthtex(NULL),
     colortex(NULL),
     auxtex0(NULL),
     auxtex1(NULL)
   {
   // Empty
   }
CFilterManager::TexGroup::TexGroup(Texture* depth,
                                   Texture* color,
                                   Texture* aux0,
                                   Texture* aux1)
   : depthtex(depth),
     colortex(color),
     auxtex0(aux0),
     auxtex1(aux1)
   {
   // Empty
   }
GraphicsOutput* CFilterManager::create_buffer(const string &name,
                                              int xsize,
                                              int ysize,
                                              const TexGroup& texgroup,
                                              int depthbits) // = 1
   {
   WindowProperties winprops;
   winprops.set_size(xsize, ysize);
   FrameBufferProperties props;
   props.set_rgb_color(1);
   props.set_depth_bits(depthbits);

   PT(Texture) depthtex = texgroup.depthtex;
   PT(Texture) colortex = texgroup.colortex;
   PT(Texture) auxtex0 = texgroup.auxtex0;
   PT(Texture) auxtex1 = texgroup.auxtex1;

   if(auxtex0 != NULL)
      {
      props.set_aux_rgba(1);
      }

   if(auxtex1 != NULL)
      {
      props.set_aux_rgba(2);
      }

   PT(GraphicsOutput) buffer = m_engine->make_output(m_win->get_pipe(),
                                                     name,
                                                     -1,
                                                     props,
                                                     winprops,
                                                     GraphicsPipe::BF_refuse_window |
                                                     GraphicsPipe::BF_resizeable,
                                                     m_win->get_gsg(),
                                                     m_win);

   if(buffer == NULL)
      {
      return buffer;
      }

   if(depthtex != NULL)
      {
      buffer->add_render_texture(depthtex,
                                 GraphicsOutput::RTM_bind_or_copy,
                                 GraphicsOutput::RTP_depth);
      }

   if(colortex != NULL)
      {
      buffer->add_render_texture(colortex,
                                 GraphicsOutput::RTM_bind_or_copy,
                                 GraphicsOutput::RTP_color);
      }

   if(auxtex0 != NULL)
      {
      buffer->add_render_texture(auxtex0,
                                 GraphicsOutput::RTM_bind_or_copy,
                                 GraphicsOutput::RTP_aux_rgba_0);
      }

   if(auxtex1 != NULL)
      {
      buffer->add_render_texture(auxtex1,
                                 GraphicsOutput::RTM_bind_or_copy,
                                 GraphicsOutput::RTP_aux_rgba_1);
      }

   buffer->set_sort(m_nextsort);
   buffer->disable_clears();
   buffer->get_display_region(0)->disable_clears();
   ++m_nextsort;
   return buffer;
   }

/*
def windowEvent(self, win):
""" When the window changes size, automatically resize all buffers """
self.resizeBuffers()
*/

// Resize all buffers to match the size of the window.
void CFilterManager::resize_buffers()
   {
   for(unsigned int i = 0; i < m_buffers.size(); ++i)
      {
      const LVecBase3f& size = m_sizes[i];
      LVecBase2n xysize = get_scaled_size(size[0], size[1], size[2]);
      // Note: python is set_size()!?
      m_buffers[i]->set_size_and_recalc(xysize.x, xysize.y);
      }
   }

// Restore everything to its original state, deleting any
// new buffers in the process.
void CFilterManager::cleanup()
   {
   for(BufferVector::iterator buffer = m_buffers.begin(); buffer != m_buffers.end(); ++buffer)
      {
      (*buffer)->clear_render_textures();
      m_engine->remove_window(*buffer);
      }
   m_buffers.clear();
   m_sizes.clear();
   set_clears(m_win, m_wclears);
   set_clears(m_region, m_rclears);
   m_camstate = m_caminit;
   DCAST(Camera, m_camera.node())->set_initial_state(m_caminit);
   m_region->set_camera(m_camera);
   m_nextsort = m_win->get_sort() - 1000;
   //m_basex = 0; // Note: not used
   //m_basey = 0; // Note: not used
   }

void CFilterManager::call_resize_buffers(const Event* eventPtr, void* dataPtr)
   {
   // precondition
   if(dataPtr == NULL)
      {
      nout << "parameter dataPtr cannot be NULL" << endl;
      return;
      }

   static_cast<CFilterManager*>(dataPtr)->resize_buffers();
   }
