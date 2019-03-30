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
#pragma once

#include <asio\ip\tcp.hpp>
#include "../Networking\BaseSocket.h"
#include "../Cryptography\BigNumber.h"
#include "../Cryptography\StreamCrypto.h"
#include <robin_hood.h>

enum NovusCommand
{
    NOVUS_CHALLENGE         = 0x00,
    NOVUS_PROOF             = 0x01
};
enum NovusStatus
{
    NOVUSSTATUS_CHALLENGE   = 0,
    NOVUSSTATUS_PROOF       = 1,
    NOVUSSTATUS_AUTHED      = 2,
    NOVUSSTATUS_CLOSED      = 3
};

#pragma pack(push, 1)
struct cAuthLogonChallenge
{
    cAuthLogonChallenge(std::string _username) : gamename{ 'W', 'o', 'W', 0 },
                            platform{'6', '8', 'x', 0}, 
                            os{'n', 'i', 'W', 0},
                            country{'S', 'U', 'n', 'e'}
    {
        command = 0;
        error = 8;
        size = 35;
        version1 = 3;
        version2 = 3;
        version3 = 5;
        build = 12340;
        timezone_bias = 60;
        ip = 16777343;
        username_length =  (u8)_username.length();
        std::memcpy(username, _username.data(), _username.length());
    }
    u8  command;
    u8  error;
    u16 size;
    u8  gamename[4];
    u8  version1;
    u8  version2;
    u8  version3;
    u16 build;
    u8  platform[4];
    u8  os[4];
    u8  country[4];
    u32 timezone_bias;
    u32 ip;
    u8  username_length;
    char username[16];
};

class NovusConnection;
struct NovusMessageHandler
{
    NovusStatus status;
    size_t packetSize;
    bool (NovusConnection::*handler)();
};
#pragma pack(pop)

class NovusConnection : public Common::BaseSocket
{
public:
    static robin_hood::unordered_map<u8, NovusMessageHandler> InitMessageHandlers();

    NovusConnection(asio::ip::tcp::socket* socket, std::string address, u16 port) : Common::BaseSocket(socket), _status(NOVUSSTATUS_CHALLENGE), _crypto(), _address(address), _port(port)
    { 
        _crypto = new StreamCrypto();
        _key = new BigNumber();
    }

    bool Start(std::string username);
    void HandleRead() override;

    bool HandleCommandChallenge();
    bool HandleCommandProof();

    NovusStatus _status;
private:
    std::string _address;
    u16 _port;

    StreamCrypto* _crypto;
    BigNumber* _key;
};