// include/types.hpp — 8-bit era fundamental types
#pragma once
#include <stddef.h>

using u8  = __UINT8_TYPE__;
using s8  = __INT8_TYPE__;
using u16 = __UINT16_TYPE__;
using u32 = __UINT32_TYPE__;
using addr_t = u16;          // real-mode segment:offset world

#define UNUSED [[maybe_unused]]
