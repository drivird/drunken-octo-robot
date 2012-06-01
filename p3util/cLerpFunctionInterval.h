/*
 * cLerpFunctionInterval.h
 *
 *  Created on: 2012-05-30
 *      Author: dri
 *
 * This class is meant to emulate the capabilities of python LerpInterval class. It is by no
 * mean a full implementation. It only contains what is needed to translate the Panda3D tutorials.
 *
 * CLerpFunctionInterval is templatized so that it can lerp a class (for instance, vector<double>).
 * For a given class A, the following operators must be defined:
 * A A::operator=(const A&)
 * A A::operator-(const A&)
 *    are used to define the lerp range, such as `range = end - start'.
 * A A::operator+(const A&)
 * A A::operator*(double)
 *    each frame, the current A is computed such as `current = start + range * t'
 *    where t starts with 0 at the beginning of the interval and ends with 1.
 * A A::operator<(const A&)
 *    is used to make sure `start <= current <= end' on the whole interval.
 *
 * Each frame, `current' is passed to the user-defined function LerpFunc along with
 * a pointer to the user-defined data.
 */

#ifndef CLERPFUNCTIONINTERVAL_H_
#define CLERPFUNCTIONINTERVAL_H_

#include "directbase.h"
#include "cLerpInterval.h"

template<typename T>
class CLerpFunctionInterval : public CLerpInterval
   {
   public:

   typedef void LerpFunc(const T& lerpedData, void* dataPtr);

   CLerpFunctionInterval(const string &name,
                         LerpFunc* lerpFuncPtr,
                         void* dataPtr,
                         double duration,
                         const T& startData,
                         const T& endData,
                         BlendType blend_type);

   private:

   virtual void priv_step(double t);

   LerpFunc* m_lerpFuncPtr;
   void* m_dataPtr;
   T m_startData;
   T m_endData;
   T m_rangeData;
   T m_curData;
   };

template<typename T>
inline
CLerpFunctionInterval<T>::CLerpFunctionInterval(const string &name,
                                                LerpFunc* lerpFuncPtr,
                                                void* dataPtr,
                                                double duration,
                                                const T& startData,
                                                const T& endData,
                                                BlendType blend_type)
   : CLerpInterval(name, duration, blend_type),
     m_lerpFuncPtr(lerpFuncPtr),
     m_dataPtr(dataPtr),
     m_startData(startData),
     m_endData(endData)
   {
   m_rangeData = m_endData - m_startData;
   m_curData = m_startData;
   }

template<typename T>
inline
void CLerpFunctionInterval<T>::priv_step(double t)
   {
   CLerpInterval::priv_step(t);

   m_curData = m_startData + m_rangeData * compute_delta(t);
   if     (m_curData < m_startData) { m_curData = m_startData; }
   else if(m_endData < m_curData  ) { m_curData = m_endData  ; }

   (*m_lerpFuncPtr)(m_curData, m_dataPtr);
   }

#endif /* CLERPFUNCTIONINTERVAL_H_ */
