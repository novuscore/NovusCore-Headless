#include <iostream>
#include <chrono>
#include <future>
#include <algorithm>
#include <asio.hpp>

#include "Connection/NovusConnection.h"
#include "Config/ConfigHandler.h"
#include "Utils/DebugHandler.h"

#include "ConsoleCommands.h"
#include "ClientHandler.h"

std::string GetLineFromCin() 
{
    std::string line;
    std::getline(std::cin, line);
    return line;
}

i32 main()
{
    /* Load Config Handler for server */
    if (!ConfigHandler::Load("client_configuration.json"))
    {
        std::getchar();
        return 0;
    }
    std::string username = "ADMIN";
    std::string password = "ADMIN";

    ClientHandler clientHandler(ConfigHandler::GetOption<f32>("tickRate", 30));
    clientHandler.Start();

    asio::io_service io_service(2);
    NovusConnection novusConnection(new asio::ip::tcp::socket(io_service), ConfigHandler::GetOption<std::string>("address", "127.0.0.1"), ConfigHandler::GetOption<u16>("port", 3724));
    novusConnection.Start(username, password);

    srand((u32)time(NULL));
    std::thread run_thread([&]
    {
        io_service.run();
    });    

    NC_LOG_MESSAGE("Client established connection to Authserver.");

    ConsoleCommandHandler consoleCommandHandler;
    bool futureAvailable = true;
    auto future = std::async(std::launch::async, GetLineFromCin);
    while (true)
    {
        Message message;
        bool shouldExit = false;
        while (clientHandler.TryGetMessage(message))
        {
            if (message.code == MSG_OUT_EXIT_CONFIRM)
            {
                shouldExit = true;
                break;
            }
            if (message.code == MSG_OUT_PRINT)
            {
                NC_LOG_MESSAGE(*message.message);
                delete message.message;
            }
        }

        if (shouldExit)
            break;

        if (futureAvailable && future.wait_for(std::chrono::milliseconds(50)) == std::future_status::ready)
        {
            std::string command = future.get();
            std::transform(command.begin(), command.end(), command.begin(), ::tolower); // Convert command to lowercase

            consoleCommandHandler.HandleCommand(clientHandler, command);
            if (command != "quit")
            {
                future = std::async(std::launch::async, GetLineFromCin);
            }
            else
            {
                futureAvailable = false;
            }
        }
    }

    io_service.stop();
    while (!io_service.stopped())
    {
        std::this_thread::yield();
    }

    return 0;
}
