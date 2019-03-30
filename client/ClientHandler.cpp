#include "ClientHandler.h"
#include "NovusTypes.h"
#include "Utils/Timer.h"
#include "Networking/Opcode/Opcode.h"

#include <thread>
#include <iostream>

ClientHandler::ClientHandler(f32 targetTickRate)
    : _isRunning(false)
    , _inputQueue(256)
    , _outputQueue(256)
{
    _targetTickRate = targetTickRate;
}

ClientHandler::~ClientHandler()
{
}

void ClientHandler::PassMessage(Message& message)
{
    _inputQueue.enqueue(message);
}

bool ClientHandler::TryGetMessage(Message& message)
{
    return _outputQueue.try_dequeue(message);
}

void ClientHandler::Start()
{
    if (_isRunning)
        return;

    std::thread thread = std::thread(&ClientHandler::Run, this);
    thread.detach();
}

void ClientHandler::Stop()
{
    if (!_isRunning)
        return;

    Message message;
    message.code = MSG_IN_EXIT;
    PassMessage(message);
}

void ClientHandler::Run()
{
    Timer timer;
    while (true)
    {
        f32 deltaTime = timer.GetDeltaTime();
        timer.Tick();

        if (!Update())
            break;

        // Wait for tick rate, this might be an overkill implementation but it has the even tickrate I've seen - MPursche
        f32 targetDelta = 1.0f / _targetTickRate;

        for (deltaTime = timer.GetDeltaTime(); deltaTime < targetDelta - 0.0025f; deltaTime = timer.GetDeltaTime())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        for (deltaTime = timer.GetDeltaTime(); deltaTime < targetDelta; deltaTime = timer.GetDeltaTime())
        {
            std::this_thread::yield();
        }
    }

    // Clean up stuff here

    Message exitMessage;
    exitMessage.code = MSG_OUT_EXIT_CONFIRM;
    _outputQueue.enqueue(exitMessage);
}

bool ClientHandler::Update()
{
    Message message;
    while (_inputQueue.try_dequeue(message))
    {
        if (message.code == -1)
            assert(false);

        if (message.code == MSG_IN_EXIT)
            return false;

        if (message.code == MSG_IN_PING)
        {
            Message pongMessage;
            pongMessage.code = MSG_OUT_PRINT;
            pongMessage.message = new std::string("PONG!");
            _outputQueue.enqueue(pongMessage);
        }
    }

    return true;
}