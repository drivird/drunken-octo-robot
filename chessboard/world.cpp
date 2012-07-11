/*
 * world.cpp
 *
 *  Created on: 2012-07-08
 *      Author: dri
 */

#include "../p3util/cOnscreenText.h"
#include "ambientLight.h"
#include "directionalLight.h"
#include "mouseWatcher.h"
#include "world.h"

// First we define some constants for the colors
const Colorf World::BLACK = Colorf(0, 0, 0, 1);
const Colorf World::WHITE = Colorf(1, 1, 1, 1);
const Colorf World::HIGHLIGHT = Colorf(0, 1, 1, 1);
const Colorf World::PIECEBLACK = Colorf(0.15, 0.15, 0.15, 1);

// Now we define some helper functions that we will need later

// This function, given a line (vector plus origin point) and a desired z value,
// will give us the point on the line where the desired z value is what we want.
// This is how we know where to position an object in 3D space based on a 2D mouse
// position. It also assumes that we are dragging in the XY plane.
//
// This is derived from the mathematical of a plane, solved for a given point
LPoint3f World::point_at_z(float z, LPoint3f point, LVector3f vec)
   {
   return point + vec * ((z-point.get_z()) / vec.get_z());
   }

// Helper function for determining whether a square should be white or black
// The modulo operations (%) generate the every-other pattern of a chess-board
const Colorf& World::square_color(int i)
   {
   if((i + ((i/8)%2))%2 == 1) { return BLACK; }
   else                       { return WHITE; }
   }

