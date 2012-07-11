/*
 * piece.cpp
 *
 *  Created on: 2012-07-10
 *      Author: dri
 */

#include "piece.h"

// Note: implemented using the factory design pattern
Piece* Piece::make_piece(PieceType pieceType,
                         int square,
                         const Colorf& color,
                         WindowFramework* windowFramework)
   {
   Piece* piece = NULL;

   switch(pieceType)
      {
      case PT_pawn:
         piece = new Pawn(square, color, windowFramework);
         break;
      case PT_knight:
         piece = new Knight(square, color, windowFramework);
         break;
      case PT_bishop:
         piece = new Bishop(square, color, windowFramework);
         break;
      case PT_rook:
         piece = new Rook(square, color, windowFramework);
         break;
      case PT_queen:
         piece = new Queen(square, color, windowFramework);
         break;
      case PT_king:
         piece = new King(square, color, windowFramework);
         break;
      default:
         nout << "ERROR: missing a PieceType case." << endl;
      }

   return piece;
   }
