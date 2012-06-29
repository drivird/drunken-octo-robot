/*
 * cFilterManager.h
 *
 *  Created on: 2012-06-20
 *      Author: dri
 *
 * This class implements in C++ Panda3D python FilterManager. I have tried to
 * remain as close as possible to the python code. This is why structs are used
 * instead of classes. Also, light-weight structs are used to simulate python
 * positional parameters but the design could be further improved using the name
 * parameter idiom. -dri
 *
 * Original python header:
 *
 * The FilterManager is a convenience class that helps with the creation
 * of render-to-texture buffers for image postprocessing applications.
 *
 * Still need to implement:
 *
 * - Make sure sort-order of buffers is correct.
 * - Matching buffer size to original region instead of original window.
 * - Intermediate layer creation.
 * - Handling of window clears.
 * - Resizing of windows.
 * - Do something about window-size roundoff problems.
 *
 */

#ifndef CFILTERMANAGER_H_
#define CFILTERMANAGER_H_

#include "pandaFramework.h"

struct CFilterManager
   {
   typedef map<string, PT(Texture)> TexMap;
   typedef vector<PT(GraphicsOutput)> BufferVector;
   typedef vector<LVecBase3f> SizeVector;

   CFilterManager(GraphicsOutput* win,
                  NodePath cam,
                  int forcex = 0,
                  int forcey = 0);

   struct ClearPair
      {
      ClearPair();
      ClearPair(bool active, const Colorf& value);

      bool m_active;
      Colorf m_value;
      };
   typedef vector<ClearPair> ClearVector;
   void get_clears(const DrawableRegion& region, ClearVector* clears);
   void set_clears(DrawableRegion* region, const ClearVector& clears);
   void set_stacked_clears(DrawableRegion* region,
                           const ClearVector& clears0,
                           const ClearVector& clears1);

   bool is_fullscreen();

   struct RenderSceneIntoParameters
      {
      RenderSceneIntoParameters();
      PT(Texture) depthtex;
      PT(Texture) colortex;
      PT(Texture) auxtex;
      int auxbits;
      TexMap* textures;
      };
   NodePath render_scene_into(const RenderSceneIntoParameters& params);

   struct RenderQuadIntoParameters
      {
      RenderQuadIntoParameters();
      int mul;
      int div;
      int align;
      PT(Texture) depthtex;
      PT(Texture) colortex;
      PT(Texture) auxtex0;
      PT(Texture) auxtex1;
      };
   NodePath render_quad_into(const RenderQuadIntoParameters& params);

   void resize_buffers();

   void cleanup();

   PT(GraphicsOutput) m_win;
   NodePath m_camera;
   int m_forcex;
   int m_forcey;
   PT(GraphicsEngine) m_engine;
   PT(DisplayRegion) m_region;
   ClearVector m_wclears;
   ClearVector m_rclears;
   CPT(RenderState) m_caminit;
   CPT(RenderState) m_camstate;
   BufferVector m_buffers;
   SizeVector m_sizes;
   int m_nextsort;
   // bool m_basex; // Note: not used
   // bool m_basey; // Note: not used

   private:

   struct LVecBase2n
      {
      LVecBase2n();
      LVecBase2n(int x, int y);
      int x, y;
      };
   LVecBase2n get_scaled_size(int mul, int div, int align);
   struct TexGroup
      {
      TexGroup();
      TexGroup(Texture* depth, Texture* color, Texture* aux0, Texture* aux1);
      PT(Texture) depthtex;
      PT(Texture) colortex;
      PT(Texture) auxtex0;
      PT(Texture) auxtex1;
      };
   GraphicsOutput* create_buffer(const string &name,
                                 int xsize,
                                 int ysize,
                                 const TexGroup& texgroup,
                                 int depthbits = 1);
   static void call_resize_buffers(const Event* eventPtr, void* dataPtr);
   };


#endif /* CFILTERMANAGER_H_ */
