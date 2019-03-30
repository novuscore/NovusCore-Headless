/*
# MIT License

# Copyright(c) 2018-2019 NovusCore

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions :

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
*/

#include "NovusConnection.h"
#include "../Networking\ByteBuffer.h"
#include "../Utils/DebugHandler.h"

robin_hood::unordered_map<u8, NovusMessageHandler> NovusConnection::InitMessageHandlers()
{
    robin_hood::unordered_map<u8, NovusMessageHandler> messageHandlers;

    messageHandlers[NOVUS_CHALLENGE] =  { NOVUSSTATUS_CHALLENGE,    3,  &NovusConnection::HandleCommandChallenge };
    messageHandlers[NOVUS_PROOF]     =  { NOVUSSTATUS_PROOF,        4,  &NovusConnection::HandleCommandProof };

    return messageHandlers;
}
robin_hood::unordered_map<u8, NovusMessageHandler> const MessageHandlers = NovusConnection::InitMessageHandlers();

bool NovusConnection::Start(std::string username)
{
    try
    {
        _socket->connect(asio::ip::tcp::endpoint(asio::ip::address::from_string(_address), _port));

        cAuthLogonChallenge challenge(username);
        u32 challengeSize = 34 + (u32)username.length();

        ByteBuffer packet(challenge.size);
        packet.Resize(challengeSize);
        std::memcpy(packet.data(), &challenge, challengeSize);
        packet.WriteBytes(challengeSize);

        AsyncRead();
        Send(packet);
        return true;
    }
    catch (asio::system_error error)
    {
        NC_LOG_FATAL(error.what());
        return false;
    }
}

void NovusConnection::HandleRead()
{
    ByteBuffer& byteBuffer = GetByteBuffer();
    while (byteBuffer.GetActualSize())
    {
        u8 command = byteBuffer.GetDataPointer()[0];

        auto itr = MessageHandlers.find(command);
        if (itr == MessageHandlers.end())
        {
            byteBuffer.Clean();
            break;
        }

        // Client attempted incorrect auth step
        if (_status != itr->second.status)
        {
            Close(asio::error::shut_down);
            return;
        }

        u16 size = u16(itr->second.packetSize);
        if (byteBuffer.GetActualSize() < size)
            break;

        if (command == NOVUS_CHALLENGE || command == NOVUS_PROOF)
        {
            u16 expectedSize = command == NOVUS_CHALLENGE ? 3 : 4;
            if (byteBuffer.GetActualSize() > expectedSize)
            {
                size += command == NOVUS_CHALLENGE ? 116 : 32;
            }
        }

        if (!(*this.*itr->second.handler)())
        {
            Close(asio::error::shut_down);
            return;
        }

        byteBuffer.ReadBytes(size);
    }

    AsyncRead();
}


bool NovusConnection::HandleCommandChallenge()
{
    _status = NOVUSSTATUS_PROOF;

    return true;
}

bool NovusConnection::HandleCommandProof()
{
    _status = NOVUSSTATUS_AUTHED;

    return true;
}