World::World(WindowFramework* windowFramework)
   : m_windowFramework(windowFramework),
     m_title(),
     m_escapeEvent(),
     m_mouse1Event(),
     m_picker(),
     m_pq(NULL),
     m_pickerNode(NULL),
     m_pickerNP(),
     m_pickerRay(NULL),
     m_squareRoot(),
     m_squares(64),
     m_pieces(64, NULL),
     m_hiSq(-1),
     m_dragging(-1),
     m_mouseTask(NULL)
   {
   // This code puts the standard title and instruction text on screen
   COnscreenText title("title", COnscreenText::TS_plain);
   title.set_text("Panda3D: Tutorial - Mouse Picking");
   title.set_fg(Colorf(1,1,1,1));
   title.set_pos(0.8, -0.95);
   title.set_scale(0.07);
   m_title = title.generate();
   NodePath aspect2d = m_windowFramework->get_aspect_2d();
   m_title.reparent_to(aspect2d);
   COnscreenText escapeEvent("escapeEvent", COnscreenText::TS_plain);
   escapeEvent.set_text("ESC: Quit");
   escapeEvent.set_fg(Colorf(1,1,1,1));
   escapeEvent.set_pos(-1.3, 0.95);
   escapeEvent.set_align(TextNode::A_left);
   escapeEvent.set_scale(0.05);
   m_escapeEvent = escapeEvent.generate();
   m_escapeEvent.reparent_to(aspect2d);
   COnscreenText mouse1Event("mouse1Event", COnscreenText::TS_plain);
   mouse1Event.set_text("Left-click and drag: Pick up and drag piece");
   mouse1Event.set_fg(Colorf(1, 1, 1, 1));
   mouse1Event.set_pos(-1.3, 0.90);
   mouse1Event.set_align(TextNode::A_left);
   mouse1Event.set_scale(0.05);
   m_mouse1Event = mouse1Event.generate();
   m_mouse1Event.reparent_to(aspect2d);

   // Escape quits
   m_windowFramework->enable_keyboard();
   m_windowFramework->get_panda_framework()->define_key("escape",
                                                        "escapeQuits",
                                                        call_exit,
                                                        NULL);
   // Disable mouse camera control
   // Note: no need to `base.disableMouse()' in C++
   // Set the camera
   NodePath camera = m_windowFramework->get_camera_group();
   camera.set_pos_hpr(0, -13.75, 6, 0, -25, 0);
   // Setup default lighting
   setup_lights();

   // Since we are using collision detection to do picking, we set it up like
   // any other collision detection system with a traverser and a handler
   // self.picker = CollisionTraverser()         // Make a traverser
   m_pq = new CollisionHandlerQueue();           // Make a handler
   // Make a collision node for our picker ray
   m_pickerNode = new CollisionNode("mouseRay");
   // Attach that node to the camera since the ray will need to be positioned
   // relative to it
   m_pickerNP = camera.attach_new_node(m_pickerNode);
   // Everything to be picked will use bit 1. This way if we were doing other
   // collision we could seperate it
   m_pickerNode->set_from_collide_mask(BitMask32::bit(1));
   m_pickerRay = new CollisionRay();          // Make our ray
   m_pickerNode->add_solid(m_pickerRay);      // Add it to the collision node
   // Register the ray as something that can cause collisions
   m_picker.add_collider(m_pickerNP, m_pq);
   NodePath render = m_windowFramework->get_render();
   //m_picker.show_collisions(render);

   // Now we create the chess board and its pieces

   // We will attach all of the squares to their own root. This way we can do the
   // collision pass just on the squares and save the time of checking the rest
   // of the scene
   m_squareRoot = render.attach_new_node("squareRoot");

   // For each square
   NodePath models = m_windowFramework->get_panda_framework()->get_models();
   for(int i = 0; i < 64; ++i)
      {
      // Load, parent, color, and position the model (a single square polygon)
      m_squares[i] = m_windowFramework->load_model(models, "../models/square");
      m_squares[i].reparent_to(m_squareRoot);
      m_squares[i].set_pos(square_pos(i));
      m_squares[i].set_color(square_color(i));
      // Set the model itself to be collideable with the ray. If this model was
      // any more complex than a single polygon, you should set up a collision
      // sphere around it instead. But for single polygons this works fine.
      m_squares[i].find("**/polygon").node()->set_into_collide_mask(
         BitMask32::bit(1));
      // Set a tag on the square's node so we can look up what square this is
      // later during the collision pass
      ostringstream tag;
      tag << i;
      m_squares[i].find("**/polygon").node()->set_tag("square", tag.str());

      // We will use this variable as a pointer to whatever piece is currently
      // in this square
      }

   // The order of pieces on a chessboard from white's perspective. This list
   // contains the constructor functions for the piece classes defined below
   Piece::PieceType pieceOrder[] = {Piece::PT_rook,
                                    Piece::PT_knight,
                                    Piece::PT_bishop,
                                    Piece::PT_queen,
                                    Piece::PT_king,
                                    Piece::PT_bishop,
                                    Piece::PT_knight,
                                    Piece::PT_rook};

   for(int i = 8; i < 16; ++i)
      {
      // Load the white pawns
      m_pieces[i] = new Pawn(i, WHITE, m_windowFramework);
      }
   for(int i = 48; i < 56; ++i)
      {
      // Load the white pawns
      m_pieces[i] = new Pawn(i, PIECEBLACK, m_windowFramework);
      }
   for(int i = 0; i < 8; ++i)
      {
      // Load the special pieces for the front row and color them white
      m_pieces[i] = Piece::make_piece(pieceOrder[i],
                                      i,
                                      WHITE,
                                      m_windowFramework);
      // Load the special pieces for the back row and color them black
      m_pieces[i+56] = Piece::make_piece(pieceOrder[i],
                                         i+56,
                                         PIECEBLACK,
                                         m_windowFramework);
      }

   // This will represent the index of the currently highlited square
   m_hiSq = -1;
   // This wil represent the index of the square where currently dragged piece
   // was grabbed from
   m_dragging = -1;

   // Start the task that handles the picking
   m_mouseTask = new GenericAsyncTask("mouseTask", call_mouse_task, this);
   AsyncTaskManager::get_global_ptr()->add(m_mouseTask);
   // left-click grabs a piece
   PandaFramework* pf = m_windowFramework->get_panda_framework();
   pf->define_key("mouse1", "mouse1Down", call_grab_piece, this);
   // releasing places it
   pf->define_key("mouse1-up", "mouse1Up", call_release_piece, this);
   }

World::~World()
   {
   for(int i = 0; i < 64; ++i)
      {
      delete m_pieces[i];
      }
   }

// This function swaps the positions of two pieces
void World::swap_pieces(int fr, int to)
   {
   Piece* temp = m_pieces[fr];
   m_pieces[fr] = m_pieces[to];
   m_pieces[to] = temp;
   if(m_pieces[fr])
      {
      // self.pieces[fr].square = fr // Note: unused
      m_pieces[fr]->set_pos(square_pos(fr));
      }
   if(m_pieces[to])
      {
      // self.pieces[to].square = to // Note: unused
      m_pieces[to]->set_pos(square_pos(to));
      }
   }

