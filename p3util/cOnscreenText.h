/*
 * cOnscreenText.h
 *
 *  Created on: 2012-05-28
 *      Author: dri
 *
 * COnscreenText module: contains a C++ implementation of the
 *                       OnscreenText Python class
 *
 */

#ifndef CONSCREENTEXT_H_
#define CONSCREENTEXT_H_

#include "pandaFramework.h"

class COnscreenText : public NodePath
   {
   public:

   // These are the styles of text we might commonly see.  They set the
   // overall appearance of the text according to one of a number of
   // pre-canned styles.  You can further customize the appearance of the
   // text by specifying individual parameters as well.
   enum TextStyle
      {
      TS_plain = 1,
      TS_screen_title,
      TS_screen_prompt,
      TS_name_confirm,
      TS_black_on_white
      };

   COnscreenText(const string& name, TextStyle style = TS_plain);
   COnscreenText(const COnscreenText& other);
   ~COnscreenText();

   COnscreenText& operator=(const COnscreenText& other);

   void cleanup();

   void set_decal(bool decal);
   bool get_decal() const;

   void set_font(TextFont* font);
   TextFont* get_font() const;

   void clear_text();
   void set_text(const string& text);
   void append_text(const string& text);
   string get_text() const;

   void set_x(float x);
   void set_y(float y);
   void set_pos(float x, float y);
   void set_pos(const LVecBase2f& pos);
   const LVecBase2f& get_pos() const;

   void set_roll(float roll);
   float get_roll() const;

   void set_scale(float scale);
   void set_scale(float scaleX, float scaleY);
   void set_scale(const LVecBase2f& scale);
   const LVecBase2f& get_scale() const;

   void set_wordwrap(float wordwarp);
   float get_wordwrap() const;

   void set_fg(const Colorf& fg);
   void set_bg(const Colorf& bg);
   void set_shadow(const Colorf& shadow);
   void set_frame(const Colorf& frame);

   void set_align(TextNode::Alignment align);
   void set_draw_order(int drawOrder);

   NodePath generate() const;

   private:

   void swap(COnscreenText& other);
   void duplicate_parenting(const COnscreenText& other);

   static const float MARGIN;
   static const float SHADOW;

   struct TextNodeProxy;
   PT(TextNodeProxy) m_textNode;
   };

#endif /* CONSCREENTEXT_H_ */
