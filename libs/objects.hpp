#pragma once
#include "utils.hpp"
#include <raylib.h>
#include <vector>
#include <span>
#include <array>
#include <functional>

using Triangle = std::array<rl::Vector2, 3>;

class WorldObject
{
protected:
  rl::Vector2 m_pos;
  rl::Vector2 m_vel;
  bool m_destroyed = false;
public:
  virtual void draw() const = 0;
  virtual void update(f32 dt);
  virtual ~WorldObject() = default;

  rl::Vector2 get_pos() const { return m_pos; }
  rl::Vector2 get_vel() const { return m_vel; }
  bool is_destroyed() const { return m_destroyed; }
  void set_vel(rl::Vector2 vel) { m_vel = vel; }
  void set_pos(rl::Vector2 pos) { m_pos = pos; }
  void set_destroyed() { m_destroyed = true; }
};

class Plane final : public WorldObject
{
  f32 m_angle{};
  std::function<void(rl::Vector2)> m_shoot_listener;
public:
  Plane(rl::Vector2 pos) { m_pos = pos; }
  void draw() const override;
  void update(f32 dt) override;
  std::vector<Triangle> get_mesh() const;
  void on_shoot(std::function<void(rl::Vector2)> listener) { m_shoot_listener = listener; }
};

class Asteroid : public WorldObject
{
  int m_life{};
  f32 m_max_radius{};
  std::vector<rl::Vector2> m_vertices;
  std::vector<Triangle> m_triangles;
public:
  Asteroid(
    rl::Vector2 pos,
    bool fixed_size = false,
    rl::Vector2 size = {0, 0}
  );
  void draw() const override;
  std::vector<Triangle> get_mesh() const;
  bool intersects_with(const Plane& plane) const;
  bool out_of_screen() const;
};

class Bullet final : public Asteroid
{
public:
  Bullet(rl::Vector2 pos) : Asteroid(pos, true, {4, 6}) {}
  bool intersects_with(const Asteroid& asteroid) const;
};
