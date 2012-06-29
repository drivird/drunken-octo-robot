/*
 * cBufferViewer.cpp
 *
 *  Created on: 2012-06-04
 *      Author: dri
 */

#include "pandaFramework.h"
#include "cBufferViewer.h"
#include "geomTriangles.h"

CBufferViewer::CBufferViewer(WindowFramework* windowFrameworkPtr)
   : m_windowFrameworkPtr(windowFrameworkPtr),
     m_enabled(false),
     m_sizex(0),
     m_sizey(0),
     m_position(CP_lrcorner),
     m_layout(CL_hline),
     m_includeAll(true),
     m_cullbin("fixed"),
     m_cullsort(10000),
     m_cardindex(0),
     m_cardmaker("cubemaker"),
     m_task(NULL),
     m_dirty(true)
   {
   m_renderParent = m_windowFrameworkPtr->get_render_2d();
   m_cardmaker.set_frame(-1,1,-1,1);

   EventHandler::get_global_event_handler()->add_hook("render-texture-targets-changed",
                                                      call_refresh_readout,
                                                      this);

   if(ConfigVariableBool("show-buffers", false).get_value())
      {
      enable(true);
      }
   }

// Force the readout to be refreshed.  This is usually invoked
// by GraphicsOutput::add_render_texture (via an event handler).
// However, it is also possible to invoke it manually.  Currently,
// the only time I know of that this is necessary is after a
// window resize (and I ought to fix that).
void CBufferViewer::refresh_readout()
   {
   m_dirty = true;

   // Call enabled again, mainly to ensure that the task has been
   // started.
   enable(m_enabled);
   }

// Note: not needed
//    def isValidTextureSet(self, x):
//        """Access: private. Returns true if the parameter is a
//        list of GraphicsOutput and Texture, or the keyword 'all'."""
//        if (isinstance(x, list)):
//            for elt in x:
//                if (self.isValidTextureSet(elt)==0):
//                    return 0
//        else:
//            return (x=="all") or (isinstance(x, Texture)) or (isinstance(x, GraphicsOutput))

// Returns true if the buffer viewer is currently enabled.
bool CBufferViewer::is_enabled()
   {
   return m_enabled;
   }

// Turn the buffer viewer on or off.  The initial state of the
// buffer viewer depends on the Config variable 'show-buffers'.
void CBufferViewer::enable(bool x)
   {
   m_enabled = x;
   m_dirty = true;
   if(x && m_task == NULL)
      {
      m_task = new GenericAsyncTask("buffer-viewer-maintain-readout",
                                    call_maintain_readout,
                                    this);
      if(m_task != NULL)
         {
         m_task->set_priority(1);
         AsyncTaskManager::get_global_ptr()->add(m_task);
         }
      }
   }

// Toggle the buffer viewer on or off.  The initial state of the
// enable flag depends on the Config variable 'show-buffers'.
void CBufferViewer::toggle_enable()
   {
   enable(!m_enabled);
   }

// Set the size of each card.  The units are relative to
// render2d (ie, 1x1 card is not square).  If one of the
// dimensions is zero, then the viewer will choose a value
// for that dimension so as to ensure that the aspect ratio
// of the card matches the aspect ratio of the source-window.
// If both dimensions are zero, the viewer uses a heuristic
// to choose a reasonable size for the card.  The initial
// value is (0, 0).
void CBufferViewer::set_card_size(float x, float y)
   {
   if(x < 0 || y < 0)
      {
      nout << "ERROR: invalid parameter to BufferViewer.setCardSize" << endl;
      return;
      }
   m_sizex = x;
   m_sizey = y;
   m_dirty = true;
   }

// Set the position of the cards.  The valid values are:
// * llcorner - put them in the lower-left  corner of the window
// * lrcorner - put them in the lower-right corner of the window
// * ulcorner - put them in the upper-left  corner of the window
// * urcorner - put them in the upper-right corner of the window
// * window   - put them in a separate window
// The initial value is 'lrcorner'.
void CBufferViewer::set_position(CardPosition pos)
   {
   if(pos == CP_window)
      {
      nout << "BufferViewer.setPosition - \"window\" mode not implemented yet." << endl;
      return;
      }
   m_position = pos;
   m_dirty = true;
   }

