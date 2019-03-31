/*
    MIT License

    Copyright (c) 2018-2019 NovusCore

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
#pragma once
#include "../Utils/StringUtils.h"
#include "../ClientHandler.h"
#include "../Message.h"

void ReloadCommand(ClientHandler& clientHandler, std::vector<std::string> subCommands)
{
	u32 hashedSubCommand = StringUtils::fnv1a_32(subCommands[0].c_str(), subCommands[0].size());
	if (hashedSubCommand == "script"_h || hashedSubCommand == "scripts"_h)
	{
		Message reloadMessage;
		reloadMessage.code = MSG_IN_RELOAD_SCRIPTS;
		clientHandler.PassMessage(reloadMessage);
	}
}