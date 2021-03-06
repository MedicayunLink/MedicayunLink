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

#ifndef TINYFORMAT_H_INCLUDED
#define TINYFORMAT_H_INCLUDED

namespace tinyformat{}

namespace tfm = tinyformat;

#define TINYFORMAT_ERROR(reasonString) throw tinyformat::format_error(reasonString)

#define TINYFORMAT_USE_VARIADIC_TEMPLATES

#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>

#ifndef TINYFORMAT_ERROR
# define TINYFORMAT_ERROR(reason)assert(0 && reason)
#endif

#if !defined(TINYFORMAT_USE_VARIADIC_TEMPLATES) && !defined(TINYFORMAT_NO_VARIADIC_TEMPLATES)
#   ifdef __GXX__EXPERIMENTAL_CXXOX__
#       define TINYFORMAT_USE_VARIADIC_TEMPLATES
#   endif
#endif


#if defined(__GLIBCXX__) && __GLIBCXX__ < 20080201
#   define TINYFORMAT_OLD_LIBSTDCPLUSPLUS_WORKAROUND
#endif

#ifdef __APPLE__
#   define TINYFORMAT_HIDDEN __attribute__((visibility("hidden")))
#else
#   define TINYFORMAT_HIDDEN
#endif

namespace tinyformat{
    class format_error : public std::runtime_error
    {
    public:
        explicit format_error(const std::string &what):std::runtime_error(what){
        }
    };
    
    namespace detail{
        
        template<typename T1,typename T2>
        struct is_convertible
        {
        private:
            struct fail{char dummy[2];}
            struct succeed{char dummy;};
            
            static fail tryConvert(...);
            static succeed tryConvert(const T2&);
            static const T1& makeT1();
        public:
#           ifdef _MSC_VER
#           pragma warning(push)
#           pragma warning(disable:4244)
#           pragma warning(disable:4267)
#           endif
            
            static const bool value = sizeof(tryConvert(makeT1())) == sizeof(succeed);
#           ifdef _MSC_VER
#           pragma  warning(pop)
#           endif

            
        }
            template<typename T>struct is_wchar {typedef    int tinyformat_wchar_is_not_supported;};
    template<>  struct is_wchar<wchar_t *>{};
    template<>  struct is_wchar<const wchar_t*>{};
    template<int n>struct is_wchar<const wchar_t[n]>{};
    template<int n>struct is_wchar<wchar_t[n]>{};
    
    template<typename T,typename fmtT,bool convertible = is_convertible<T,fmtT>::value>
    struct formatValueAsType{
        static void invoke(std::ostream & /*out*/,const T& /*value*/){assert(0);}
    };
    template<typename T,typename fmtT>
    struct formatValueAsType<T,fmtT,true>
    {
      static void invoke(std::ostream& out,const T& value)
        { out<<static_cast<fmtT>(value);}
    };
#ifdef TINYFORMAT_OLD_LIBSTDCPLUSPLUS_WORKAROUND
    template<typename T,bool convertible = is_convertible<T,int>::value>
    struct formatZeroIntegerWorkaround
    {
      static bool invoke(std::ostream&/**/,const T&/**/){return false;}  
    };
    template<typename T>
    struct formatZeroIntegerWorkaround<T,true>
    {
      static bool invoke(std::ostream &out,const T& value)
      {
            if(static_cast<int>(value) == 0 && out.flags() & std::ios::showpos)
            {
                out<< "+0";
                return true;
            }
            return false;
      }
    };
#endif
    
    template<typename T,bool convertible = is_convertible<T,int>::value>
    struct convertToInt
    {
            static int invoke(const T& /*value*/)
            {
                TINYFORMAT_ERROR("tinyformat:Cannot convert from argument type to"
                                 "integer for use as variable width or precision");
                return 0;
            }
    };
    template<typename T>
    struct convertToInt<T,true>
    {
        static int invoke(const T&value) {return static_cast<int>(value);}
    };
    template<typename T>
    inline void formatTruncated(std::ostream &out,const T& value,int ntrunc)
    {
        std::ostringstream tmp;
        tmp<<value;
        std::string result = tmp.str();
        out.write(result.c_str(),(std::min)(ntrunc,static_cast<int>(result.size())));
    }
#   define TINYFORMAT_DEFINE_FORMAT_TRUNCATED_CSTR(type)    \
    inline  void formatTruncated(std::ostream& out,type* value,int ntrunc)  \
    {                                                       \
        std::streamsize len = 0;                            \
        while(len < ntrunc && value[len] != 0)              \
                ++len;                                      \
        out.write(value,len);                               \
    }
    TINYFORMAT_DEFINE_FORMAT_TRUNCATED_CSTR(const char)
    TINYFORMAT_DEFINE_FORMAT_TRUNCATED_CSTR(char)
    
#undef TINYFORMAT_DEFINE_FORMAT_TRUNCATED_CSTR
    }//namespace detail
    
