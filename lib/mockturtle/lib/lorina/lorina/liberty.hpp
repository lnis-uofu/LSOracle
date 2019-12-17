/* lorina: C++ parsing library
 * Copyright (C) 2018  EPFL
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

/*!
  \file liberty.hpp
  \brief Implements liberty parser
  \author Heinz Riener
*/

#pragma once

#include <lorina/common.hpp>
#include <lorina/diagnostics.hpp>
#include <lorina/detail/utils.hpp>
#include <fstream>
#include <iostream>
#include <regex>

namespace lorina
{

/*! \brief A reader visitor for the LIBERTY format.
 *
 * Callbacks for the LIBERTY format.
 */
    class liberty_reader
    {
    public:
    }; // liberty_reader

/*! \brief A LIBERTY reader for pretty-printing.
 *
 * Callbacks for pretty-printing LIBERTY.
 */
    class liberty_pretty_printer : public liberty_reader
    {
    public:
    }; // liberty_pretty_printer

    namespace detail
    {

        enum class token_kind
        {
            eof = 0,
            id = 1,
            lparan = 2, // (
            rparan = 3, // )
            lscope = 4, // {
            rscope = 5, // }
            semicolon = 6, // ;
            colon = 7, // :
            comma = 8, // ,
            lexer_error = 9
        };

        token_kind char_to_token_kind( char const c )
        {
            switch ( c )
            {
                case '(':
                    return token_kind::lparan;
                case ')':
                    return token_kind::rparan;
                case '{':
                    return token_kind::lscope;
                case '}':
                    return token_kind::rscope;
                case ':':
                    return token_kind::colon;
                case ';':
                    return token_kind::semicolon;
                case ',':
                    return token_kind::comma;
                default:
                    return token_kind::lexer_error;
            }
        }

        std::string token_kind_to_string( token_kind const& kind )
        {
            switch ( kind )
            {
                case token_kind::eof:
                    return "eof";
                case token_kind::id:
                    return "id";
                case token_kind::lparan:
                    return "(";
                case token_kind::rparan:
                    return ")";
                case token_kind::lscope:
                    return "{";
                case token_kind::rscope:
                    return "}";
                case token_kind::semicolon:
                    return ";";
                case token_kind::colon:
                    return ":";
                case token_kind::comma:
                    return ",";
                case token_kind::lexer_error:
                    return "error";
                default:
                    return "unknown";
            }
        }

        struct token
        {
            std::string lexem;
            token_kind kind;
        }; /* token */

        class liberty_lexer
        {
        public:
            explicit liberty_lexer( std::istream& in )
                    : _in( in )
            {}

            token operator()()
            {
                uint8_t comment_mode = 0u;
                bool quote_mode = false;
                std::string lexem;

                bool okay;
                char c;
                while ( ( okay = bool( _in.get( c ) ) ) )
                {
                    /* begin parse comments */
                    if ( ( c == '/' && comment_mode == 0 ) ||
                         ( c == '*' && comment_mode == 1 ) )
                    {
                        ++comment_mode;
                        lexem.push_back( c );
                        continue;
                    }
                    else if ( comment_mode == 1 )
                    {
                        comment_mode = 0;
                    }

                    if ( c == '*' && comment_mode == 2 )
                    {
                        ++comment_mode;
                        lexem.push_back( c );
                        continue;
                    }
                    else if ( c == '/' && comment_mode == 3 )
                    {
                        lexem.push_back( c );
                        // std::cout << lexem << std::endl;
                        comment_mode = 0;
                        lexem = "";
                        continue;
                    }
                    else if ( comment_mode >= 2 )
                    {
                        comment_mode = 2;
                        lexem.push_back( c );
                        continue;
                    }
                    /* end parse comments */

                    if ( quote_mode )
                    {
                        if ( c == '\"' )
                            quote_mode = false;
                        lexem.push_back( c );
                        continue;
                    }

                    assert( !quote_mode );

                    if ( c == '\"' )
                    {
                        quote_mode = true;
                        lexem.push_back( c );
                        continue;
                    }

                    if ( !lexem.empty() && is_whitespace( c ) )
                        break;

                    if ( is_separator( c ) )
                    {
                        if ( !lexem.empty() )
                        {
                            _in.putback( c );
                            break;
                        }

                        return { std::string{c}, char_to_token_kind( c ) };
                    }

                    if ( !is_whitespace( c ) )
                        lexem.push_back( c );
                }

                return lexem.empty() ? token{"", token_kind::eof} : token{lexem, token_kind::id};
            }

            bool is_whitespace( char c ) const
            {
                return ( c == ' ' || c == '\n' || c == '\\' );
            }

            bool is_separator( char c ) const
            {
                return ( c == '(' || c == ')' || c == '{' || c == '}' || c == ';' || c == ':' || c == ',' );
            }

        protected:
            std::istream& _in;
        }; // liberty_lexer

        class liberty_parser
        {
        public:
            explicit liberty_parser( std::istream& in, const liberty_reader& reader, diagnostic_engine *diag )
                    : _lexer( in )
                    , _reader( reader )
                    , _diag( diag )
            {}

