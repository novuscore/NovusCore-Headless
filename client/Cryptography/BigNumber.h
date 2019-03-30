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

#include <memory>
#include <string>
#include "../NovusTypes.h"

struct bignum_st;

class BigNumber
{
public:
    BigNumber();
    BigNumber(BigNumber const& bn);
    BigNumber(u32);
    ~BigNumber();

    void SetUInt32(u32);
    void SetUInt64(u64);
    u32 GetUInt32();

    std::string BN2Hex() const;
    std::string BN2Dec() const;
    void Hex2BN(char const* string);
    void Bin2BN(u8 const* data, i32 size);
    std::unique_ptr<u8[]> BN2BinArray(size_t size = 0, bool littleEndian = true);

    void Rand(size_t bits);
    BigNumber Exponential(BigNumber const&);
    BigNumber ModExponential(BigNumber const& bigNum1, BigNumber const& bigNum2);

    bool IsZero() const;
    bool IsNegative() const;
    i32 GetBytes(void);

    BigNumber& operator=(BigNumber const& bigNum);
    BigNumber operator+=(BigNumber const& bigNum);
    BigNumber operator+(BigNumber const& bigNum)
    {
        BigNumber t(*this);
        return t += bigNum;
    }
    BigNumber operator-=(BigNumber const& bigNum);
    BigNumber operator-(BigNumber const& bigNum)
    {
        BigNumber t(*this);
        return t -= bigNum;
    }
    BigNumber operator*=(BigNumber const& bigNum);
    BigNumber operator*(BigNumber const& bigNum)
    {
        BigNumber t(*this);
        return t *= bigNum;
    }
    BigNumber operator/=(BigNumber const& bigNum);
    BigNumber operator/(BigNumber const& bigNum)
    {
        BigNumber t(*this);
        return t /= bigNum;
    }
    BigNumber operator%=(BigNumber const& bigNum);
    BigNumber operator%(BigNumber const& bigNum)
    {
        BigNumber t(*this);
        return t %= bigNum;
    }

    struct bignum_st *BigNum() { return _bigNum; }
private:
    struct bignum_st* _bigNum;

};