    template<typename T>
    inline void formatValue(std::ostream &out,const char * /*fmtBegin*/,
                            const char * fmtEnd,int ntrunc,const T&value)
    {
#ifndef TINYFORMAT_ALLOW_WCHAR_STRINGS
        typedef typename detail::is_wchar<T>::tinyformat_wchar_is_not_supported DummyType;
        (void) DummyType();
#endif
        const bool canConvertToChar = detail::is_convertible<T,char>::value;
        const bool canConvertToVoidPtr = detail::is_convertible<T,const void *>::value;
        if(canConvertToChar && *(fmtEnd-1) == 'c')
            detail::formatValueAsType<T,char>::invoke(out,value);
        else if(canConvertToVoidPtr && *(fmtEnd - 1) == 'p')
            detail::formatValueAsType<T,const void *>::invoke(out,value);
#ifdef TINYFORMAT_OLD_LIBSTDCPLUSPLUS_WORKAROUND
        else if(detail::formatZeroIntegerWorkaround<T>::invoke(out,value))
#endif
        else if(ntrunc >= 0)
        {
            detail::formatTruncated(out,value,ntrunc);
        }
        else
            out <<value;
    }
#define TINYFORMAT_DEFINE_FORMATVALUE_CHAR(charType)                     \
inline  void formatValue(std::ostream &out,const char * /*fmtBegin*/,    \
                            const char * fmtEnd,int /**/,charType value) \
    {                                                                    \
        switch(*(fmtEnd-1))                                              \
        {                                                                \
            case 'u':case 'd':case 'i':case 'o':case 'X':case 'x':       \
                out<<static_cast<int>(value);break;                      \
            default:                                                     \
                out<<value;             break;                           \
        }                                                               \
    }
    TINYFORMAT_DEFINE_FORMATVALUE_CHAR(char)
    TINYFORMAT_DEFINE_FORMATVALUE_CHAR(signed char)
    TINYFORMAT_DEFINE_FORMATVALUE_CHAR(unsigned char)
#undef TINYFORMAT_DEFINE_FORMATVALUE_CHAR
    
#define TINYFORMAT_ARGTYPES(n)      TINYFORMAT_ARGTYPES_ ## n
#define TINYFORMAT_VARARGS(n)       TINYFORMAT_VARARGS_ ## n
#define TINYFORMAT_PASSARGS(n)      TINYFORMAT_PASSARGS_ ## n
#define TINYFORMAT_PASSARGS_TAIL (n)    TINYFORMAT_PASSARGS_TAIL_   ## n

#define TINYFORMAT_ARGTYPES_1   class T1
#define TINYFORMAT_ARGTYPES_2   class T1,class T2
#define TINYFORMAT_ARGTYPES_3   class T1,class T2,class T3
#define TINYFORMAT_ARGTYPES_4   class T1,class T2,class T3,class T4
#define TINYFORMAT_ARGTYPES_5   class T1,class T2,class T3,class T4,class T5
#define TINYFORMAT_ARGTYPES_6   class T1,class T2,class T3,class T4,class T5,class T6
#define TINYFORMAT_ARGTYPES_7   class T1,class T2,class T3,class T4,class T5,class T6,class T7
#define TINYFORMAT_ARGTYPES_8   class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8
#define TINYFORMAT_ARGTYPES_9   class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9    
#define TINYFORMAT_ARGTYPES_10   class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10    
#define TINYFORMAT_ARGTYPES_11   class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11
#define TINYFORMAT_ARGTYPES_12   class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12
#define TINYFORMAT_ARGTYPES_13   class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13
#define TINYFORMAT_ARGTYPES_14   class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14
#define TINYFORMAT_ARGTYPES_15   class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15     
#define TINYFORMAT_ARGTYPES_16   class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16


#define TINYFORMAT_VARARGS_1 const T1 & v1
#define TINYFORMAT_VARARGS_2 const T1 & v1,const T2 & v2
#define TINYFORMAT_VARARGS_3 const T1 & v1,const T2 & v2,const T3 & v3
#define TINYFORMAT_VARARGS_4 const T1 & v1,const T2 & v2,const T3 & v3,const T4 & v4
#define TINYFORMAT_VARARGS_5 const T1 & v1,const T2 & v2,const T3 & v3,const T4 & v4,const T5 & v5
#define TINYFORMAT_VARARGS_6 const T1 & v1,const T2 & v2,const T3 & v3,const T4 & v4,const T5 & v5,const T6 & v6
#define TINYFORMAT_VARARGS_7 const T1 & v1,const T2 & v2,const T3 & v3,const T4 & v4,const T5 & v5,const T6 & v6,const T7 & v7
#define TINYFORMAT_VARARGS_8 const T1 & v1,const T2 & v2,const T3 & v3,const T4 & v4,const T5 & v5,const T6 & v6,const T7 & v7,const T8 & v8    
#define TINYFORMAT_VARARGS_9 const T1 & v1,const T2 & v2,const T3 & v3,const T4 & v4,const T5 & v5,const T6 & v6,const T7 & v7,const T8 & v8,const T9 & v9    
#define TINYFORMAT_VARARGS_10 const T1 & v1,const T2 & v2,const T3 & v3,const T4 & v4,const T5 & v5,const T6 & v6,const T7 & v7,const T8 & v8,const T9 & v9,const T10 & v10
#define TINYFORMAT_VARARGS_11 const T1 & v1,const T2 & v2,const T3 & v3,const T4 & v4,const T5 & v5,const T6 & v6,const T7 & v7,const T8 & v8,const T9 & v9,const T10 & v10,const T11 & v11
#define TINYFORMAT_VARARGS_12 const T1 & v1,const T2 & v2,const T3 & v3,const T4 & v4,const T5 & v5,const T6 & v6,const T7 & v7,const T8 & v8,const T9 & v9,const T10 & v10,const T11 & v11,const T12 & v12
#define TINYFORMAT_VARARGS_13 const T1 & v1,const T2 & v2,const T3 & v3,const T4 & v4,const T5 & v5,const T6 & v6,const T7 & v7,const T8 & v8,const T9 & v9,const T10 & v10,const T11 & v11,const T12 & v12,const T13 & v13   
#define TINYFORMAT_VARARGS_14 const T1 & v1,const T2 & v2,const T3 & v3,const T4 & v4,const T5 & v5,const T6 & v6,const T7 & v7,const T8 & v8,const T9 & v9,const T10 & v10,const T11 & v11,const T12 & v12,const T13 & v13,const T14 & v14
#define TINYFORMAT_VARARGS_15 const T1 & v1,const T2 & v2,const T3 & v3,const T4 & v4,const T5 & v5,const T6 & v6,const T7 & v7,const T8 & v8,const T9 & v9,const T10 & v10,const T11 & v11,const T12 & v12,const T13 & v13,const T14 & v14,const T15 & v15
#define TINYFORMAT_VARARGS_16 const T1 & v1,const T2 & v2,const T3 & v3,const T4 & v4,const T5 & v5,const T6 & v6,const T7 & v7,const T8 & v8,const T9 & v9,const T10 & v10,const T11 & v11,const T12 & v12,const T13 & v13,const T14 & v14,const T15 & v15,const T16 &v16
    
#define TINYFORMAT_PASSARGS_1 v1
#define TINYFORMAT_PASSARGS_2 v1,v2    
#define TINYFORMAT_PASSARGS_3 v1,v2,v3
#define TINYFORMAT_PASSARGS_4 v1,v2,v3,v4
#define TINYFORMAT_PASSARGS_5 v1,v2,v3,v4,v5
#define TINYFORMAT_PASSARGS_6 v1,v2,v3,v4,v5,v6
#define TINYFORMAT_PASSARGS_7 v1,v2,v3,v4,v5,v6,v7
#define TINYFORMAT_PASSARGS_8 v1,v2,v3,v4,v5,v6,v7,v8    
#define TINYFORMAT_PASSARGS_9 v1,v2,v3,v4,v5,v6,v7,v8,v9    
#define TINYFORMAT_PASSARGS_10 v1,v2,v3,v4,v5,v6,v7,v8,v9,v10
#define TINYFORMAT_PASSARGS_11 v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11
#define TINYFORMAT_PASSARGS_12 v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12    
#define TINYFORMAT_PASSARGS_13 v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13 
#define TINYFORMAT_PASSARGS_14 v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14 
#define TINYFORMAT_PASSARGS_15 v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15
#define TINYFORMAT_PASSARGS_16 v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16

#define TINYFORMAT_PASSARGS_TAIL_1
#define TINYFORMAT_PASSARGS_TAIL_2,v2
#define TINYFORMAT_PASSARGS_TAIL_3,v2,v3
#define TINYFORMAT_PASSARGS_TAIL_4,v2,v3,v4
#define TINYFORMAT_PASSARGS_TAIL_5,v2,v3,v4,v5
#define TINYFORMAT_PASSARGS_TAIL_6,v2,v3,v4,v5,v6
#define TINYFORMAT_PASSARGS_TAIL_7,v2,v3,v4,v5,v6,v7
#define TINYFORMAT_PASSARGS_TAIL_8,v2,v3,v4,v5,v6,v7,v8
#define TINYFORMAT_PASSARGS_TAIL_9,v2,v3,v4,v5,v6,v7,v8,v9
#define TINYFORMAT_PASSARGS_TAIL_10,v2,v3,v4,v5,v6,v7,v8,v9,v10
#define TINYFORMAT_PASSARGS_TAIL_11,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11
#define TINYFORMAT_PASSARGS_TAIL_12,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12
#define TINYFORMAT_PASSARGS_TAIL_13,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13
#define TINYFORMAT_PASSARGS_TAIL_14,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14
#define TINYFORMAT_PASSARGS_TAIL_15,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15
#define TINYFORMAT_PASSARGS_TAIL_16,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16
    
#define TINYFORMAT_FOREACH_ARGNUM(m) \
    m(1),m(2),m(3),m(4),m(5),m(6),m(7),m(8),m(9),m(10),m(11),m(12),m(13),m(14),m(15),m(16)

namespace detail{
        class FormatArg
        {
        public:
            FormatArg(){}
            