// Set the layout of the cards.  The valid values are:
// * vline - display them in a vertical line
// * hline - display them in a horizontal line
// * vgrid - display them in a vertical grid
// * hgrid - display them in a horizontal grid
// * cycle - display one card at a time, using selectCard/advanceCard
// The default value is 'hline'.
void CBufferViewer::set_layout(CardLayout lay)
   {
   m_layout = lay;
   m_dirty = true;
   }

// Only useful when using setLayout('cycle').  Sets the index
// that selects which card to display.  The index is taken modulo
// the actual number of cards.
void CBufferViewer::select_card(int i)
   {
   m_cardindex = i;
   m_dirty = true;
   }

// Only useful when using setLayout('cycle').  Increments the index
// that selects which card to display.  The index is taken modulo
// the actual number of cards.
void CBufferViewer::advance_card()
   {
   ++m_cardindex;
   m_dirty = true;
   }

// Set the include-set for the buffer viewer.  The include-set
// specifies which of the render-to-texture targets to display.
// Valid inputs are the string 'all' (display every render-to-texture
// target), or a list of GraphicsOutputs or Textures.  The initial
// value is 'all'.
void CBufferViewer::set_include_all()
   {
   m_includeAll = true;
   m_include.clear();
   }

void CBufferViewer::set_include(const vector<Texture*>& x)
   {
   m_includeAll = false;
   m_include = x;
   m_dirty = true;
   }

void CBufferViewer::set_include(const vector<GraphicsOutput*>& x)
   {
   m_includeAll = false;
   m_include.clear();
   for(vector<GraphicsOutput*>::const_iterator i = x.begin(); i != x.end(); ++i)
      {
      for(int itex = 0; itex < (*i)->count_textures(); ++itex)
         {
         m_include.push_back((*i)->get_texture(itex));
         }
      }
   m_dirty = true;
   }

void CBufferViewer::set_include(const GraphicsEngine* x)
   {
   m_include.clear();
   for(int iwin = 0; iwin < x->get_num_windows(); ++iwin)
      {
      PT(GraphicsOutput) win =  x->get_window(iwin);
      for(int itex = 0; itex < win->count_textures(); ++itex)
         {
         m_include.push_back(win->get_texture(itex));
         }
      }
   m_dirty = true;
   }

// Set the exclude-set for the buffer viewer.  The exclude-set
// should be a list of GraphicsOutputs and Textures to ignore.
// The exclude-set is subtracted from the include-set (so the excludes
// effectively override the includes.)  The initial value is the
// empty list.
void CBufferViewer::set_exclude_none()
   {
   m_exclude.clear();
   m_dirty = true;
   }

void CBufferViewer::set_exclude(const vector<Texture*>& x)
   {
   m_exclude = x;
   m_dirty = true;
   }

void CBufferViewer::set_exclude(const vector<GraphicsOutput*>& x)
   {
   m_exclude.clear();
   for(vector<GraphicsOutput*>::const_iterator i = x.begin(); i != x.end(); ++i)
      {
      for(int itex = 0; itex < (*i)->count_textures(); ++itex)
         {
         m_exclude.push_back((*i)->get_texture(itex));
         }
      }
   m_dirty = true;
   }

// Set the cull-bin and sort-order for the output cards.  The
// default value is 'fixed', 10000.
void CBufferViewer::set_sort(const string& bin, int sort)
   {
   m_cullbin = bin;
   m_cullsort = sort;
   m_dirty = true;
   }

// Set the scene graph root to which the output cards should
// be parented.  The default is render2d.
void CBufferViewer::set_render_parent(NodePath renderParent)
   {
   m_renderParent = renderParent;
   m_dirty = true;
   }

// Note: no need
//def analyzeTextureSet(self, x, set):
//    """Access: private.  Converts a list of GraphicsObject,
//    GraphicsEngine, and Texture into a table of Textures."""
//
//    if (isinstance(x, list)):
//        for elt in x:
//            self.analyzeTextureSet(elt, set)
//    elif (isinstance(x, Texture)):
//        set[x] = 1
//    elif (isinstance(x, GraphicsOutput)):
//        for itex in range(x.countTextures()):
//            tex = x.getTexture(itex)
//            set[tex] = 1
//    elif (isinstance(x, GraphicsEngine)):
//        for iwin in range(x.getNumWindows()):
//            win = x.getWindow(iwin)
//            self.analyzeTextureSet(win, set)
//    elif (x=="all"):
//        self.analyzeTextureSet(base.graphicsEngine, set)
//   else: return

