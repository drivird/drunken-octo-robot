/*
 * cOnscreenText.cpp
 *
 *  Created on: 2012-05-28
 *      Author: dri
 */

#include "cOnscreenText.h"

const float COnscreenText::MARGIN = 0.1;
const float COnscreenText::SHADOW = 0.4;

   // COnscreenText is implemented using the `Pimpl idiom' technique
// where the `implementation' part is done within struct TextNodeProxy.
struct COnscreenText::TextNodeProxy : public TextNode
   {
   TextNodeProxy(const string& name, COnscreenText::TextStyle style);
   TextNodeProxy(const TextNodeProxy& other);
   virtual ~TextNodeProxy();
   void update_transform_mat();
   LVecBase2f m_scale;
   LVecBase2f m_pos;
   float m_roll;
   float m_wordwrap;
   };

// TextNodeProxy constructor
// Algorithm follows Python version, mostly.
// Called from COnscreenText constructor
COnscreenText::TextNodeProxy::TextNodeProxy(const string& name,
                                            COnscreenText::TextStyle style)
   : TextNode(name),
     m_scale(0, 0),
     m_pos(0, 0),
     m_roll(0),
     m_wordwrap(0)
   {
   // Choose the default parameters according to the selected style.
   Colorf fg(0, 0, 0, 0);
   Colorf bg(0, 0, 0, 0);
   Colorf shadow(0, 0, 0, 0);
   Colorf frame(0, 0 ,0 ,0);
   TextNode::Alignment align(A_center);
   switch(style)
      {
      case TS_plain:
         m_scale = LVecBase2f(0.07, 0.07);
         fg = Colorf(0, 0, 0, 1);
         bg = Colorf(0, 0, 0, 0);
         shadow = Colorf(0, 0, 0, 0);
         frame = Colorf(0, 0, 0 ,0);
         //align = A_center;
         break;

      case TS_screen_title:
         m_scale = LVecBase2f(0.15, 0.15);
         fg = Colorf(1, 0.2, 0.2, 1);
         bg = Colorf(0, 0, 0, 0);
         shadow = Colorf(0, 0, 0, 1);
         frame = Colorf(0, 0, 0 ,0);
         //align = A_center;
         break;

      case TS_screen_prompt:
         m_scale = LVecBase2f(0.1, 0.1);
         fg = Colorf(1, 1, 0, 1);
         bg = Colorf(0, 0, 0, 0);
         shadow = Colorf(0, 0, 0, 1);
         frame = Colorf(0, 0, 0 ,0);
         //align = A_center;
         break;

      case TS_name_confirm:
         m_scale = LVecBase2f(0.1, 0.1);
         fg = Colorf(0, 1, 0, 1);
         bg = Colorf(0, 0, 0, 0);
         shadow = Colorf(0, 0, 0, 0);
         frame = Colorf(0, 0, 0 ,0);
         //align = A_center;
         break;

      case TS_black_on_white:
         m_scale = LVecBase2f(0.1, 0.1);
         fg = Colorf(0, 0, 0, 1);
         bg = Colorf(1, 1, 1, 1);
         shadow = Colorf(0, 0, 0, 0);
         frame = Colorf(0, 0, 0 ,0);
         //align = A_center;
         break;

      default:
         nout << "DESIGN ERROR: missing a TextStyle case." << endl;
         return;
      }

   set_text_color(fg);
   set_align(align);

   if(bg[3] != 0)
      {
      // If we have a background color, create a card.
      set_card_color(bg);
      set_card_as_margin(MARGIN, MARGIN, MARGIN, MARGIN);
      }

   if(shadow[3] != 0)
      {
      // If we have a shadow color, create a shadow.
      // Can't use the *shadow interface because it might be a VBase4.
      // textNode.setShadowColor(*shadow)
      set_shadow_color(shadow);
      set_shadow(SHADOW, SHADOW);
      }

   if(frame[3] != 0)
      {
      // If we have a frame color, create a frame.
      set_frame_color(frame);
      set_frame_as_margin(MARGIN, MARGIN, MARGIN, MARGIN);
      }

   update_transform_mat();
   }

// TextNodeProxy copy constructor
// Having access to TextNode protected copy constructor
// makes implementation a lot simpler.
COnscreenText::TextNodeProxy::TextNodeProxy(const TextNodeProxy& other)
   : TextNode(other),
     m_scale(other.m_scale),
     m_pos(other.m_pos),
     m_roll(other.m_roll),
     m_wordwrap(other.m_wordwrap)
   {
   // empty
   }

// TextNodeProxy destructor
COnscreenText::TextNodeProxy::~TextNodeProxy()
   {
   // Empty
   }

