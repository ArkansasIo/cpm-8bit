// include/types.hpp — 8-bit era fundamental types
#pragma once
#include <cstdint>
#include <cstddef>

using u8  = std::uint8_t;
using s8  = std::int8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using addr_t = u16;          // real-mode segment:offset world

#define UNUSED [[maybe_unused]]
