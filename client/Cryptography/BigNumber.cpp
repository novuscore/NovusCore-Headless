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

#include "BigNumber.h"
#include <openssl/bn.h>
#include <cstring>
#include <algorithm>
#include <memory>
#include "../NovusTypes.h"

BigNumber::BigNumber() : _bigNum(BN_new()) { }

BigNumber::BigNumber(BigNumber const& bigNum) : _bigNum(BN_dup(bigNum._bigNum)) { }
BigNumber::BigNumber(u32 val) : _bigNum(BN_new())
{
    SetUInt32(val);
}
BigNumber::~BigNumber()
{
    BN_free(_bigNum);
}

void BigNumber::SetUInt32(u32 val)
{
    BN_set_word(_bigNum, val);
}
void BigNumber::SetUInt64(u64 val)
{
    SetUInt32((u32)(val >> 32));
    BN_lshift(_bigNum, _bigNum, 32);
    SetUInt32((u32)(val & 0xFFFFFFFF));
}
u32 BigNumber::GetUInt32()
{
    return (u32)BN_get_word(_bigNum);
}

std::string BigNumber::BN2Hex() const
{
    char* ch = BN_bn2hex(_bigNum);
    std::string ret = ch;
    OPENSSL_free(ch);
    return ret;
}
std::string BigNumber::BN2Dec() const
{
    char* ch = BN_bn2dec(_bigNum);
    std::string ret = ch;
    OPENSSL_free(ch);
    return ret;
}
void BigNumber::Hex2BN(char const* str)
{
    BN_hex2bn(&_bigNum, str);
}
void BigNumber::Bin2BN(u8 const* data, i32 size)
{
    u8* array = new u8[size];

    for (i32 i = 0; i < size; i++)
        array[i] = data[size - 1 - i];

    BN_bin2bn(array, size, _bigNum);

    delete[] array;
}
std::unique_ptr<u8[]> BigNumber::BN2BinArray(size_t size, bool littleEndian)
{
    i32 numBytes = GetBytes();
    i32 neededSize = ((i32)size >= numBytes) ? (i32)size : numBytes;

    u8* array = new u8[neededSize];

    // Zero Fill remaining bytes if needed size is greater than the length of BigNumber
    if (neededSize > numBytes)
        memset((void*)array, 0, neededSize);

    BN_bn2bin(_bigNum, (unsigned char *)array);

    // Openssl bignumbers store data as big endian format by default, we should reverse the array if we desire little endian
    if (littleEndian)
        std::reverse(array, array + numBytes);

    std::unique_ptr<u8[]> ret(array);
    return ret;
}

void BigNumber::Rand(size_t bits)
{
    BN_rand(_bigNum, (i32)bits, 0, 1);
}
BigNumber BigNumber::Exponential(BigNumber const& bigNum)
{
    BigNumber ret;
    BN_CTX *bigNumContext;

    bigNumContext = BN_CTX_new();
    BN_exp(ret._bigNum, _bigNum, bigNum._bigNum, bigNumContext);
    BN_CTX_free(bigNumContext);

    return ret;
}
BigNumber BigNumber::ModExponential(BigNumber const& bn1, BigNumber const& bn2)
{
    BigNumber ret;
    BN_CTX *bigNumContext;

    bigNumContext = BN_CTX_new();
    BN_mod_exp(ret._bigNum, _bigNum, bn1._bigNum, bn2._bigNum, bigNumContext);
    BN_CTX_free(bigNumContext);

    return ret;
}

bool BigNumber::IsZero() const
{
    return BN_is_zero(_bigNum);
}
bool BigNumber::IsNegative() const
{
    return BN_is_negative(_bigNum);
}
i32 BigNumber::GetBytes(void)
{
    return BN_num_bytes(_bigNum);
}

BigNumber& BigNumber::operator=(BigNumber const& bigNum)
{
    if (this == &bigNum)
        return *this;

    BN_copy(_bigNum, bigNum._bigNum);
    return *this;
}
BigNumber BigNumber::operator+=(BigNumber const& bigNum)
{
    BN_add(_bigNum, _bigNum, bigNum._bigNum);
    return *this;
}
BigNumber BigNumber::operator-=(BigNumber const& bigNum)
{
    BN_sub(_bigNum, _bigNum, bigNum._bigNum);
    return *this;
}
BigNumber BigNumber::operator*=(BigNumber const& bigNum)
{
    BN_CTX *bigNumContext;

    bigNumContext = BN_CTX_new();
    BN_mul(_bigNum, _bigNum, bigNum._bigNum, bigNumContext);
    BN_CTX_free(bigNumContext);

    return *this;
}
BigNumber BigNumber::operator/=(BigNumber const& bigNum)
{
    BN_CTX *bigNumContext;

    bigNumContext = BN_CTX_new();
    BN_div(_bigNum, nullptr, _bigNum, bigNum._bigNum, bigNumContext);
    BN_CTX_free(bigNumContext);

    return *this;
}
BigNumber BigNumber::operator%=(BigNumber const& bigNum)
{
    BN_CTX *bigNumContext;

    bigNumContext = BN_CTX_new();
    BN_mod(_bigNum, _bigNum, bigNum._bigNum, bigNumContext);
    BN_CTX_free(bigNumContext);

    return *this;
}