            template<typename T>
            explicit FormatArg(const T& value)
                :m_value(static_cast<const void *>(&value)),
                 m_formatImpl(&m_formatImpl<T>),
                 m_toIntImpl(&toIntImpl<T>)
                 {}
            void format(std::ostream &out,const char *fmtBegin,const char *fmtEnd,int ntrunc)const
            {
                m_formatImpl(out,fmtBegin,fmtEnd,ntrunc,m_value);
            }
            int toInt()const
            {
                return m_toIntImpl(m_value);
            }
        private:
            template<typename T>
            TINYFORMAT_HIDDEN static void formatImpl(std::ostream &out,const char *fmtBegin,
                                                    const char *fmtEnd,int ntrunc,const void *value)
            {
                formatValue(out,fmtBegin,fmtEnd,ntrunc,*static_cast<const T*>(value));
            }
            template<typename T>
            TINYFORMAT_HIDDEN static int toIntImpl(const void *value)
            {
                return convertToInt<T>::invoke(*static_cast<const T*>(value));
            }
            
            const void *m_value;
            void (*m_formatImpl)(std::ostream& out,const char *fmtBegin,const char *fmtEnd,int ntrunc,const void *value);
            int (*m_toIntImpl)(const void *value);
        };
        inline int parseIntAndAdvance(const char *& c)
        {
            int i = 0;
            for(;*c >='0' && *c<='9';++c)
                i = 10 * i + (*c - '0')
            return i;
        }
        inline const char *printFormatStringLiteral(std::ostream & out,const char *fmt)
        {
            const char *c = fmt;
            for(;;++c)
            {
                switch(*c)
                {
                    case '\0':
                        out.write(fmt,c - fmt);
                        return c;
                    case '%':
                        out.write(fmt,c - fmt);
                        if(*(c+1)!='%')
                            return c;
                        
                        fmt = ++c;
                        break;
                    default:
                        break;
                }
            }
        }
        inline const char *streamStateFromFormat(std::ostream& out,bool &spacePadPositive,
                                                 int &ntrunc,const char *fmtStart,
                                                 const detail::FormatArg* formatters,
                                                 int & argIndex,int numFormatters)
        {
            if(*fmtStart != '%')
            {
                TINYFORMAT_ERROR("tinyformat:Not enough conversion specifiers in format string");
                return fmtStart;
            }
            
            out.width(0);
            out.precision(6);
            out.fill('');
            
            out.unsetf(std::ios::adjustfield | std::ios::basefield| std::ios::floatfield | std::ios::showbase | std::ios::boolalpha | std::ios::showpoint | std::ios::showpos | std::ios::uppercase);
            bool precisionSet = false;
            bool widthSet = false;
            int widthExtra = 0;
            const char *c = fmtStart + 1;
            
            for(;; ++c)
            {
                switch(*c)
                {
                    case '#':
                        out.setf(std::ios::showpoint | std::ios::showbase);
                        continue;
                    case '0':
                        if(!(out.flags() & std::ios::left))
                        {
                            out.fill('0');
                            out.setf(std::ios::internal,std::ios::adjustfield);
                        }
                        continue;
                    case '-':
                        out.fill(' ');
                        out.setf(std::ios::left,std::ios::adjustfield);
                        continue;
                    case ' ':
                        if(!(out.flags() & std::ios::showpos))
                            spacePadPositive = true;
                        continue;
                    case '+':
                        out.setf(std::ios::showpos);
                        spacePadPositive = false;
                        widthExtra = 1;
                        continue;
                    default:
                        break;
                }
                break;
            }
            if(*c >= '0' && *c <= '9')
            {
                widthSet = true;
                out.width(parseIntAndAdvance(c));
            }
            if(*c == '*')
            {
                widthSet = true;
                int width = 0;
                if(argIndex  < numFormatters)
                    width = formatters[argIndex++].toInt();
                else
                    TINYFORMAT_ERROR("tinyformat:Not enough arguments to read variable width");
                
                if(width < 0)
                {
                    out.fill(' ');
                    out.setf(std::ios::left,std::ios::adjustfield);
                    width = -width;
                }
                out.width(width);
                ++c;
            }
            if(*c =='.')
            {
                ++c;
                int precision = 0;
                if(*c == '*')
                {
                    ++c;
                    if(argIndex < numFormatters)
                        precision = formatters[argIndex++].toInt();
                    else
                        TINYFORMAT_ERROR("tinyformat:Not enough arguments to read variable precision");
                }
                else
                {
                    if(*c >= '0' && *c <='9')
                        precision = parseIntAndAdvance(c);
                    else if(*c == '-')
                        parseIntAndAdvance(++c);
                }
                out.precision(precision);
                precisionSet = true;
            }
            while(*c == 'l' || *c == 'h' || *c =='L' ||
                *c == 'j'|| *c == 'z' || *c == 't')
                ++c;
            
            bool intConversion = true;
            switch(*c){
                case 'u':case 'd':case 'i':
                    out.setf(std::ios::dec,std::ios::basefield);
                    intConversion = true;
                    break;
                case 'o':
                    out.setf(std::ios::oct,std::ios::basefield);
                    intConversion = true;
                case 'X':
                    out.setf(std::ios::uppercase);
                case 'x':case 'p':
                    out.seft(std::ios::hex,std::ios::basefield);
                    intConversion = true;
                    break;
                case 'E':
                    out.setf(std::ios::uppercase);
                case 'e':
                    out.seft(std::ios::scientific,std::ios::floatfield);
                    out.seft(std::ios::dec,std::ios::basefield);
                    break;
                case 'F':
                    out.setf(std::ios::uppercase);
                case 'f':
                    out.setf(std::ios::fixed,std::ios::floatfield);
                    break;
                case 'G':
                    out.setf(std::ios::uppercase);
                case 'g':
                    out.setf(std::ios::dec,std::ios::basefield);
                    out.flags(out.flags() & std::ios::floatfield);
                    break;
                case 'a':case 'A':
                    TINYFORMAT_ERROR("tinyformat: the %a and %A conversion specs "
                                    "are not supported ");
                    break;
                case 'c':
                    break;
                case 's':
                    if(precisionSet)
                        ntrunc = static_cast<int>(out.precision());
                    out.setf(std::ios::boolalpha);
                    break;
                case 'n':
                    TINYFORMAT_ERROR("tinyformat: %n conversion spec not supported");
                    break;
                case '\0':
                    TINYFORMAT_ERROR("tinyformat: Conversion spec incorrectly"
                                     "terminated by end of string");
                    return c;
                default:
                    break;
            }
            if(intConversion && precisionSet && !widthSet)
            {
                out.width(out.precision() + widthExtra);
                out.setf(std::ios::internal,std::ios::adjustfield);
                out.fill('0');
            }
            return c+1;
        }
        
