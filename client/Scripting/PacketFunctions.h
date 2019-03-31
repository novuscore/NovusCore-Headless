#pragma once
#include "../Utils/DebugHandler.h"
#include "../Config/ConfigHandler.h"
#include "AngelBinder.h"

#include "../Connection/NovusConnection.h"
#include "ScriptEngine.h"

namespace PacketFunctions
{
	inline void SendLoginChallenge(std::string username, std::string password)
	{
		NC_LOG_MESSAGE("Send login!");
		
		NovusConnection novusConnection(new asio::ip::tcp::socket(*ScriptEngine::GetIOService()), ConfigHandler::GetOption<std::string>("address", "127.0.0.1"), ConfigHandler::GetOption<u16>("port", 3724));
		novusConnection.Start(username, password);
	}

	inline void HelloWorld()
	{
		NC_LOG_MESSAGE("Hello World!");
	}
}

void RegisterPacketFunctions(AngelBinder::Engine* engine)
{
	engine->asEngine()->SetDefaultNamespace("Packet");
	AngelBinder::Exporter::Export(*engine)
		[
			AngelBinder::Exporter::Functions()
			.def("HelloWorld", &PacketFunctions::HelloWorld)
			.def("SendLoginChallenge", &PacketFunctions::SendLoginChallenge)
		];

	engine->asEngine()->SetDefaultNamespace("");
}