/*
 * util.h
 *
 *  Created on: 2012-07-10
 *      Author: dri
 */

#ifndef UTIL_H_
#define UTIL_H_

// A handy little function for getting the proper position for a given square
inline
LPoint3f square_pos(int i)
   {
   return LPoint3f((i%8) - 3.5, i/8 - 3.5, 0);
   }

#endif /* UTIL_H_ */
