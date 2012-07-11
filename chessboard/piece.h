/*
 * piece.h
 *
 *  Created on: 2012-07-09
 *      Author: dri
 *
 */

#ifndef PIECE_H_
#define PIECE_H_

#include "pandaFramework.h"
#include "util.h"

// Class for a piece. This just handles loading the model and setting initial
// position and color

class Piece : public NodePath
   {
   public:

   enum PieceType
      {
      PT_pawn,
      PT_knight,
      PT_bishop,
      PT_rook,
      PT_queen,
      PT_king
      };

   Piece(int square,
         const Colorf& color,
         WindowFramework* windowFramework,
         const string& model);
   virtual ~Piece() = 0; // base class, not to instantiate

   static Piece* make_piece(PieceType piece,
                            int square,
                            const Colorf& color,
                            WindowFramework* windowFramework);
   };

inline
Piece::Piece(int square,
             const Colorf& color,
             WindowFramework* windowFramework,
             const string& model)
   {
   NodePath render = windowFramework->get_render();
   NodePath::operator=(
      windowFramework->load_model(windowFramework->get_render(), model));
   set_color(color);
   set_pos(square_pos(square));
   }

inline
Piece::~Piece()
   {
   // Empty
   }

// Classes for each type of chess piece
// Obviously, we could have done this by just passing a string to Piece's init.
// But if you wanted to make rules for how the pieces move, a good place to start
// would be to make an isValidMove(toSquare) method for each piece type
// and then check if the destination square is acceptable during ReleasePiece

class Pawn : public Piece
   {
   public:

   Pawn(int square,
        const Colorf& color,
        WindowFramework* windowFramework);
   virtual ~Pawn();
   };

inline
Pawn::Pawn(int square,
           const Colorf& color,
           WindowFramework* windowFramework)
   : Piece(square, color, windowFramework, "../models/pawn")
   {
   // Empty
   }

inline
Pawn::~Pawn()
   {
   // Empty
   }

class Knight : public Piece
   {
   public:

   Knight(int square,
          const Colorf& color,
          WindowFramework* windowFramework);
   virtual ~Knight();
   };

inline
Knight::Knight(int square,
               const Colorf& color,
               WindowFramework* windowFramework)
   : Piece(square, color, windowFramework, "../models/knight")
   {
   // Empty
   }

inline
Knight::~Knight()
   {
   // Empty
   }

class Bishop : public Piece
   {
   public:

   Bishop(int square,
          const Colorf& color,
          WindowFramework* windowFramework);
   virtual ~Bishop();
   };

inline
Bishop::Bishop(int square,
               const Colorf& color,
               WindowFramework* windowFramework)
   : Piece(square, color, windowFramework, "../models/bishop")
   {
   // Empty
   }

inline
Bishop::~Bishop()
   {
   // Empty
   }

class Rook : public Piece
   {
   public:

   Rook(int square,
        const Colorf& color,
        WindowFramework* windowFramework);
   virtual ~Rook();
   };

inline
Rook::Rook(int square,
           const Colorf& color,
           WindowFramework* windowFramework)
   : Piece(square, color, windowFramework, "../models/rook")
   {
   // Empty
   }

inline
Rook::~Rook()
   {
   // Empty
   }

class Queen : public Piece
   {
   public:

   Queen(int square,
         const Colorf& color,
         WindowFramework* windowFramework);
   virtual ~Queen();
   };

inline
Queen::Queen(int square,
             const Colorf& color,
             WindowFramework* windowFramework)
   : Piece(square, color, windowFramework, "../models/queen")
   {
   // Empty
   }

inline
Queen::~Queen()
   {
   // Empty
   }

class King : public Piece
   {
   public:

   King(int square,
        const Colorf& color,
        WindowFramework* windowFramework);
   virtual ~King();
   };

inline
King::King(int square,
           const Colorf& color,
           WindowFramework* windowFramework)
   : Piece(square, color, windowFramework, "../models/king")
   {
   // Empty
   }

inline
King::~King()
   {
   // Empty
   }

#endif /* PIECE_H_ */