AsyncTask::DoneStatus World::call_mouse_task(GenericAsyncTask *task,
                                             void *data)
   {
   // precondition
   if(data == NULL)
      {
      nout << "ERROR: parameter data cannot be NULL." << endl;
      return AsyncTask::DS_done;
      }

   static_cast<World*>(data)->mouse_task();

   return AsyncTask::DS_cont;
   }

void World::mouse_task()
   {
   // This task deals with the highlighting and dragging based on the mouse

   // First, clear the current highlight
   if(m_hiSq >= 0)
      {
      m_squares[m_hiSq].set_color(square_color(m_hiSq));
      m_hiSq = -1;
      }

   // Check to see if we can access the mouse. We need it to do anything else
   PT(MouseWatcher) mouseWatcher =
      DCAST(MouseWatcher, m_windowFramework->get_mouse().node());
   if(mouseWatcher->has_mouse())
      {
      // get the mouse position
      const LPoint2f& mpos = mouseWatcher->get_mouse();

      // Set the position of the ray based on the mouse position
      m_pickerRay->set_from_lens(m_windowFramework->get_camera(0), mpos);

      // If we are dragging something, set the position of the object
      // to be at the appropriate point over the plane of the board
      if(m_dragging >= 0)
         {
         // Gets the point described by pickerRay.getOrigin(), which is relative
         // to camera, relative instead to render
         NodePath render = m_windowFramework->get_render();
         NodePath camera = NodePath(m_windowFramework->get_camera(0));
         LPoint3f nearPoint =
            render.get_relative_point(camera, m_pickerRay->get_origin());
         // Same thing with the direction of the ray
         LVector3f nearVec =
            render.get_relative_vector(camera, m_pickerRay->get_direction());
         m_pieces[m_dragging]->set_pos(point_at_z(0.5, nearPoint, nearVec));
         }

      // Do the actual collision pass (Do it only on the squares for
      // efficiency purposes)
      m_picker.traverse(m_squareRoot);
      if(m_pq->get_num_entries() > 0)
         {
         // if we have hit something, sort the hits so that the closest
         // is first, and highlight that node
         m_pq->sort_entries();
         int i = atoi(
            m_pq->get_entry(0)->get_into_node()->get_tag("square").c_str());
         // Set the highlight on the picked square
         m_squares[i].set_color(HIGHLIGHT);
         m_hiSq = i;
         }
      }
   }

void World::call_grab_piece(const Event* event, void* data)
   {
   // precondition
   if(data == NULL)
      {
      nout << "ERROR: parameter data cannot be NULL." << endl;
      return;
      }

   static_cast<World*>(data)->grab_piece();
   }

void World::grab_piece()
   {
   // If a square is highlighted and it has a piece, set it to dragging mode
   if(m_hiSq >= 0 && m_pieces[m_hiSq] != NULL)
      {
      m_dragging = m_hiSq;
      m_hiSq = -1;
      }
   }

void World::call_release_piece(const Event* event, void* data)
   {
   // precondition
   if(data == NULL)
      {
      nout << "ERROR: parameter data cannot be NULL." << endl;
      return;
      }

   static_cast<World*>(data)->release_piece();
   }

void World::release_piece()
   {
   // Letting go of a piece. If we are not on a square, return it to its
   // original position. Otherwise, swap it with the piece in the new square
   if(m_dragging >= 0)   //Make sure we really are dragging something
      {
      // We have let go of the piece, but we are not on a square
      if(m_hiSq < 0)
         {
         m_pieces[m_dragging]->set_pos(square_pos(m_dragging));
         }
      else
         {
         // Otherwise, swap the pieces
         swap_pieces(m_dragging, m_hiSq);
         }
      }

   // We are no longer dragging anything
   m_dragging = -1;
   }

// This function sets up some default lighting
void World::setup_lights()
   {
   PT(AmbientLight) ambientLight = new AmbientLight("ambientLight");
   ambientLight->set_color(Colorf(.8, .8, .8, 1));
   PT(DirectionalLight) directionalLight =
      new DirectionalLight("directionalLight");
   directionalLight->set_direction(LVecBase3f(0, 45, -45));
   directionalLight->set_color(Colorf(0.2, 0.2, 0.2, 1));
   NodePath render = m_windowFramework->get_render();
   render.set_light(render.attach_new_node(directionalLight));
   render.set_light(render.attach_new_node(ambientLight));
   }


void World::call_exit(const Event* event, void* data)
   {
   exit(0);
   }
