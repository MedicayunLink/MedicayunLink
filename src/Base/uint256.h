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

#ifndef BITCOIN_UINT256_H
#define BITCOIN_UINT256_H

#include <assert.h>
#include <cstring>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <vector>
#include "../crypto/common.h"

template<unsigned int BITS>
class base_blob
{
protected:
        enum {WIDTH=BITS/8};
        uint8_t data[WIDTH];
public:
        base_blob()
        {
            memset(data,0,sizeof(data));
        }
        explicit base_blob(const std::vector<unsigned char>& vch);
        
        bool IsNull()const
        {
            for(int i = 0 ;i < WIDTH; i ++)
                if(data[i] != 0)
                    return false;
                return true;
        }
        void SetNull()
        {
            memset(data,0,sizeof(data));
        }
        inline int Compare(const base_blob &other)const{
            return memcmp(data,other.data,sizeof(data));
        }
        
        friend inline bool operator==(const base_blob &a,const base_blob& b){return a.Compare(b) == 0;}
        friend inline bool operator!=(const base_blob &a,const base_blob& b){return a.Compare(b) != 0;}
        friend inline bool operator<(const base_blob &a,const base_blob& b){return a.Compare(b) <0;}
        
        std::string GetHex()const;
        void SetHex(const char *psz);
        void SetHex(const std::string &str);
        std::string ToString() const;
        
        unsigned char *begin(){
            return &data[0];
        }
        unsigned char *end(){
            return &data[WIDTH];
        }
        const unsigned char *begin()const{
            return &data[0];
        }
        const unsigned char *end()const{
            return &data[WIDTH];
        }
        unsigned int size()const{
            return sizeof(data);
        }
        uint64_t GetUint64(int pos)const{
            const uint8_t *ptr = data + pos * 8;
            return ((uint64_t)ptr[0]) |\
                    ((uint64_t)ptr[1]) << 8 |\
                    ((uint64_t)ptr[2]) << 16 |\
                    ((uint64_t)ptr[3]) <<24 |\
                    ((uint64_t)ptr[4]) <<32 |\
                    ((uint64_t)ptr[5]) <<40 |\
                    ((uint64_t)ptr[6]) <<48 |\
                    ((uint64_t)ptr[7]) <<56;
        }
        template<typename Stream>
        void Serialize(Stream &s)const{
            s.write((char*)data,sizeof(data));
        }
        template<typename Stream>
        void Unserialize(Stream& s){
            s.read((char*)data,sizeof(data));
        }
};
class uint160 : public base_blob<160>{
public:
    uint160(){}
    explicit uint160(const base_blob<160>& b):base_blob<160>(b){}
    explicit uint160(const std::vector<unsigned char>& vch):base_blob<160>(vch){}
};
class uint256 : public base_blob<256>{
public:
    uint256(){}
    explicit uint256(const base_blob<256>&b):base_blob<256>(b){}
    explicit uint256(const std::vector<unsigned char>&vch):base_blob<256>(vch){}
    
    uint64_t GetCheapHash()const{
        return ReadLE64(data);
    }
};
inline uint256 uint256S(const char *str){
    uint256 rv;
    rv.SetHex(str);
    return rv;
}
inline uint256 uint256(const std::string &str){
    uint256 rv;
    rv.SetHex(str);
    return rv;
}
#endif