// Access: private.  Each texture card is displayed with
// a two-pixel wide frame (a ring of black and a ring of white).
// This routine builds the frame geometry.  It is necessary to
// be precise so that the frame exactly aligns to pixel
// boundaries, and so that it doesn't overlap the card at all.
NodePath CBufferViewer::make_frame(int sizex, int sizey)
   {
   CPT(GeomVertexFormat) format = GeomVertexFormat::get_v3cp();
   PT(GeomVertexData) vdata = new GeomVertexData("card-frame", format, Geom::UH_dynamic);

   auto_ptr<GeomVertexWriter> vwriter(new GeomVertexWriter(vdata, "vertex"));
   auto_ptr<GeomVertexWriter> cwriter(new GeomVertexWriter(vdata, "color" ));

   vector<int> ringoffset;
   ringoffset.reserve(4);
   ringoffset.push_back(0);
   ringoffset.push_back(1);
   ringoffset.push_back(1);
   ringoffset.push_back(2);

   vector<int> ringbright;
   ringbright.reserve(4);
   ringbright.push_back(0);
   ringbright.push_back(0);
   ringbright.push_back(1);
   ringbright.push_back(1);

   for(int ring = 0; ring < 4; ++ring)
      {
      float offsetx = (ringoffset[ring]*2.0) / (float)sizex;
      float offsety = (ringoffset[ring]*2.0) / (float)sizey;
      int bright = ringbright[ring];
      vwriter->add_data3f(-1-offsetx, 0, -1-offsety);
      vwriter->add_data3f(1+offsetx, 0, -1-offsety);
      vwriter->add_data3f(1+offsetx, 0,  1+offsety);
      vwriter->add_data3f(-1-offsetx, 0,  1+offsety);
      cwriter->add_data3f(bright, bright, bright);
      cwriter->add_data3f(bright, bright, bright);
      cwriter->add_data3f(bright, bright, bright);
      cwriter->add_data3f(bright, bright, bright);
      }

   PT(GeomTriangles) triangles = new GeomTriangles(Geom::UH_static);
   for(int i = 0; i < 2; ++i)
      {
      int delta = i*8;
      triangles->add_vertices(0+delta, 4+delta, 1+delta);
      triangles->add_vertices(1+delta, 4+delta, 5+delta);
      triangles->add_vertices(1+delta, 5+delta, 2+delta);
      triangles->add_vertices(2+delta, 5+delta, 6+delta);
      triangles->add_vertices(2+delta, 6+delta, 3+delta);
      triangles->add_vertices(3+delta, 6+delta, 7+delta);
      triangles->add_vertices(3+delta, 7+delta, 0+delta);
      triangles->add_vertices(0+delta, 7+delta, 4+delta);
      }
   triangles->close_primitive();

   PT(Geom) geom = new Geom(vdata);
   geom->add_primitive(triangles);
   PT(GeomNode) geomnode = new GeomNode("card-frame");
   geomnode->add_geom(geom);
   return NodePath(geomnode);
   }

