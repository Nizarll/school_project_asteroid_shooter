#include <cmath>
#include <array>
#include <random>
#include <print>

#include "objects.hpp"

static constexpr auto ASTEROID_SIZE_RANGE = std::pair<u32, u32>{ 20, 40 };
static constexpr auto ASTEROID_SEGMENTS = std::pair<u32, u32>{ 6, 9 };
static constexpr auto PLANE_WIDTH  = 50;
static constexpr auto PLANE_HEIGHT = 60;
static constexpr auto SPEED_MULTIPLIER = .01;
static constexpr auto PLANE_BASE_SPEED = 12 * SPEED_MULTIPLIER;
static constexpr std::array PLANE_VERTICES {
  rl::Vector2{0.0f, PLANE_HEIGHT / -2.0f},
  rl::Vector2{PLANE_WIDTH / 2.0f, PLANE_HEIGHT / 2.0f},
  rl::Vector2{0.0f, PLANE_HEIGHT / 2.0f - 10},
  rl::Vector2{-PLANE_WIDTH / 2.0f, PLANE_HEIGHT / 2.0f}
};
static bool DEBUG_MODE = false;

static bool point_in_triangle(rl::Vector2 p, rl::Vector2 a, rl::Vector2 b, rl::Vector2 c)
{
  auto sign = [](rl::Vector2 p1, rl::Vector2 p2, rl::Vector2 p3) {
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
  };

  float d1 = sign(p, a, b);
  float d2 = sign(p, b, c);
  float d3 = sign(p, c, a);

  bool has_neg = (d1 < 0) or (d2 < 0) or (d3 < 0);
  bool has_pos = (d1 > 0) or (d2 > 0) or (d3 > 0);

  return not (has_neg and has_pos);
}

static bool segments_intersect(rl::Vector2 p1, rl::Vector2 p2, rl::Vector2 q1, rl::Vector2 q2)
{
  auto orient = [](rl::Vector2 a, rl::Vector2 b, rl::Vector2 c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
  };
  float o1 = orient(p1, p2, q1);
  float o2 = orient(p1, p2, q2);
  float o3 = orient(q1, q2, p1);
  float o4 = orient(q1, q2, p2);
  return (o1 * o2 < 0) && (o3 * o4 < 0);
}

static bool triangles_intersect(const Triangle& a, const Triangle& b)
{
  for (int i = 0; i < 3; i++) {
    if (point_in_triangle(a[i], b[0], b[1], b[2])) return true;
    if (point_in_triangle(b[i], a[0], a[1], a[2])) return true;
  }

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      if (segments_intersect(a[i], a[(i+1)%3], b[j], b[(j+1)%3])) return true;
    }
  }

  return false;
}

static bool collides_with(std::span<const Triangle> a, std::span<const Triangle> b)
{
  for (const auto& triangle_a : a)
  for (const auto& triangle_b : b)
  if (triangles_intersect(triangle_a, triangle_b)) return true;
  return false;
}

void WorldObject::update(f32 dt)
{
  m_pos.x += m_vel.x * dt;
  m_pos.y -= m_vel.y * dt;
}

void Plane::update(f32 dt)
{
  auto vel = rl::Vector2{};
  auto mouse_pos = rl::GetMousePosition();

  if (rl::IsKeyPressed(rl::KEY_E)) {
    DEBUG_MODE = not DEBUG_MODE; // this is debug should be in some context not tied to the plane updatge
  }
  if (rl::IsKeyDown(rl::KEY_D)) vel.x += 1;
  if (rl::IsKeyDown(rl::KEY_Q)) vel.x -= 1;
  if (rl::IsKeyDown(rl::KEY_S)) vel.y -= 1;
  if (rl::IsKeyDown(rl::KEY_W)) vel.y += 1;

  if (rl::IsKeyPressed(rl::KEY_SPACE)) {
    m_shoot_listener(m_pos + rotate(PLANE_VERTICES[0], m_angle));
  }
  vel = rl::Vector2Normalize(vel);

  m_vel = vel * PLANE_BASE_SPEED;
  m_angle = -std::atan2(
    m_pos.x - mouse_pos.x,
    m_pos.y - mouse_pos.y
  );
  WorldObject::update(dt);
}

