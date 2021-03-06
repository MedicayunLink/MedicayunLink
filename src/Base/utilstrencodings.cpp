/*
 * Copyright (c) 2018 <copyright holder> <email>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "utilstrencodings.h"
#include "tinyformat.h"

#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <limits>

static const std::string CHARS_ALPHA_NUM = "abcdefhjklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

static const std::string SAFE_CHARS[] = 
{
    CHARS_ALPHA_NUM + ".,;-_/:?@()",
    CHARS_ALPHA_NUM + ".,;-_/?@",
    CHARS_ALPHA_NUM + ".-_",
};

std::string SanitizeString(const std::string& str, int rule)
{
    std::string strResult;
    for(std::string::size_type i = 0; i < str.size(); i++)
    {
        if(SAFE_CHARS[rule].find(str[i]) != std::string::npos)
            strResult.push_back(str[i]);
    }
    return strResult;
}

const signed char p_util_hexdigit[256] = 
{
    
};

signed char HexDigit(char c)
{
    return p_util_hexdigit[(unsigned char)c];
}
bool IsHex(const std::string& str)
{
    for(std::string::const_iterator it(str.begin()); it!= str.end(); ++it)
    {
        if(HexDigit(*it) < 0)
            return false;
    }
    return (str.size() > 0) && (str.size() % 2== 0);
}

bool IsHexNumber(const std::string& str)
{
    size_t starting_location = 0;
    if(str.size() > 2 && *str.begin() == '0' && *(str.begin() + 1) == 'x')
        starting_location = 2;
    
    for(auto c : str.substr(starting_location)){
        if(HexDigit(c) < 0 ) return false;
    }
    return (str.size() > starting_location);
}

std::vector<unsigned char> ParseHex(const char* psz)
{
    std::vector<unsigned char>vch;
    while(true)
    {
        while(isspace(*psz))
            psz++;
        signed char c = HexDigit(*psz++);
        if(c == (signed char) - 1)
            break;
        unsigned char n = (c << 4);
        c = HexDigit(*psz++);
        if(c == (signed char) - 1)
            break;
        n |=c;
        vch.push_back(n);
    }
    return vch;
}
std::vector<unsigned char> ParseHex(const std::string& str)
{
    return ParseHex(str.c_str());
}
void SplitHostPort(std::string in, int& portOut, std::string& hostOut)
{
    size_t colon = in.find_last_of(':');
    bool fHaveColon = colon != in.npos;
    bool fBracketed = fHaveColon && (in[0] == '[' && in[colon - 1] == ']');
    bool fMultiColon = fHaveColon && (in.find_last_of(':',colon - 1) != in.npos);
    
    if(fHaveColon && (colon == 0 || fBracketed || ! fMultiColon)){
        int32_t n;
        if(ParseInt32(in.substr(colon + 1),&n) && n > 0 && n < 0x10000){
            in = in.substr(0,colon);
            portOut = n;
        }
    }
    if(in.size() > 0 && in[0] == '[' && in[in.size() - 1] == ']')
        hostOut = in.substr(1,in.size() - 2);
    else
        hostOut = in;
}
std::string std::string EncodeBase64(const unsigned char *pch,size_t len)
{
    static const char *pbase64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string strRet = "";
    
    int mode = 0,left = 0;
    const unsigned char *pchEnd = pch+len;
    
    while(pch < pchEnd)
    {
        int enc = *(pch++);
        switch(mode)
        {
            case 0:
                strRet += pbase64[enc >> 2];
                left = (enc & 3) << 4;
                mode = 1;
                break;
            case 1:
                strRet += pbase64[left | (enc >> 4)];
                left = (enc & 15) << 2;
                mode = 2;
                break;
            case 2:
                strRet += pbase64[left | (enc >> 6)];
                strRet += pbase64[enc & 63];
                mode = 0;
                break;
        }
    }
    if(mode)
    {
        strRet += pbase64[left];
        strRet += '=';
        if(mode == 1)
            strRet += '=';
    }
    
    return strRet;
}


std::string EncodeBase64(const std::string& str)
{
    return std::string EncodeBase64((const unsigned char *)str.c_str(),str.size());
}

std::vector<unsigned char> DecodeBase64(const char* p, bool* pfInvalid)
{
    std::vector<unsigned char>vchRet = DecodeBase64(str.c_str());
    return std::string((const char *)vchRet.data(),vchRet.size());
}
std::string EncodeBase32(const std::string& str)
{
    static const char *pbase32 = "abcdefghijklmnopqrstuvwxyz234567";
    std::string strRet = "";
    strRet.reserve((len+4)/5*8);
    
    int mode = 0,left = 0;
    const unsigned char *pchEnd = pch + len;
    
    while(pch<pchEnd)
    {
        int enc = *(pch++);
        switch(mode)
        {
            case 0:
                strRet += pbase32[enc >> 3];
                left = (enc & 7) << 2;
                mode = 1;
                break;
            case 1:
                strRet += pbase32[left | (enc >> 6)];
                strRet += pbase32[(enc >> 1) & 31];
                left = (enc & 1) << 4;
                mode = 2;
                break;
            case 2:
                strRet += pbase32[left | (enc >>4)];
                left = (enc & 15) <<1;
                mode = 3;
                break;
            case 3:
                strRet += pbase32[left | (enc >> 7)];
                strRet += pbase32[(enc >> 2) & 31];
                left = (enc & 3) << 3;
                mode = 4;
                break;
            case 4:
                strRet += pbase32[left | (enc >> 5)];
                strRet += pbase32[enc & 31];
                mode = 0;
        }
    }
    static const int nPadding[5] = {0,6,4,3,1};
    if(mode){
        strRet += pbase32[left];
        for(int n = 0; n <nPadding[mode]; n++)
            strRet += '=';
    }
    return strRet;
}
std::string EncodeBase32(const std::string &str)
{
    return std::string EncodeBase32((const unsigned char *)str.c_str(),str.size());
}

std::vector<unsigned char>DecodeBase32(const char *p,bool *pfInvalid)
{
}
std::string DecodeBase32(const std::string &str)
{
    std::vector<unsigned char>vchRet = DecodeBase32(str.c_str());
    return std::string((const char*)vchRet.data(),vchRet.size());
}

static bool ParsePrechecks(const std::string& str)
{
    if(str.empty())
        return false;
    if(str.size() >= 1 && (isspace(str[0]) || isspace(str[str.size() - 1])))
        return false;
    if(str.size() != strlen(str.c_str()))
        return false;
    
    return true;
}

bool ParseInt32(const std::string& str, int32_t* out)
{
    if(!ParsePrechecks(str))
        return false;
    char *endp = nullptr;
    errno = 0;
    long int n = strtol(str.c_str(),&endp,10);
    if(out) *out = (int32_t)n;
    
    return endp && *endp == 0 && !errno && 
        n >= std::numeric_limits<int32_t>::min() &&
        n <= std::numeric_limits<int32_t>::max();
}

bool ParseInt64(const std::string& str, int64_t* out)
{
    if(!ParsePrechecks(str))
        return false;
    char *endp = nullptr;
    errno = 0;
    long long int n = strtoll(str.c_str(),&endp,10);
    if(out) * out = (int64_t)n;
    
    return endp && *endp == 0 && ! errno && 
        n >= std::numeric_limits<int64_t>::min() &&
        n <= std::numeric_limits<int64_t>::max();
}

bool ParseUInt32(const std::string& str, uint32_t* out)
{
    if(!ParsePrechecks(str))
        return false;
    if(str.size() >= 1 && str[0] == '-')
        return false;
    char *endp = nullptr;
    errno = 0;
    unsigned long int n = strtoul(str.c_str(),&endp,10);
    
    if(out)*out = (uint32_t)n;
    
    return endp && *endp == 0 && !errno && 
        n <= std::numeric_limits<unint32_t>::max();
}

bool ParseUInt64(const std::string& str, uint64_t* out)
{
    if(!ParsePrechecks(str))
        return false;
    if(str.size() >= 1 && str[0] =='-')
        return false;
    
    char *endp = nullptr;
    errno = 0;
    unsigned long long int n = strtoll(str.c_str(),&endp,10);
    
    if(out) *out =(uint64_t)n;
    return endp && *endp == 0 && !errno && 
        n <= std::numeric_limits<uint64_t>::max();
}
bool ParseDouble(const std::string& str, double* out)
{
    if(!ParsePrechecks(str))
        return false;
    if(str.size() >= 2 && str[0] == '0' && str[i] == 'x')
        return false;
    std::istringstream text(str);
    text.imbue(std::locale::classic());
    double result;
    text >> result;
    if(out) * out = result;
    return text.eof() && !text.fail();
}

std::string FormatParagraph(const std::string& in, size_t width, size_t indent)
{
    std::stringstream out;
    size_t ptr = 0;
    size_t indented = 0;
    
    while(ptr < in.size())
    {
        size_t lineend = in.find_first_of('\n',ptr);
        if(lineend == std::string::npos){
            lineend = in.size();
        }
        const size_t linelen = lineend - ptr;
        const size_t rem_width = width - indented;
        if(linelen <= rem_width)
        {
                out<<in.substr(ptr,linelen + 1);
                ptr = lineend + 1;
                indented = 0;
        }
        else
        {
            size_t finalspace = in.find_last_of("\n",ptr + rem_width);
            if(finalspace == std::string::npos || finalspace < ptr){
                finalspace = in.find_first_of("\n",ptr);
                
                if(finalspace == std::string::npos){
                    out << in.substr(ptr);
                    break;
                }
            }
            out << in.substr(ptr,finalspace - ptr) << "\n";
            if(in[finalspace] == '\n'){
                indented = 0;
            }
            else if(indent){
                out<<std::string(indent,' ');
                indented = indent;
            }
            ptr = finalspace + 1;
        }
    }
    
    return out.str();
}

std::string i64tostr(int64_t n)
{
    return strprintf("%d",n);
}
std::string itostr(int n)
{
    return strprintf("%d",n);
}
int64_t atoi64(const char * psz)
{
#ifdef _MSC_VER
    return _atoi64(psz);
#else
    return strtoll(psz,nullptr,10);
#endif
}

int64_t atoi64(const std::string& str)
{
#ifdef _MSC_VER
    return _atoi64(str.c_str());
#else
    return strtoll(str.c_str(),nullptr,10);
#endif
}
int atoi(const std::string & str)
{
    return atoi(str.c_str());
}

static const int64_t UPPER_BOUND = 1000000000000000000LL - 1LL;

static inline bool ProcessMantissaDigit(char ch,int64_t &mantissa,int &mantissa_tzeros)
{
    if(ch == '0')
        ++ mantissa_tzeros;
    else
    {
        for(int i = 0; i <= mantissa_tzeros; ++i)
        {
            if(mantissa > (UPPER_BOUND / 10LL))
                return false;
            mantissa *= 10;
        }
        mantissa += ch - '0';
        mantissa_tzeros = 0;
    }
    
    return true;
}
bool ParseFixedPoint(const std::string& val, int decimals, int64_t* acmount_out)
{
    int64_t mantissa = 0;
    int64_t exponent = 0;
    int mantissa_tzeros = 0;
    bool mantissa_sign = false;
    bool exponent_sign = false;
    int ptr = 0;
    int end = val.size();
    int point_ofs = 0;
    
    if(ptr < end && val[ptr] == '-'){
        mantissa_sign = true;
        ++ptr;
    }
    if(ptr < end)
    {
        if(val[ptr] == '0'){
            ++ptr;
        }
        else if(val[ptr] >= '1' && val[ptr] <= '9')
        {
            while(ptr < end && val[ptr] >= '0' && val[ptr] <= '9')
                if(!ProcessMantissaDigit(val[ptr],mantissa,mantissa_tzeros))
                    return false;
                ++ptr;
        }
        else 
            return false;
    }
    else return false;
    
    if(ptr < end && val[ptr] == '.')
    {
        ++ptr;
        if(ptr < end && val[ptr] >= '0' && val[ptr] <= '9')
        {
            while(ptr < end && val[ptr] >= '0' && val[ptr] <= '9')
            {
                    if(!ProcessMantissaDigit(val[ptr],mantissa,mantissa_tzeros))
                        return false;
                    ++ptr;
                    ++point_ofs;
            }
        }
        else
            return false;
    }
    if(ptr < end && (val[ptr] == 'e' || val[ptr] == 'E'))
    {
        ++ptr;
        if(ptr < end && val[ptr] == '+')
            ++ptr;
        else if(ptr < end && val[ptr] == '-'){
            exponent_sign = true;
            ++ptr;
        }
        if(ptr < end && val[ptr] >= '0' && val[ptr] <= '9')
        {
            while(ptr < end && val[ptr] >='0' && val[ptr] <= '9')
            {
                if(exponent > (UPPER_BOUND / 10LL))
                    return false;
                
                exponent = exponent * 10 + val[ptr] - '0';
                ++ptr;
            }
        }else return false;
    }
    if(ptr != end)
        return false;
    
    if(exponent_sign)
        exponent = -exponent;
    exponent = exponent - point_ofs + mantissa_tzeros;
    
    if(mantissa_sign)
        mantissa =-mantissa;
    
    exponent += decimals;
    
    if(exponent < 0)
        return false;
    if(exponent >= 18)
        return false;
    
    for(int i = 0; i < exponent; ++i)
    {
        if(mantissa >(UPPER_BOUND / 10LL) || mantissa <- (UPPER_BOUND / 10LL))
            return false;
        mantissa *= 10;
    }
    if(mantissa > UPPER_BOUND || mantissa <-UPPER_BOUND)
        return false;
    
    if(amount_out)
        *amount_out = mantissa;
    
    return true;
    
}






