// Create a transform for the text for our scale and position.
// We'd rather do it here, on the text itself, rather than on
// our NodePath, so we have one fewer transforms in the scene
// graph.
void COnscreenText::TextNodeProxy::update_transform_mat()
   {
   LMatrix4f mat =
         LMatrix4f::scale_mat(m_scale.get_x(), 1, m_scale.get_y()) *
         LMatrix4f::rotate_mat(m_roll, LVecBase3f(0, -1, 0)) *
         LMatrix4f::translate_mat(m_pos.get_x(), 0, m_pos.get_y());
   set_transform(mat);
   }

// COnscreenText constructor
// Creates a NodePath that allocates a new TextNode which is initialized with `style'.
// You should use NodePath::reparent_to() to insert the onscreen text into the aspect 2d
// of the scene graph.
// style: one of the pre-canned style parameters defined at the
//        head of this file.  This sets up the default values for
//        many parameters.
COnscreenText::COnscreenText(const string& name,
                             TextStyle style)     // = TS_plain
   : m_textNode(new TextNodeProxy(name, style))
   {
   // Ok, now update the node.
   NodePath::operator=(NodePath(m_textNode));
   }

// COnscreenText copy constructor.
COnscreenText::COnscreenText(const COnscreenText& other)
   : m_textNode(new TextNodeProxy(*other.m_textNode))
   {
   duplicate_parenting(other);
   }

// COnscreenText assignment operator.
COnscreenText& COnscreenText::operator=(const COnscreenText& other)
   {
   COnscreenText(other).swap(*this);
   duplicate_parenting(other);
   return *this;
   }

// Duplicate parenting of the `other' node
void COnscreenText::duplicate_parenting(const COnscreenText& other)
   {
   if(other.get_parent().is_empty())
      {
      NodePath::operator=(NodePath(m_textNode));
      }
   else
      {
      NodePath::operator=(
            other.get_parent().attach_new_node(m_textNode, other.get_sort()));
      }
   }

// Swap `this' data with the `other'
void COnscreenText::swap(COnscreenText& other)
   {
   PT(TextNodeProxy) p = m_textNode;
   m_textNode = other.m_textNode;
   other.m_textNode = p;
   }

// Free memory and the scene graph.
void COnscreenText::cleanup()
   {
   m_textNode = NULL;
   remove_node();
   }

// Cleanup upon destruction.
COnscreenText::~COnscreenText()
   {
   cleanup();
   }

// Reimplementation of TextNode::set_card_decal.
// decal: if this is true, the text is decalled onto its
//        background card.  Useful when the text will be parented
//        into the 3-D scene graph.
void COnscreenText::set_decal(bool decal)
   {
   m_textNode->set_card_decal(decal);
   }

// Reimplementation of TextNode::get_card_decal.
bool COnscreenText::get_decal() const
   {
   return m_textNode->get_card_decal();
   }

// Reimplementation of TextNode::set_font.
// fontPtr: the font to use for the text.
void COnscreenText::set_font(TextFont* fontPtr)
   {
   m_textNode->set_font(fontPtr);
   }

// Reimplementation of TextNode::get_font.
TextFont* COnscreenText::get_font() const
   {
   return m_textNode->get_font();
   }

// Reimplementation of TextNode::clear_text.
void COnscreenText::clear_text()
   {
   m_textNode->clear_text();
   }

// Reimplementation of TextNode::set_text.
// text: the actual text to display.
void COnscreenText::set_text(const string& text)
   {
   m_textNode->set_text(text);
   }

// Reimplementation of TextNode::append_text.
void COnscreenText::append_text(const string& text)
   {
   m_textNode->append_text(text);
   }

// Reimplementation of TextNode::get_text.
string COnscreenText::get_text() const
   {
   return m_textNode->get_text();
   }

// Position the onscreen text on the x-axis.
void COnscreenText::set_x(float x)
   {
   set_pos(LVecBase2f(x, m_textNode->m_pos.get_y()));
   }

// Position the onscreen text on the y-axis.
void COnscreenText::set_y(float y)
   {
   set_pos(LVecBase2f(m_textNode->m_pos.get_x(), y));
   }

// Position the onscreen text in 2d screen space.
void COnscreenText::set_pos(float x, float y)
   {
   set_pos(LVecBase2f(x, y));
   }

// Position the onscreen text in 2d screen space.
// pos: the x, y position of the text on the screen.
void COnscreenText::set_pos(const LVecBase2f& pos)
   {
   m_textNode->m_pos = pos;
   m_textNode->update_transform_mat();
   }

// Returns the position of the onscreen text
// in 2d screen space.
const LVecBase2f& COnscreenText::get_pos() const
   {
   return m_textNode->m_pos;
   }

// Rotate the onscreen text around the screen's normal
void COnscreenText::set_roll(float roll)
   {
   m_textNode->m_roll = roll;
   m_textNode->update_transform_mat();
   }

