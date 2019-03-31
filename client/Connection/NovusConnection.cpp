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
#include "../Cryptography/SHA1.h"
#include "../Scripting/PacketHooks.h"

robin_hood::unordered_map<u8, NovusMessageHandler> NovusConnection::InitMessageHandlers()
{
    robin_hood::unordered_map<u8, NovusMessageHandler> messageHandlers;

    messageHandlers[NOVUS_CHALLENGE] =  { NOVUSSTATUS_CHALLENGE,    3,  &NovusConnection::HandleCommandChallenge };
    messageHandlers[NOVUS_PROOF]     =  { NOVUSSTATUS_PROOF,        4,  &NovusConnection::HandleCommandProof };

    return messageHandlers;
}
robin_hood::unordered_map<u8, NovusMessageHandler> const MessageHandlers = NovusConnection::InitMessageHandlers();

bool NovusConnection::Start(std::string username, std::string password)
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

        _username = username;
        _password = password;

        // Hash password
        SHA1Hasher passwordHash;
        passwordHash.UpdateHash(_username + ":" + _password);
        passwordHash.Finish();
        _passwordKey->Bin2BN(passwordHash.GetData(), 20);
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
    if (_status == NOVUSSTATUS_CLOSED)
    {
        AsyncRead();
        return;
    }

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

        if (command == NOVUS_CHALLENGE)
        {
            if (byteBuffer.GetActualSize() == 3)
            {
                // Challenge Failed
                sAuthLogonChallengeHeader challengeHeader;
                challengeHeader.Read(byteBuffer);

				PacketHooks::CallHook(PacketHooks::HOOK_ONLOGIN_CHALLENGE, _username, challengeHeader.result);

                NC_LOG_ERROR("Login Failed: (%u, %u, %u)", (u32)challengeHeader.command, (u32)challengeHeader.error, (u32)challengeHeader.result);

                Close(asio::error::shut_down);
                return;
            }

            size += 116;
        }
        else if (command == NOVUS_PROOF)
        {
            if (byteBuffer.GetActualSize() == 4)
            {
                // Proof Failed
                sAuthLogonProofHeader ProofHeader;
                ProofHeader.Read(byteBuffer);

                NC_LOG_ERROR("Proof Failed: (%u, %u, %u)", (u32)ProofHeader.command, (u32)ProofHeader.error, (u32)ProofHeader.accountFlags);

                Close(asio::error::shut_down);
                return;
            }
            else
            {
                size += 32;
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
    sAuthLogonChallengeData* logonChallenge = reinterpret_cast<sAuthLogonChallengeData*>(GetByteBuffer().GetReadPointer());
    
	PacketHooks::CallHook(PacketHooks::HOOK_ONLOGIN_CHALLENGE, _username, logonChallenge->result);

    BigNumber N, A, B, a, u, x, S, salt, version_challenge, g(logonChallenge->g), k(3);
    B.Bin2BN(logonChallenge->b, 32);
    N.Bin2BN(logonChallenge->n, 32);
    salt.Bin2BN(logonChallenge->salt, 32);
    version_challenge.Bin2BN(logonChallenge->version_challenge, 16);

    // Hash password
    SHA1Hasher shaPassword;
    shaPassword.UpdateHashForBn(2, &salt, _passwordKey);
    shaPassword.Finish();
    x.Bin2BN(shaPassword.GetData(), 20);

    // Random Key Pair
    a.Rand(19 * 8);
    A = g.ModExponential(a, N);

    if ((B % N).IsZero())
        return false;

    assert(A.GetBytes() <= 32);

    // Compute Session Key
    SHA1Hasher sha;
    sha.UpdateHashForBn(2, &A, &B);
    sha.Finish();

    u.Bin2BN(sha.GetData(), 20);
    S = (B - (k * g.ModExponential(x, N))).ModExponential(a + (u * x), N);

    u8 t[32];
    u8 t1[16];
    memcpy(t, S.BN2BinArray(32).get(), 32);

    for (i32 i = 0; i < 16; ++i)
        t1[i] = t[i * 2];

    sha.Init();
    sha.UpdateHash(t1, 16);
    sha.Finish();

    u8 vK[40];
    for (i32 i = 0; i < 20; ++i)
        vK[i * 2] = sha.GetData()[i];

    for (i32 i = 0; i < 16; ++i)
        t1[i] = t[i * 2 + 1];

    sha.Init();
    sha.UpdateHash(t1, 16);
    sha.Finish();

    for (i32 i = 0; i < 20; ++i)
        vK[i * 2 + 1] = sha.GetData()[i];
    _key->Bin2BN(vK, 40);

    // Generate Proof
    sha.Init();
    sha.UpdateHashForBn(1, &N);
    sha.Finish();

    u8 hash[20];
    memcpy(hash, sha.GetData(), 20);
    sha.Init();
    sha.UpdateHashForBn(1, &g);
    sha.Finish();

    for (i32 i = 0; i < 20; ++i)
        hash[i] ^= sha.GetData()[i];

    sha.Init();
    sha.UpdateHash(_username);
    sha.Finish();

    BigNumber t3;
    t3.Bin2BN(hash, 20);
    u8 t4[SHA_DIGEST_LENGTH];
    memcpy(t4, sha.GetData(), SHA_DIGEST_LENGTH);

    sha.Init();
    sha.UpdateHashForBn(1, &t3);
    sha.UpdateHash(t4, SHA_DIGEST_LENGTH);
    sha.UpdateHashForBn(4, &salt, &A, &B, _key);
    sha.Finish();

    BigNumber M;
    M.Bin2BN(sha.GetData(), sha.GetLength());

    cAuthLogonProof logonProof;
    logonProof.command = NOVUS_PROOF;
    std::memcpy(logonProof.A, A.BN2BinArray(32).get(), 32);
    std::memcpy(logonProof.M1, M.BN2BinArray(20).get(), 20);
    std::memset(logonProof.crc_hash, 0, 20);
    logonProof.number_of_keys = 0;
    logonProof.securityFlags = 0;


    ByteBuffer packet(sizeof(cAuthLogonProof));
    packet.Resize(sizeof(cAuthLogonProof));
    std::memcpy(packet.data(), &logonProof, sizeof(cAuthLogonProof));
    packet.WriteBytes(sizeof(cAuthLogonProof));

    // Finish SRP6
    sha.Init();
    sha.UpdateHashForBn(3, &A, &M, _key);
    sha.Finish();
    memcpy(_proofM2, sha.GetData(), 20);

    Send(packet);
    return true;
}

bool NovusConnection::HandleCommandProof()
{
    _status = NOVUSSTATUS_AUTHED;
    sAuthLogonProofData* logonProof = reinterpret_cast<sAuthLogonProofData*>(GetByteBuffer().GetReadPointer());

    if (!memcmp(_proofM2, logonProof->M2, 20))
    {
        /* Send Realmlist here */
        return true;
    }
    else
    {
        NC_LOG_ERROR("Server sent invalid proof");
        return false;
    }
}