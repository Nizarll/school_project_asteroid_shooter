#include <print>
#include "game.hpp"
#include <iostream>
#include <ranges>
#include <raylib.h>
#include "rl.hpp"

static rl::Vector2 random_border_spawn()
{
  const float w = rl::GetScreenWidth();
  const float h = rl::GetScreenHeight();

  switch (rl::GetRandomValue(0, 3))
  {
    case 0: return { 0.0f, (f32)rl::GetRandomValue(0, h) }; // left
    case 1: return { w, (f32)rl::GetRandomValue(0, h) }; // right
    case 2: return { (f32)rl::GetRandomValue(0, w), 0.0f };              // top
    default:return { (f32)rl::GetRandomValue(0, w), h };                 // bottom
  }
}

Game::Game(uint framerate) : m_plane(rl::Vector2(100, 200))
{
  rl::InitWindow(600, 400, "Asteroid Shooter");
  rl::SetTargetFPS(framerate);
  m_plane.on_shoot([this](rl::Vector2 shoot_point) {
    handle_shoot(shoot_point);
  });
}

void Game::handle_shoot(rl::Vector2 shoot_point)
{
  auto dir = rl::Vector2Normalize(shoot_point - m_plane.get_pos());
  dir.y *= -1.0f;
  constexpr float speed = .8f; // tweak
  Bullet bullet(shoot_point);
  bullet.set_vel(dir * speed);

  m_bullets.emplace_back(bullet);
}

void Game::spawn_asteroid()
{
    rl::Vector2 spawn = random_border_spawn();
    rl::Vector2 dir   = rl::Vector2Normalize(m_plane.get_pos() - spawn);
    dir.y *= -1.0f;

    constexpr float speed = .2f; // tweak
    Asteroid a(spawn);
    a.set_vel(dir * speed);

    m_asteroids.emplace_back(a);
}

void Game::spawn_asteroids_around_bullet(const Bullet& bullet)
{
  constexpr float speed = .2f;
  for (int i = 0; i < rl::GetRandomValue(2, 4); i++) {

    auto dir = rotate(bullet.get_vel(), rl::GetRandomValue(-30, 30)  * 3.14 / 180);
    Asteroid a(bullet.get_pos(), true, {10, 15});
    a.set_vel(dir * speed);
    m_asteroids.emplace_back(a);
  }
}


void Game::reset()
{
  m_score = 0;
  m_game_over = false;
  m_asteroids.clear();
  m_bullets.clear();
  m_plane.set_pos({100, 200});
  m_elapsed_time = 0.0f;
}

void Game::run()
{
  while (not rl::WindowShouldClose())
  {
    int screen_width = rl::GetScreenWidth();
    int screen_height = rl::GetScreenHeight();

    if (m_game_over and rl::IsKeyPressed(rl::KEY_R)) {
      reset();
    }

    if (not m_game_over) {
      static auto timer = .0f;
      auto dt = rl::GetFrameTime() * 1000;
      timer += dt;
      m_elapsed_time += dt;

      constexpr float base_spawn_interval = 1000.0f;
      constexpr float min_spawn_interval = 200.0f;
      constexpr float decrease_rate = 0.00005f;

      float spawn_interval = base_spawn_interval - (m_elapsed_time * decrease_rate);
      if (spawn_interval < min_spawn_interval) spawn_interval = min_spawn_interval;

      if (timer >= spawn_interval) {
        spawn_asteroid();
        timer = 0;
      }
      {
        m_plane.update(dt);
        for (auto& asteroid : m_asteroids)
        {
          asteroid.update(dt);
          if (asteroid.intersects_with(m_plane)) {
            m_game_over = true;
          }
          if (asteroid.out_of_screen()) asteroid.set_destroyed();
        }
        for (auto& bullet : m_bullets) {
          bullet.update(dt);
          for (auto& asteroid : m_asteroids) {
            if (bullet.intersects_with(asteroid)) {
              asteroid.set_destroyed();
              bullet.set_destroyed();
              spawn_asteroids_around_bullet(bullet);
              m_score++;
            }
          }
          if (bullet.out_of_screen()) bullet.set_destroyed();
        }
      }
      std::erase_if(m_asteroids, [](const Asteroid& asteroid) {
        return asteroid.is_destroyed();
      });
      std::erase_if(m_bullets, [](const Bullet& bullet) {
        return bullet.is_destroyed();
      });
    }

    rl::BeginDrawing();
    rl::ClearBackground(rl::BLACK);
    for (auto asteroid : m_asteroids) asteroid.draw();
    for (auto bullet : m_bullets) bullet.draw();

    m_plane.draw();

    rl::DrawText(rl::TextFormat("Score: %d", m_score), 10, 10, 24, rl::WHITE);

    if (m_game_over) {
      const char* game_over_text = "GAME OVER";
      int text_width = rl::MeasureText(game_over_text, 48);
      rl::DrawText(game_over_text, (screen_width - text_width) / 2, screen_height / 2 - 24, 48, rl::RED);

      const char* restart_text = "Press R to Restart";
      int restart_width = rl::MeasureText(restart_text, 24);
      rl::DrawText(restart_text, (screen_width - restart_width) / 2, screen_height / 2 + 40, 24, rl::WHITE);
    }

    rl::EndDrawing();
  }
}