// Returns the rotation factor of the onscreen text
// around the screen's normal.
float COnscreenText::get_roll() const
   {
   return m_textNode->m_roll;
   }

// Scale the text in 2d space.  You may specify either a single
// uniform scale, or two scales, or a tuple of two scales.
// See overloads, below.
void COnscreenText::set_scale(float scale)
   {
   set_scale(LVecBase2f(scale, scale));
   }

void COnscreenText::set_scale(float scaleX, float scaleY)
   {
   set_scale(LVecBase2f(scaleX, scaleY));
   }

void COnscreenText::set_scale(const LVecBase2f& scale)
   {
   m_textNode->m_scale = scale;
   m_textNode->update_transform_mat();
   }

// Returns the scale of the text in 2d space.
const LVecBase2f& COnscreenText::get_scale() const
   {
   return m_textNode->m_scale;
   }


// Reimplementation of TextNode::set_wordwrap.
// wordwrap: either the width to wordwrap the text at, or 0
//           to specify no automatic word wrapping.
void COnscreenText::set_wordwrap(float wordwrap)
   {
   m_textNode->m_wordwrap = wordwrap;
   if(wordwrap != 0)
      {
      m_textNode->set_wordwrap(wordwrap);
      }
   else
      {
      m_textNode->clear_wordwrap();
      }
   }

// Returns the text wordwrapping distance.
float COnscreenText::get_wordwrap() const
   {
   return m_textNode->m_wordwrap;
   }

// Reimplementation of TextNode::set_text_color.
// fg: the (r, g, b, a) foreground color of the text.
void COnscreenText::set_fg(const Colorf& fg)
   {
   m_textNode->set_text_color(fg);
   }

// Reimplementation of TextNode::set_card_color
// bg: the (r, g, b, a) background color of the text.  If the
//     fourth value, a, is nonzero, a card is created to place
//     behind the text and set to the given color.
void COnscreenText::set_bg(const Colorf& bg)
   {
   if(bg[3] != 0)
      {
      // If we have a background color, create a card.
      m_textNode->set_card_color(bg);
      m_textNode->set_card_as_margin(MARGIN, MARGIN, MARGIN, MARGIN);
      }
   else
      {
      // Otherwise, remove the card.
      m_textNode->clear_card();
      }
   }

// Reimplementation of both TextNode::set_shadow_color and TextNode::set_shadow.
// shadow: the (r, g, b, a) color of the shadow behind the text.
//         If the fourth value, a, is nonzero, a little drop shadow
//         is created and placed behind the text.
void COnscreenText::set_shadow(const Colorf& shadow)
   {
   if(shadow[3] != 0)
      {
      // If we have a shadow color, create a shadow.
      m_textNode->set_shadow_color(shadow);
      m_textNode->set_shadow(SHADOW, SHADOW);
      }
   else
      {
      // Otherwise, remove the shadow.
      m_textNode->clear_shadow();
      }
   }

// Reimplementation of TextNode::set_shadow.
void COnscreenText::set_shadow_offset(const LVecBase2f& offset)
   {
   m_textNode->set_shadow(offset);
   }

// Reimplementation of TextNode::set_frame_color
// frame: the (r, g, b, a) color of the frame drawn around the
//        text.  If the fourth value, a, is nonzero, a frame is
//        created around the text.
void COnscreenText::set_frame(const Colorf& frame)
   {
   if(frame[3] != 0)
      {
      // If we have a frame color, create a frame.
      m_textNode->set_frame_color(frame);
      m_textNode->set_frame_as_margin(MARGIN, MARGIN, MARGIN, MARGIN);
      }
   else
      {
      // Otherwise, remove the frame.
      m_textNode->clear_frame();
      }
   }

// Reimplementation of TextNode::set_align
// align: one of TextNode::A_Left, TextNode::A_right, or TextNode::A_center.
void COnscreenText::set_align(TextNode::Alignment align)
   {
   return m_textNode->set_align(align);
   }

// Reimplementation of TextNode::set_draw_order
// drawOrder: the drawing order of this text with respect to
//            all other things in the 'fixed' bin within render2d.
//            The text will actually use drawOrder through drawOrder +
//            2.
void COnscreenText::set_draw_order(int drawOrder)
   {
   m_textNode->set_bin("fixed");
   m_textNode->set_draw_order(drawOrder);
   }

// Reimplementation of TextNode::generate returning an independent
// NodePath parented just like the COnscreenText. When you are done
// setting the COnscreenText, call this method save the onscreen text
// in a lighter node.
NodePath COnscreenText::generate() const
   {
   if(get_parent().is_empty())
      {
      return NodePath(m_textNode->generate());
      }
   else
      {
      return get_parent().attach_new_node(m_textNode->generate(), get_sort());
      }
   }