            bool debug()
            {
                do
                {
                    _tok = _lexer();

                    std::cout << fmt::format( "kind:{:10} lexem:{}\n",
                                              token_kind_to_string( _tok.kind ),
                                              _tok.lexem );
                } while ( _tok.kind != token_kind::eof );

                /* report success if no errors occurred and the current token
                   indicates the end of the file */
                return ( num_errors == 0 && _tok.kind == token_kind::eof );
            }

            bool run()
            {
                /* report success if no errors occurred and the current token
                   indicates the end of the file */
                return ( parse_library_def() &&
                         num_errors == 0 &&
                         _tok.kind == token_kind::eof );
            }

            bool parse_library_def()
            {
                /* get token */
                consume();

                if ( !is( token_kind::id, "library" ) )
                    return false;
                consume();

                if ( !is( token_kind::lparan ) )
                    return false;
                consume();

                if ( !is( token_kind::id ) )
                    return false;
                std::string id = _tok.lexem;
                consume();

                if ( !is( token_kind::rparan ) )
                    return false;
                consume();

                std::cout << fmt::format( "library ( {} )\n", id );

                if ( !is( token_kind::lscope ) )
                    return false;
                consume();

                std::cout << "{" << std::endl;

                parse_defs();

                if ( !is( token_kind::rscope ) )
                    return false;
                consume();

                std::cout << "}" << std::endl;

                if ( !is( token_kind::eof ) )
                    return false;

                return true;
            }

            bool parse_defs()
            {
                bool okay;
                while ( !is( token_kind::rscope ) )
                {
                    okay = parse_def();
                    if ( !okay )
                        return false;
                }
                return true;
            }

            bool parse_def()
            {
                std::string key;
                if ( is( token_kind::id ) )
                {
                    key = _tok.lexem;
                    consume();
                }

                if ( is( token_kind::colon ) )
                {
                    std::string value;
                    consume();

                    if ( !is( token_kind::id ) )
                        return false;
                    value = _tok.lexem;
                    consume();

                    if ( !is( token_kind::semicolon ) )
                        return false;
                    consume();

                    std::cout << fmt::format( "{} : {};\n", key, value );
                    return true;
                }
                else if ( is( token_kind::lparan ) )
                {
                    std::vector<std::string> values;
                    consume();

                    while ( !is( token_kind::rparan ) )
                    {
                        if ( is( token_kind::id ) )
                        {
                            values.emplace_back( _tok.lexem );
                            consume();

                            if ( is( token_kind::comma ) )
                                consume();
                        }
                    }

                    assert( is( token_kind::rparan ) );
                    consume();

                    if ( is( token_kind::semicolon ) )
                    {
                        consume();
                        std::string out;
                        for ( auto i = 0u; i < values.size(); ++i )
                        {
                            out += values.at( i );
                            if ( i+1 < values.size() )
                                out += ", ";
                        }
                        std::cout << fmt::format( "{}( {} );\n", key, out );
                        return true;
                    }
                    else if ( is( token_kind::lscope ) )
                    {
                        consume();

                        assert( values.size() <= 1u );
                        std::cout << fmt::format( "{}( {} )\n", key, values.size() == 0u ? "" : values.at( 0u ) );
                        std::cout << "{" << std::endl;

                        bool okay = parse_defs();
                        if ( !okay )
                            return false;

                        if ( !is( token_kind::rscope ) )
                            return false;
                        consume();

                        std::cout << "}" << std::endl;
                        return true;
                    }
                }
                return false;
            }

        protected:
            void consume()
            {
                _tok = _lexer();
            }

            bool is( token_kind const& kind ) const
            {
                return ( _tok.kind == kind );
            }

            bool is( token_kind const& kind, std::string const& lexem ) const
            {
                return ( _tok.kind == kind && _tok.lexem == lexem );
            }

        protected:
            liberty_lexer _lexer;
            const liberty_reader& _reader;
            diagnostic_engine *_diag;

            token _tok;
            uint32_t num_errors{0};
        }; // liberty_parser

    } /* detail */

/*! \brief Reader function for LIBERTY format.
 *
 * Reads LIBERTY format from a stream and invokes a callback
 * method for each parsed primitive and each detected parse error.
 *
 * \param in Input stream
 * \param reader A LIBERTY reader with callback methods invoked for parsed primitives
 * \param diag An optional diagnostic engine with callback methods for parse errors
 * \return Success if parsing have been successful, or parse error if parsing have failed
 */
    inline return_code read_liberty( std::istream& in, const liberty_reader& reader, diagnostic_engine* diag = nullptr )
    {
        detail::liberty_parser p( in, reader, diag );
        if ( p.run() )
            return return_code::success;

        return return_code::parse_error;
    }

/*! \brief Reader function for LIBERTY format.
 *
 * Reads LIBERTY format from a file and invokes a callback method for each
 * parsed primitive and each detected parse error.
 *
 * \param filename Name of the file
 * \param reader A LIBERTY reader with callback methods invoked for parsed primitives
 * \param diag An optional diagnostic engine with callback methods for parse errors
 * \return Success if parsing have been successful, or parse error if parsing have failed
 */
    inline return_code read_liberty( const std::string& filename, const liberty_reader& reader, diagnostic_engine* diag = nullptr )
    {
        std::ifstream in( detail::word_exp_filename( filename ), std::ifstream::in );
        return read_liberty( in, reader, diag );
    }

} // namespace lorina