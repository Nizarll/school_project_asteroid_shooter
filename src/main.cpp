#include <print>
#include "game.hpp"

static constexpr auto FRAMERATE = 60;

auto main() -> int
{
  Game game{FRAMERATE};
  game.run();
  return 0;
}
