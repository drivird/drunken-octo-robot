/*
 * cLerpFunctionInterval.h
 *
 *  Created on: 2012-05-30
 *      Author: dri
 *
 * This class is meant to emulate the capabilities of python LerpInterval class. It is by no
 * mean a full implementation. It only contains what is needed to translate the Panda3D tutorials.
 *
 */

#ifndef CLERPFUNCTIONINTERVAL_H_
#define CLERPFUNCTIONINTERVAL_H_

#include "directbase.h"
#include "cLerpInterval.h"

class CLerpFunctionInterval : public CLerpInterval
   {
   public:

   typedef void LerpFunc(const double& lerpedData, void* dataPtr);

   CLerpFunctionInterval(const string &name,
                         LerpFunc* lerpFuncPtr,
                         void* dataPtr,
                         double duration,
                         double startData,
                         double endData,
                         BlendType blend_type);

   private:

   virtual void priv_step(double t);

   LerpFunc* m_lerpFuncPtr;
   void* m_dataPtr;
   double m_startData;
   double m_endData;
   double m_rangeData;
   double m_curData;
   };

#endif /* CLERPFUNCTIONINTERVAL_H_ */