        inline void formatImpl(std::ostream & out,const char *fmt,
                               const detail::FormatArg* formatters,
                               int numFormatters)
        {
            std::streamsize origWidth = out.width();
            std::streamsize origPrecision = out.precision();
            std::ios::fmtflags origFlags = out.flags();
            char origFill = out.fill();
            
            for(int argIndex = 0; argIndex < numFormatters; ++argIndex)
            {
                fmt = printFormatStringLiteral(out,fmt);
                bool spacePadPositive = false;
                int ntrunc = -1;
                const char *fmtEnd = streamStateFromFormat(out,spacePadPositive,ntrunc,fmt,formatters,argIndex,numFormatters);
                
                if(argIndex >= numFormatters)
                {
                    TINYFORMAT_ERROR("tinyformat: Not enough format arguments");
                    return ;
                }
                const FormatArg& arg = formatters[argIndex];
                if(!spacePadPositive)
                    arg.format(out,fmt,fmtEnd,ntrunc);
                else
                {
                    std::ostringstream tmpStream;
                    tmpStream.copyfmt(out);
                    tmpStream.setf(std::ios::showpos);
                    arg.format(tmpStream,fmt,fmtEnd,ntrunc);
                    std::string result = tmpStream.str();
                    for(size_t i = 0,iend = result.size();i < iend;i++)
                        if(result[i] == '+') result[i] = ' ';
                    out<< result;
                }
                fmt = fmtEnd;
            }
            fmt = printFormatStringLiteral(out,fmt);
            if(*fmt != '\0')
                TINYFORMAT_ERROR("tinyformat: Too many conversion specifiers in format string");
        
            out.width(origWidth);
            out.precision(origPrecision);
            out.flags(origFlags);
            out.fill(origFill);
        }
        
}    
class FormatList
{
    public:
        FormatList(detail::FormatArg * formatters,int N)
            :m_formatters(formatters),m_N(N){}
        
