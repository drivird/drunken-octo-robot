/*
 * cLerpFunctionInterval.cpp
 *
 *  Created on: 2012-05-30
 *      Author: dri
 */

#include "cLerpFunctionInterval.h"

CLerpFunctionInterval::CLerpFunctionInterval(const string &name,
                                             LerpFunc* lerpFuncPtr,
                                             void* dataPtr,
                                             double duration,
                                             double startData,
                                             double endData,
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

void CLerpFunctionInterval::priv_step(double t)
   {
   CLerpInterval::priv_step(t);

   m_curData = m_startData + m_rangeData * compute_delta(t);
   if     (m_curData < m_startData) { m_curData = m_startData; }
   else if(m_endData < m_curData  ) { m_curData = m_endData  ; }

   (*m_lerpFuncPtr)(m_curData, m_dataPtr);
   }
