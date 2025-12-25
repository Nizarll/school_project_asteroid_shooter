#pragma once

#include "utils.hpp"
#include "objects.hpp"
#include <vector>

class Game
{
  Plane m_plane;
  std::vector<Asteroid> m_asteroids;
  std::vector<Bullet> m_bullets;
  u32 m_score = 0;
  bool m_game_over = false;
  float m_elapsed_time = 0.0f;
public:
  Game(uint fps);
  void run();
private:
  void spawn_asteroid();
  void spawn_asteroids_around_bullet(const Bullet& bullet);
  void handle_shoot(rl::Vector2 shoot_point);
  void reset();
};