std::vector<Triangle> Plane::get_mesh() const
{
  std::vector<Triangle> mesh;
  mesh.reserve(PLANE_VERTICES.size() - 2);
  for (std::size_t i = 1; i < PLANE_VERTICES.size() - 1; i++) {
    Triangle t{
      m_pos + rotate(PLANE_VERTICES[0], m_angle),
      m_pos + rotate(PLANE_VERTICES[i], m_angle),
      m_pos + rotate(PLANE_VERTICES[i + 1], m_angle)
    };
    mesh.push_back(t);
  }

  return mesh;
}

void Plane::draw() const
{
  for (usz i = 0; i < PLANE_VERTICES.size(); i++) {
    rl::Vector2 start = rotate(PLANE_VERTICES[i], m_angle);
    rl::Vector2 end   = rotate(PLANE_VERTICES[(i + 1) % PLANE_VERTICES.size()], m_angle);
    rl::DrawLineV(m_pos + start, m_pos + end,  rl::WHITE);
  }

  if (DEBUG_MODE) {
    for(auto tr : get_mesh()) rl::DrawTriangleLines(tr[0], tr[1], tr[2], rl::RED);
    rl::DrawCircleV(m_pos, 2, rl::WHITE);
  }
}

//
Asteroid::Asteroid(
  rl::Vector2 pos,
  bool fixed_size,
  rl::Vector2 size
) : WorldObject()
{
  m_pos = pos;
  m_vel = {};
  auto theta = 0.0f;

  auto vertex_nb = 10;
  if (not fixed_size) {
    std::random_device rd{};
    std::mt19937 eg(rd());
    std::uniform_int_distribution<u32> random_vertex(ASTEROID_SEGMENTS.first, ASTEROID_SEGMENTS.second);
    std::uniform_real_distribution<f32> random_size(ASTEROID_SIZE_RANGE.first, ASTEROID_SIZE_RANGE.second);
    vertex_nb = random_vertex(eg);

    for (u32 i = 0; i < vertex_nb; i++) {
      theta += 2 * std::numbers::pi / vertex_nb;
      m_vertices.emplace_back(rl::Vector2{
        std::cos(theta) * random_size(eg),
        std::sin(theta) * random_size(eg),
      });
      m_max_radius = std::max(m_max_radius, rl::Vector2Length(m_vertices.back()));
    }
  } else {
    for (u32 i = 0; i < vertex_nb; i++) {
      theta += 2 * std::numbers::pi / vertex_nb;
      m_vertices.emplace_back(rl::Vector2{
        std::cos(theta) * size.x,
        std::sin(theta) * size.y,
      });
      m_max_radius = std::max(m_max_radius, rl::Vector2Length(m_vertices.back()));
    }
  }
  for (u32 i = 1; i < vertex_nb - 1; i++) {
    m_triangles.push_back({
      m_vertices[0],
      m_vertices[i],
      m_vertices[i + 1]
    });
  }
}

std::vector<Triangle> Asteroid::get_mesh() const
{
  std::vector<Triangle> mesh;
  mesh.reserve(m_triangles.size());

  for (const auto& tri : m_triangles) {
    mesh.push_back({
      m_pos + tri[0],
      m_pos + tri[1],
      m_pos + tri[2]
    });
  }
  return mesh;
}

void Asteroid::draw() const
{
  for (std::size_t i = 0; i < m_vertices.size(); i++) {
    rl::Vector2 start = m_vertices[i];
    rl::Vector2 end   = m_vertices[(i + 1) % m_vertices.size()];
    rl::DrawLineV(m_pos + start, m_pos + end, rl::WHITE);
  }
  if (DEBUG_MODE) {
    for(auto tr : get_mesh()) rl::DrawTriangleLines(tr[0], tr[1], tr[2], rl::WHITE);
  }
}

bool Asteroid::intersects_with(const Plane& plane) const
{
  auto plane_mesh = plane.get_mesh();
  auto asteroid_mesh = get_mesh();
  return collides_with(asteroid_mesh, plane_mesh);
}

bool Asteroid::out_of_screen() const
{
  auto screen_width = rl::GetScreenWidth();
  auto screen_height = rl::GetScreenHeight();
  return (
    get_pos().x > screen_width or
    get_pos().x < 0 or
    get_pos().y < 0 or
    get_pos().y > screen_height
  );
}

bool Bullet::intersects_with(const Asteroid& asteroid) const
{
  auto asteroid_mesh = asteroid.get_mesh();
  auto bullet = get_mesh();
  return collides_with(bullet, asteroid_mesh);
}