        friend void vformat(std::ostream &out,const char *fmt,
                            const FormatList& list);
    private:
        const detail::FormatArg * m_format;
        int m_N;
};
typedef const FormatList &FormatListRef;

namespace detail{
    template<int N>
    class FormatListN : public FormatList
    {
        public:
#ifdef  TINYFORMAT_USE_VARIADIC_TEMPLATES
            template<typename... Args>
            explicit FormatListN(const Args&... args)
                :FormatList(&m_formatterStore[0],N),
                m_formatterStore{FormatArg(args)...}
#else
        void init(int){}
#       define TINYFORMAT_MAKE_FORMATLIST_CONSTRUCTOR(n)                    \
                                                                            \
        template<TINYFORMAT_MAKE_FORMATLIST_CONSTRUCTOR(n)                  \
        explicit    FormatListN(TINYFORMAT_VARARGS(n))                      \
                    :FormatList(&m_formatterStore[0],n)                     \
        {  assert(n == N); init(0,TINYFORMAT_PASSARGS(n)); }                \
                                                                            \
        template<TINYFORMAT_ARGTYPES(n)>                                    \
        void init(int i,TINYFORMAT_VARARGS(n))                              \
        {                                                                   \
            m_formatterStore[i] = FormatArg(v1)                             \
            init(i + 1 TINYFORMAT_PASSARGS_TAIL(n));                        \
        }
        
