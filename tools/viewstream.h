/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <iostream>
#include <string_view>
#include <cstring>

template<typename CharType, class TraitsType >
class view_streambuf final: public std::basic_streambuf<CharType, TraitsType > {
private:
    typedef std::basic_streambuf<CharType, TraitsType > super_type;
    typedef view_streambuf<CharType, TraitsType> self_type;
public:

    /**
    *  These are standard types.  They permit a standardized way of
    *  referring to names of (or names dependent on) the template
    *  parameters, which are specific to the implementation.
    */
    typedef typename super_type::char_type char_type;
    typedef typename super_type::traits_type traits_type;
    typedef typename traits_type::int_type int_type;
    typedef typename traits_type::pos_type pos_type;
    typedef typename traits_type::off_type off_type;

    typedef typename std::basic_string_view<char_type, traits_type> source_view;

    explicit view_streambuf(const source_view& src) noexcept:
        super_type(),
        src_( src )
    {
        auto *buff = const_cast<char_type*>( src_.data() );
        this->setg( buff , buff, buff + src_.length() );
    }

    std::streamsize xsgetn(char_type* _s, std::streamsize _n) override
    {
        if(0 == _n)
            return 0;
        if( (this->gptr() + _n) >= this->egptr() ) {
            _n =  this->egptr() - this->gptr();
            if(0 == _n && !traits_type::not_eof(this->underflow() ) )
                return -1;
        }
        std::memmove(static_cast<void*>(_s), this->gptr(), size_t(_n));
        this->gbump( static_cast<int>(_n) );
        return _n;
    }

    int_type pbackfail(int_type _c) override
    {
        char_type *pos = this->gptr() - 1;
        *pos = traits_type::to_char_type(_c );
        this->pbump(-1);
        return 1;
    }

    int_type underflow() override
    {
        return traits_type::eof();
    }

    std::streamsize showmanyc() override
    {
        return static_cast<std::streamsize>( this->egptr() - this->gptr() );
    }

    ~view_streambuf() override = default;
private:
    const source_view& src_;
};

template<typename _char_type>
class view_istream final:public std::basic_istream<_char_type, std::char_traits<_char_type> > {
public:
    view_istream(const view_istream&) = delete;
    view_istream& operator=(const view_istream&) = delete;
private:
    typedef std::basic_istream<_char_type, std::char_traits<_char_type> > super_type;
    typedef view_streambuf<_char_type, std::char_traits<_char_type> > streambuf_type;
public:
    typedef _char_type  char_type;
    typedef typename super_type::int_type int_type;
    typedef typename super_type::pos_type pos_type;
    typedef typename super_type::off_type off_type;
    typedef typename super_type::traits_type traits_type;
    typedef typename streambuf_type::source_view source_view;

    explicit view_istream(const source_view& src):
        super_type( nullptr ),
        sb_(nullptr)
    {
        sb_ = new streambuf_type(src);
        this->init( sb_ );
    }


    view_istream(view_istream&& other) noexcept:
        super_type( std::forward<view_istream>(other) ),
        sb_( std::move( other.sb_ ) )
    {}

    view_istream& operator=(view_istream&& rhs) noexcept
    {
        view_istream( std::forward<view_istream>(rhs) ).swap( *this );
        return *this;
    }

    ~view_istream() override {
        delete sb_;
    }

private:
    streambuf_type *sb_;
};
