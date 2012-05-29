/*
 * world.h
 *
 *  Created on: 2012-05-22
 *      Author: dri
 */

#ifndef WORLD_H_
#define WORLD_H_

#include "pandaFramework.h"
#include "robot.h"

class World
   {
   public:

   World(WindowFramework* windowFrameworkPtr);

   private:

   enum RobotId
      {
      R_robot1,
      R_robot2
      };

   enum PunchId
      {
      P_left_punch,
      P_right_punch
      };

   NodePath gen_label_text(const string& text, int i) const;
   void try_punch(RobotId robotId, PunchId punchId);
   void check_punch(RobotId robotId);
   void setup_lights();

   World(); // to prevent use of the default constructor
   template<int robotId, int punchId> static void call_try_punch(const Event* eventPtr, void* dataPtr);
   template<int robotId> static void call_check_punch(void* dataPtr);
   static void sys_exit(const Event* eventPtr, void* dataPtr);
   static AsyncTask::DoneStatus step_interval_manager(GenericAsyncTask *taskPtr, void *dataPtr);

   PT(WindowFramework) m_windowFrameworkPtr;
   NodePath m_titleNp;
   NodePath m_escapeEventTextNp;
   NodePath m_aKeyEventTextNp;
   NodePath m_sKeyEventTextNp;
   NodePath m_kKeyEventTextNp;
   NodePath m_lKeyEventTextNp;
   NodePath m_ringNp;
   Robot m_robot1;
   Robot m_robot2;
   };

#endif /* WORLD_H_ */
