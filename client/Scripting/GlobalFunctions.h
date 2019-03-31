#pragma once
#include "../Utils/DebugHandler.h"
#include "AngelBinder.h"

#include "PacketHooks.h"

namespace GlobalFunctions
{
	inline void RegisterPacketCallback(u32 callbackId, asIScriptFunction* callback)
	{
		NC_LOG_MESSAGE("Register Callback!");
		PacketHooks::Register(static_cast<PacketHooks::Hooks>(callbackId), callback);
	}

	inline void Print(std::string& message)
	{
		NC_LOG_MESSAGE("[Script]: %s", message.c_str());
	}
}

void RegisterGlobalFunctions(AngelBinder::Engine* engine)
{
	// Register*Callback functions need to be registered manually since the binder does not support it
	engine->asEngine()->RegisterFuncdef("void PacketCallback(string, uint8)");
	engine->asEngine()->RegisterGlobalFunction("void RegisterPacketCallback(uint32 id, PacketCallback @cb)", asFUNCTION(GlobalFunctions::RegisterPacketCallback), asCALL_CDECL);

	AngelBinder::Exporter::Export(*engine)
		[
			AngelBinder::Exporter::Functions()
			.def("Print", &GlobalFunctions::Print)
		];
}