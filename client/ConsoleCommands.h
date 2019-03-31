#pragma once

#include <map>
#include <vector>
#include <sstream>
#include <iterator>
#include <functional>

#include "Utils/StringUtils.h"
#include "Utils/DebugHandler.h"

#include "ConsoleCommands/QuitCommand.h"
#include "ConsoleCommands/PingCommand.h"
#include "ConsoleCommands/ReloadCommand.h"

class ConsoleCommandHandler
{
public:
	ConsoleCommandHandler()
	{
		RegisterCommand("quit"_h, &QuitCommand);
		RegisterCommand("ping"_h, &PingCommand);
		RegisterCommand("reload"_h, &ReloadCommand);
	}

	void HandleCommand(ClientHandler& clientHandler, std::string& command)
	{
		if (command.size() == 0)
			return;

		std::vector<std::string> splitCommand = StringUtils::SplitString(command);
		u32 hashedCommand = StringUtils::fnv1a_32(splitCommand[0].c_str(), splitCommand[0].size());

		auto commandHandler = commandHandlers.find(hashedCommand);
		if (commandHandler != commandHandlers.end())
		{
			splitCommand.erase(splitCommand.begin());
			commandHandler->second(clientHandler, splitCommand);
		}
		else
		{
			NC_LOG_WARNING("Unhandled command: " + command);
		}
	}
private:
	void RegisterCommand(u32 id, const std::function<void(ClientHandler&, std::vector<std::string>)>& handler)
	{
		commandHandlers.insert_or_assign(id, handler);
	}

	std::map<u16, std::function<void(ClientHandler&, std::vector<std::string>)>> commandHandlers = {};
};