// Access: private.  Whenever necessary, rebuilds the entire
// display from scratch.  This is only done when the configuration
// parameters have changed.
AsyncTask::DoneStatus CBufferViewer::maintain_readout(GenericAsyncTask* taskPtr)
   {
   // If nothing has changed, don't update.
   if(!m_dirty)
      {
      return AsyncTask::DS_cont;
      }
   m_dirty = false;

   // Delete the old set of cards.
   for(vector<NodePath>::iterator card = m_cards.begin(); card != m_cards.end(); ++card)
      {
      card->remove_node();
      }
   m_cards.clear();

   // If not enabled, return.
   if(!m_enabled)
      {
      AsyncTaskManager::get_global_ptr()->remove(m_task);
      m_task = NULL;
      return AsyncTask::DS_done;
      }

   // Generate the include and exclude sets.
   int excludeCount = 0;
   PT(GraphicsEngine) graphicsEnginePtr = m_windowFrameworkPtr->get_panda_framework()->get_graphics_engine();
   if(m_includeAll)
      {
      set_include(graphicsEnginePtr);
      }

   // Generate a list of cards and the corresponding windows.
   vector<GraphicsOutput*> wins;
   for(int iwin = 0; iwin < graphicsEnginePtr->get_num_windows(); ++iwin)
      {
      PT(GraphicsOutput) win = graphicsEnginePtr->get_window(iwin);
      for(int itex = 0; itex < win->count_textures(); ++itex)
         {
         Texture* tex = win->get_texture(itex);
         bool isIncluded = false;
         for(vector<Texture*>::iterator i = m_include.begin(); i != m_include.end() && !isIncluded; ++i)
            {
            Texture* includedTex = *i;
            if(tex == includedTex)
               {
               isIncluded = true;
               }
            }
         bool isExcluded = false;
         for(vector<Texture*>::iterator i = m_exclude.begin(); i != m_exclude.end() && !isExcluded; ++i)
            {
            Texture* excludedTex = *i;
            if(tex == excludedTex)
               {
               isExcluded = true;
               }
            }
         if(isIncluded && !isExcluded)
            {
            if(tex->get_texture_type() == Texture::TT_cube_map)
               {
               for(int face = 0; face < 6; ++face)
                  {
                  m_cardmaker.set_uv_range_cube(face);
                  NodePath card = NodePath(m_cardmaker.generate());
                  card.set_texture(tex);
                  m_cards.push_back(card);
                  }
               }
            else
               {
               NodePath card = win->get_texture_card();
               card.set_texture(tex);
               m_cards.push_back(card);
               }
            wins.push_back(win);
            m_exclude.push_back(tex);
            ++excludeCount;
            }
         }
      }

   while(excludeCount--)
      {
      m_exclude.pop_back();
      }

   if(m_cards.size() == 0)
      {
      AsyncTaskManager::get_global_ptr()->remove(m_task);
      m_task = NULL;
      return AsyncTask::DS_done;
      }

   int ncards = m_cards.size();

   // Decide how many rows and columns to use for the layout.
   int rows = 0;
   int cols = 0;
   switch(m_layout)
      {
      case CL_hline:
         rows = 1;
         cols = ncards;
         break;
      case CL_vline:
         rows = ncards;
         cols = 1;
         break;
      case CL_hgrid:
         rows = (int)sqrt((float)ncards);
         cols = rows;
         if(rows * cols < ncards) { ++cols; }
         if(rows * cols < ncards) { ++rows; }
         break;
      case CL_vgrid:
         rows = (int)sqrt((float)ncards);
         cols = rows;
         if(rows * cols < ncards) { ++rows; }
         if(rows * cols < ncards) { ++cols; }
         break;
      case CL_cycle:
         rows = 1;
         cols = 1;
         break;
      default:
         nout << "ERROR: missing a CardLayout case." << endl;
      }

   // Choose an aspect ratio for the cards.  All card size
   // calculations are done in pixel-units, using integers,
   // in order to ensure that everything ends up neatly on
   // a pixel boundary.

   int aspectx = wins[0]->get_x_size();
   int aspecty = wins[0]->get_y_size();
   for(vector<GraphicsOutput*>::const_iterator win = wins.begin(); win != wins.end(); ++win)
      {
      if(((*win)->get_x_size()*aspecty) != ((*win)->get_y_size()*aspectx))
         {
         aspectx = 1;
         aspecty = 1;
         }
      }

   // Choose a card size.  If the user didn't specify a size,
   // use a heuristic, otherwise, just follow orders.  The
   // heuristic uses an initial card size of 42.66666667% of
   // the screen vertically, which comes to 256 pixels on
   // an 800x600 display.  Then, it double checks that the
   // readout will fit on the screen, and if not, it shrinks it.

   float bordersize = 4.0;

   int sizey = 0;
   int sizex = 0;
   if(m_sizex == 0 && m_sizey == 0)
      {
      sizey = (int)(0.4266666667 * m_windowFrameworkPtr->get_graphics_output()->get_y_size());
      sizex = (int)(sizey * aspectx / aspecty);
      int v_sizey = (int)((m_windowFrameworkPtr->get_graphics_output()->get_y_size() - (rows-1) - (rows*2)) / rows);
      int v_sizex = (int)(v_sizey * aspectx / aspecty);
      if(v_sizey < sizey || v_sizex < sizex)
         {
         sizey = v_sizey;
         sizex = v_sizex;
         }

       int adjustment = 2;
       float h_sizex = (float)(m_windowFrameworkPtr->get_graphics_output()->get_x_size() - adjustment) / (float)cols;

       h_sizex -= bordersize;
       if(h_sizex < 1.0)
          {
          h_sizex = 1.0;
          }

       float h_sizey = floor(h_sizex * aspecty / aspectx);
       if(h_sizey < sizey || h_sizex < sizex)
          {
          sizey = (int)h_sizey;
          sizex = (int)h_sizex;
          }
      }
   else
      {
      sizex = (m_sizex * m_windowFrameworkPtr->get_graphics_output()->get_x_size())/2;
      sizey = (m_sizey * m_windowFrameworkPtr->get_graphics_output()->get_y_size())/2;
      if(sizex == 0) { sizex = (int)(sizey*aspectx/aspecty); }
      if(sizey == 0) { sizey = (int)(sizex*aspecty/aspectx); }
      }

   // Convert from pixels to render2d-units.
   float fsizex = (2.0 * sizex) / (float)m_windowFrameworkPtr->get_graphics_output()->get_x_size();
   float fsizey = (2.0 * sizey) / (float)m_windowFrameworkPtr->get_graphics_output()->get_y_size();
   float fpixelx = 2.0 / (float)m_windowFrameworkPtr->get_graphics_output()->get_x_size();
   float fpixely = 2.0 / (float)m_windowFrameworkPtr->get_graphics_output()->get_y_size();

   // Choose directional offsets
   float dirx = 0;
   float diry = 0;
   switch(m_position)
      {
      case CP_llcorner:
         dirx = -1.0;
         diry = -1.0;
         break;
      case CP_lrcorner:
         dirx =  1.0;
         diry = -1.0;
         break;
      case CP_ulcorner:
         dirx = -1.0;
         diry =  1.0;
         break;
      case CP_urcorner:
         dirx =  1.0;
         diry =  1.0;
         break;
      default:
         nout << "ERROR: window mode not implemented yet" << endl;
      }

   // Create the frame
   NodePath frame = make_frame(sizex, sizey);

   // Now, position the cards on the screen.
   // For each card, create a frame consisting of eight quads.

   for(int r = 0; r < rows; ++r)
      {
      for(int c = 0; c < cols; ++c)
         {
         int index = c + r*cols;
         if(index < ncards)
            {
            index = (index + m_cardindex) % m_cards.size();

            float posx = dirx * (1.0 - ((c + 0.5) * (fsizex + fpixelx * bordersize))) - (fpixelx * dirx);
            float posy = diry * (1.0 - ((r + 0.5) * (fsizey + fpixely * bordersize))) - (fpixely * diry);
            NodePath placer = NodePath("card-structure");
            placer.set_pos(posx, 0, posy);
            placer.set_scale(fsizex*0.5, 1.0, fsizey*0.5);
            placer.set_bin(m_cullbin, m_cullsort);
            placer.reparent_to(m_renderParent);
            frame.instance_to(placer);
            m_cards[index].reparent_to(placer);
            m_cards[index] = placer;
            }
         }
      }

   return AsyncTask::DS_cont;
   }


AsyncTask::DoneStatus CBufferViewer::call_maintain_readout(GenericAsyncTask* taskPtr, void* dataPtr)
   {
   // precondition
   if(dataPtr == NULL)
      {
      nout << "parameter dataPtr cannot be NULL" << endl;
      return AsyncTask::DS_done;
      }

   return static_cast<CBufferViewer*>(dataPtr)->maintain_readout(taskPtr);
   }

void CBufferViewer::call_refresh_readout(const Event* eventPtr, void* dataPtr)
   {
   // precondition
   if(dataPtr == NULL)
      {
      nout << "parameter dataPtr cannot be NULL" << endl;
      return;
      }

   static_cast<CBufferViewer*>(dataPtr)->refresh_readout();
   }
