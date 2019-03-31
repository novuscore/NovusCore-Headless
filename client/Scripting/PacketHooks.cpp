#pragma once
#include "PacketHooks.h"

std::array<std::vector<asIScriptFunction*>, PacketHooks::COUNT> PacketHooks::_hooks;