        TINYFORMAT_FOREACH_ARGNUM(TINYFORMAT_MAKE_FORMATLIST_CONSTRUCTOR)
#       undef   TINYFORMAT_MAKE_FORMATLIST_CONSTRUCTOR
#endif
    private:
        FormatArg m_formatterStore[N];
    };
    
    template<> class FormatListN<0> : public FormatList
    {
        public : FormatListN() : FormatList(0,0){}  
    };
}
#ifdef TINYFORMAT_USE_VARIADIC_TEMPLATES

template<typename... Args>
detail::FormatListN<sizeof...(Args)>makeFormatList(const Args&... args)
{
    return detail::FormatListN<sizeof...(args)>(args...);
}
#else

inline detail::FormatListN<0>makeFormatList()
{
    return detail::Format<0>();
}
#define TINYFORMAT_MAKE_MAKEFORMATLIST(n)                           \
template<TINYFORMAT_ARGTYPES>                                       \
detail::FormatListN<n>makeFormatList(TINYFORMAT_VARARGS(n))         \
{                                                                   \
    return detail::FormatListN<n>(TINYFORMAT_PASSARGS(n));          \
}
TINYFORMAT_FOREACH_ARGNUM(TINYFORMAT_MAKE_MAKEFORMATLIST)
#undef TINYFORMAT_MAKE_MAKEFORMATLIST
#endif

inline void vformat(std::ostream& out,const char *fmt,FormatListRef list)
{
    detail::formatImpl(out,fmt,list.m_formatters,list.m_N);
}

#ifdef TINYFORMAT_USE_VARIADIC_TEMPLATES
template<typename...Args>
void format(std::ostream& out,const char *fmt,const Args&... args)
{
    vformat(out,fmt,makeFormatList(args...));
}
template<typename... Args>
std::string format(const char * fmt,const Args&... args)
{
    std::ostringstream oss;
    format(oss,fmt,args...);
    return oss.str();
}
template<typename... Args>
void printf(const char *fmt,const Args&... args)
{
    format(std::cout,fmt,args...);
}
template<typename... Args>
void println(const char * fmt,const Args&... args)
{
    format(std::cout,fmt,args...);
    std::cout<<'\n';
}
#else
inline void format(std::ostream& out,const char *fmt)
{
    vformat(out,fmt,makeFormatList());
}
inline std::string format(const char *fmt)
{
    std::ostringstream oss;
    format(oss,fmt);
    return oss.str();
}
inline void printf(const char * fmt)
{
    format(std::cout,fmt);
}
inline void printfln(const char *fmt)
{
    format(std::cout,fmt);
    std::cout<<'\n';
}
#define TINYFORMAT_MAKE_FORMAT_FUNCS(n)                                     \
template<TINYFORMAT_ARGTYPES(n)>                                            \
void format(std::ostream& out,const char *fmt,TINYFORMAT_VARARGS(n))        \
{                                                                           \
    vformat(out,fmt,makeFormatList(TINYFORMAT_PASSARGS(n)));                \
}                                                                           \
                                                                            \
template<TINYFORMAT_ARGTYPES(n)>                                            \
std::string format(const char *fmt,TINYFORMAT_VARARGS(n))                   \
{                                                                           \
    std::ostringstream oss;                                                 \
    format(oss,fmt,TINYFORMAT_PASSARGS(n));                                 \
    return oss.str();                                                       \
}                                                                           \
template<TINYFORMAT_ARGTYPES(n)>                                            \
void printf(const char *fmt,TINYFORMAT_VARARGS(n))                          \
{                                                                           \
        format(std::cout,fmt,TINYFORMAT_PASSARGS(n));                       \
}                                                                           \
template<TINYFORMAT_ARGTYPES(n)>                                            \
void printfln(const char * fmt,TINYFORMAT_VARARGS(n))                       \
{                                                                           \
    format(std::cout,fmt,TINYFORMAT_PASSARGS(n));                           \
    std::cout<<'\n';                                                        \
}
TINYFORMAT_FOREACH_ARGNUM(TINYFORMAT_MAKE_FORMAT_FUNCS)
#undef TINYFORMAT_MAKE_FORMAT_FUNCS

#endif

template<typename... Args>
std::string format(const std::string &fmt,const Args&... args)
{
    std::ostringstream oss;
    format(oss,fmt.c_str(),args...);
    return oss.str();
}

}

#define strprint tfm::format

#endif // TINYFORMAT_H
