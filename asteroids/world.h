/*
 * world.h
 *
 *  Created on: 2012-05-17
 *      Author: dri
 */

#ifndef WORLD_H_
#define WORLD_H_


class World
   {
   public:

   World(WindowFramework* windowFrameworkPtr);

   private:

   enum Key
      {
      K_turn_left = 0,
      K_turn_right,
      K_accel,
      K_fire,
      K_keys
      };

   // Constants that will control the behavior of the game. It is good to group
   // constants like this so that they can be changed once without having to find
   // everywhere they are used in code
   static const int   SPRITE_POS     =  55; // At default field of view and a depth of 55, the screen
                                            // dimensions is 40x30 units
   static const int   SCREEN_X       =  20; // Screen goes from -20 to 20 on X
   static const int   SCREEN_Y       =  15; // Screen goes from -15 to 15 on Y
   static const int   TURN_RATE      = 360; // Degrees ship can turn in 1 second
   static const int   ACCELERATION   =  10; // Ship acceleration in units/sec/sec
   static const int   MAX_VEL        =   6; // Maximum ship velocity in units/sec
   static const int   MAX_VEL_SQ     =  36; // Square of the ship velocity
   static const float PI             = 3.14159265;
   static const float DEG_TO_RAD     = 0.0174532925; // translates degrees to radians for sin and cos
   static const int   BULLET_LIFE    =   2; // How long bullets stay on screen before removed
   static const float BULLET_REPEAT  = 0.2; // How often bullets can be fired
   static const int   BULLET_SPEED   =  10; // Speed bullets move
   static const int   AST_TEX_NB     =   3; // Number of different asteroid textures
   static const int   AST_MIN_DIST   =   5; // Minimal distance for asteroid generation
   static const int   AST_INIT_NB    =  10; // Initial number of asteroids
   static const int   AST_INIT_VEL   =   1; // Velocity of the largest asteroids
   static const int   AST_INIT_SCALE =   3; // Initial asteroid scale
   static const float AST_VEL_SCALE  = 2.2; // How much asteroid speed multiplies when broken up
   static const float AST_SIZE_SCALE = 0.6; // How much asteroid scale changes when broken up
   static const float AST_MIN_SCALE  = 1.1; // If and asteroid is smaller than this and is hit,
                                            // it disappears instead of splitting up

   enum Alignment
      {
      A_center  = TextNode::A_center,
      A_left   = TextNode::A_left
      };

   World(); // to prevent use of the default constructor
   NodePath load_object(const string& tex = "",
                        const LPoint2f& pos = LPoint2f(0,0),
                        float depth = SPRITE_POS,
                        float scale = 1,
                        bool transparency = true) const;
   NodePath gen_label_text(const string& text, int i) const;
   void set_key(Key key, bool val);
   void set_velocity(NodePath objNp, const LVecBase3f& val);
   LVecBase3f get_velocity(NodePath objNp) const;
   void set_expires(NodePath objNp, float val);
   float get_expires(NodePath objNp) const;
   void spawn_asteroids();
   AsyncTask::DoneStatus game_loop(GenericAsyncTask* taskPtr);
   void update_pos(NodePath objNp, double dt);
   void asteroid_hit(list<NodePath>::iterator astPtr);
   void update_ship(double dt);
   void fire(double time);

   NodePath onscreen_text(const string& text, const Colorf& fg, const LPoint2f& pos, Alignment align, float scale) const;
   static void sys_exit(const Event* eventPtr, void* dataPtr);
   template<int key, bool value> static void call_set_key(const Event* eventPtr, void* dataPtr);
   static AsyncTask::DoneStatus call_game_loop(GenericAsyncTask* taskPtr, void* dataPtr);
   static AsyncTask::DoneStatus call_restart(GenericAsyncTask* taskPtr, void* dataPtr);
   void restart();

   PT(WindowFramework) m_windowFrameworkPtr;
   PT(GenericAsyncTask) m_gameTaskPtr;
   NodePath m_titleNp;
   NodePath m_escapeTextNp;
   NodePath m_leftKeyTextNp;
   NodePath m_rightKeyTextNp;
   NodePath m_upKeyTextNp;
   NodePath m_spaceKeyTextNp;
   NodePath m_bgNp;
   NodePath m_shipNp;
   vector<bool> m_keys;
   list<NodePath> m_bullets;
   list<NodePath> m_asteroids;
   double m_last;
   double m_nextBullet;
   bool m_alive;
   };

#endif /* WORLD_H_ */
