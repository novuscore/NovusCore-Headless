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
enum AuthResult
{
    AUTH_SUCCESS                                  = 0x00,
    AUTH_FAIL_BANNED                              = 0x03,
    AUTH_FAIL_UNKNOWN_ACCOUNT                     = 0x04,
    AUTH_FAIL_INCORRECT_PASSWORD                  = 0x05,
    AUTH_FAIL_ALREADY_ONLINE                      = 0x06,
    AUTH_FAIL_NO_TIME                             = 0x07,
    AUTH_FAIL_DB_BUSY                             = 0x08,
    AUTH_FAIL_VERSION_INVALID                     = 0x09,
    AUTH_FAIL_VERSION_UPDATE                      = 0x0A,
    AUTH_FAIL_INVALID_SERVER                      = 0x0B,
    AUTH_FAIL_SUSPENDED                           = 0x0C,
    AUTH_FAIL_FAIL_NOACCESS                       = 0x0D,
    AUTH_SUCCESS_SURVEY                           = 0x0E,
    AUTH_FAIL_PARENTCONTROL                       = 0x0F,
    AUTH_FAIL_LOCKED_ENFORCED                     = 0x10,
    AUTH_FAIL_TRIAL_ENDED                         = 0x11,
    AUTH_FAIL_USE_BATTLENET                       = 0x12,
    AUTH_FAIL_ANTI_INDULGENCE                     = 0x13,
    AUTH_FAIL_EXPIRED                             = 0x14,
    AUTH_FAIL_NO_GAME_ACCOUNT                     = 0x15,
    AUTH_FAIL_CHARGEBACK                          = 0x16,
    AUTH_FAIL_INTERNET_GAME_ROOM_WITHOUT_BNET     = 0x17,
    AUTH_FAIL_GAME_ACCOUNT_LOCKED                 = 0x18,
    AUTH_FAIL_UNLOCKABLE_LOCK                     = 0x19,
    AUTH_FAIL_CONVERSION_REQUIRED                 = 0x20,
    AUTH_FAIL_DISCONNECTED                        = 0xFF
};

#pragma pack(push, 1)
struct cAuthLogonChallenge
{
    cAuthLogonChallenge(std::string _username) : gamename{ 'W', 'o', 'W', 0 },
                            platform{'6', '8', 'x', 0}, 
                            os{'n', 'i', 'W', 0},
                            country{'S', 'U', 'n', 'e'}
    {
        username_length = (u8)_username.length();
        std::memcpy(username, _username.data(), _username.length());

        command = NOVUS_CHALLENGE;
        error = 8;
        size = 30 + username_length;
        version1 = 3;
        version2 = 3;
        version3 = 5;
        build = 12340;
        timezone_bias = 60;
        ip = 16777343;
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

struct cAuthLogonProof
{
    u8 command;
    u8 A[32];
    u8 M1[20];
    u8 crc_hash[20];
    u8 number_of_keys;
    u8 securityFlags;
};

struct sAuthLogonChallengeHeader
{
    u8  command;
    u8  error;
    u8  result;

    void Read(ByteBuffer& buffer)
    {
        buffer.Read<u8>(command);
        buffer.Read<u8>(error);
        buffer.Read<u8>(result);
    }
};

struct sAuthLogonChallengeData
{
    u8 command;
    u8 error;
    u8 result;
    u8 b[32];
    u8 g_length;
    u8 g;
    u8 n_length;
    u8 n[32];
    u8 salt[32];
    u8 version_challenge[16];
    u8 security_flags;
};

struct sAuthLogonProofHeader
{
    u8  command;
    u8  error;
    u16 accountFlags;

    void Read(ByteBuffer& buffer)
    {
        buffer.Read<u8>(command);
        buffer.Read<u8>(error);
        buffer.Read<u16>(accountFlags);
    }
};

struct sAuthLogonProofData
{
    u8  cmd;
    u8  error;
    u8  M2[20];
    u32 AccountFlags;
    u32 SurveyId;
    u16 LoginFlags;
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
        _passwordKey = new BigNumber();
    }

    bool Start(std::string username, std::string password);
    void HandleRead() override;

    bool HandleCommandChallenge();
    bool HandleCommandProof();

    NovusStatus _status;
private:
    std::string _username;
    std::string _password;

    std::string _address;
    u16 _port;

    StreamCrypto* _crypto;
    BigNumber* _key;
    BigNumber* _passwordKey;
    u8 _proofM2[20];
};