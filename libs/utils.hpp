#pragma once

#include <cstdint>
#include <type_traits>
#include "rl.hpp"

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using f32 = float;
using f64 = double;
using usz = std::size_t;
using isz = std::make_signed_t<usz>;
using uint = unsigned int;

static inline rl::Vector2 rotate(const rl::Vector2& v, float angle) {
  return {
    v.x * std::cos(angle) - v.y * std::sin(angle),
    v.x * std::sin(angle) + v.y * std::cos(angle)
  };
}
