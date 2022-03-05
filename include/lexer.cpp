

#include <vector>
#include <string>
#include <iostream>

#include "commons.h"
using namespace commons;

#include "lexer.h"

#ifndef out
#define out(in) std::cout << (in) << std::endl;
#endif // !out

std::vector<token::instance>& lexer::tokens() {
    return buffer;
}

inline token::instance lexer::creator(token::type type) {
    return token::instance{ type, lineno, offset, position, collector };
}

inline void lexer::destroy() {
    offset += (uint32_t)collector.size();
    collector.clear();
}

#define cell source[position]
#define peek source[position + 1]
#define roll source[position++]
#define next source[++position]
#define jump(offset) source[position + offset]
#define generate(kind) \
    buffer.push_back(creator(kind)); \
    destroy(); \
    continue;
#define rollerate() \
    destroy(); \
    continue;
lexer::lexer(std::string& source)
{
    lineno = 1;
    offset = 0;
    collector = "";
    length = (int)source.size();

    // canche
    char clam;

    for (position = 0; position < length; ++position)
    {
        current = source[position];

        if (is_filter(current))
        {
            if (current == ' ' || current == '\t') ++offset; // record space of offset
            else if (current == '\n')
            {
                ++lineno;
                offset = 0;
            }
            continue;
        }

        else if (is_letter(current))
        {
            collector += current; // current must be a letter
            while (is_letter(clam = next) || ('0' <= clam && clam <= '9')) // read the next, stop in !letter
                collector += clam;
            // then, should back the last position
            --position;

            if (is_lower(current))
            {
                if (collector == "undefined") buffer.push_back(creator(token::UNDEFINED));
                else if (collector == "null") buffer.push_back(creator(token::NULL_T));
                else if (collector == "true") buffer.push_back(creator(token::TRUE));
                else if (collector == "false") buffer.push_back(creator(token::FALSE));
                else if (collector == "var") buffer.push_back(creator(token::VAR));
                else if (collector == "const") buffer.push_back(creator(token::CONST));
                else if (collector == "func") buffer.push_back(creator(token::FUNC));
                else if (collector == "return") buffer.push_back(creator(token::RETURN));
                else if (collector == "if") buffer.push_back(creator(token::IF));
                else if (collector == "else") buffer.push_back(creator(token::ELSE));
                else if (collector == "while") buffer.push_back(creator(token::WHILE));
                else if (collector == "for") buffer.push_back(creator(token::FOR));
                else if (collector == "break") buffer.push_back(creator(token::BREAK));
                else if (collector == "continue") buffer.push_back(creator(token::CONTINUE));
                else if (collector == "void") buffer.push_back(creator(token::VOID));
                else if (collector == "let") buffer.push_back(creator(token::LET));
                else if (collector == "this") buffer.push_back(creator(token::THIS));

                else if (collector == "register") buffer.push_back(creator(token::RESERVED));
                else if (collector == "volatile") buffer.push_back(creator(token::RESERVED));
                else if (collector == "interface") buffer.push_back(creator(token::RESERVED));
                else if (collector == "synchronized") buffer.push_back(creator(token::RESERVED));
                else if (collector == "yield") buffer.push_back(creator(token::RESERVED));
                else if (collector == "namespace") buffer.push_back(creator(token::RESERVED));
                else if (collector == "typedef") buffer.push_back(creator(token::RESERVED));
                else if (collector == "marco") buffer.push_back(creator(token::RESERVED));
                
                else if (collector == "as") buffer.push_back(creator(token::RESERVED));
                else if (collector == "async") buffer.push_back(creator(token::RESERVED));
                else if (collector == "await") buffer.push_back(creator(token::RESERVED));
                else if (collector == "crate") buffer.push_back(creator(token::RESERVED));
                else if (collector == "enum") buffer.push_back(creator(token::RESERVED));
                else if (collector == "implement") buffer.push_back(creator(token::RESERVED));
                else if (collector == "match") buffer.push_back(creator(token::RESERVED));
                else if (collector == "self") buffer.push_back(creator(token::RESERVED));
                else if (collector == "struct") buffer.push_back(creator(token::RESERVED));
                else if (collector == "static") buffer.push_back(creator(token::RESERVED));
                else if (collector == "trait") buffer.push_back(creator(token::RESERVED));
                else if (collector == "where") buffer.push_back(creator(token::RESERVED));
                else if (collector == "public") buffer.push_back(creator(token::RESERVED));
                else if (collector == "private") buffer.push_back(creator(token::RESERVED));
                else if (collector == "protected") buffer.push_back(creator(token::RESERVED));
                
                else buffer.push_back(creator(token::IDENTIFIER));
                destroy();
                continue;
            }

            // out current 'jump(0)', the last char of the identifter 

            // default not a customed class
            buffer.push_back(creator(token::IDENTIFIER));
            destroy();
            continue;
        }

        else if (is_digit(current)) {
            collector += current;
            clam = peek; // should 0~9, x, .
            if (clam == ';') {
                buffer.push_back(creator(token::NUMBER));
                destroy();
                continue;
            }

            bool is_hex_express = clam == 'x';
            bool is_dec_express = clam == '.'; // if true, x.xxx, must be dec
            int point_counter = 0;

            if (is_hex_express || is_dec_express) {
                collector += clam;
                ++position;
            }

            if (is_hex_express) {
                while (is_digit16(clam = next)) {
                    collector += clam;
                }
            }
            else {
                while (is_digit(clam = next) || clam == '.') {
                    if (clam == '.') point_counter++;
                    if (point_counter > 1) {
                        collector += clam;
                        out("**** unexcept number: " + collector); // **** should never be executed
                        break;
                    }
                    collector += clam;
                }
            }

            if (is_dec_express && ((clam = cell) == 'f' || clam == 'd')) {
                collector += clam;
                buffer.push_back(creator(token::NUMBER));
                destroy();
                continue;
            }

            // focus on, position roll to next
            --position;

            buffer.push_back(creator(token::NUMBER));
            destroy();
            continue;
        }

        else if ('"' == current)
        {
            while (true)
            {
                clam = next;
                if ('\\' == clam) {
                    ++position;
                    switch (cell)
                    {
                    case '0': collector.push_back('\0'); break;
                    case 'a': collector.push_back('\a'); break;
                    case 'b': collector.push_back('\b'); break;
                    case 'f': collector.push_back('\f'); break;
                    case 'n': collector.push_back('\n'); break;
                    case 'r': collector.push_back('\r'); break;
                    case 't': collector.push_back('\t'); break;
                        // case 'u': collector.push_back('\u'); break;
                    case '"': collector.push_back('"'); break;
                    case '\\': collector.push_back('\\'); break;
                    default:
                        out("unsupport escape: " + cell);
                        break;
                    }
                    // out("coll: " + collector + " peek: " + peek);
                    continue;
                }
                else if ('"' == clam)
                    break;
                collector += clam;
            }

            //  cell: "
            generate(token::STRING);
        }

        else if (is_symbol(current))
        {
            collector += current;
            if (',' == current) { generate(token::COMMA); }
            else if ('.' == current) { generate(token::DOT); }
            else if (':' == current) { generate(token::COLON); }
            else if (';' == current) { generate(token::SEMI); }
            else if ('(' == current) { generate(token::LP); }
            else if (')' == current) { generate(token::RP); }
            else if ('[' == current) { generate(token::LB); }
            else if (']' == current) { generate(token::RB); }
            else if ('{' == current) { generate(token::LC); }
            else if ('}' == current) { generate(token::RC); }
            else if ('?' == current) { generate(token::HOOK); }

            else if ('^' == current) {
                if ('=' == peek) {
                    ++position;
                    collector += '=';
                    generate(token::ASSIGN_BITXOR);
                }
                else { generate(token::BITXOR); }
            }
            else if ('~' == current) {
                generate(token::BITNOT);
            }
            else if ('&' == current) {
                if ('&' == peek) {
                    ++position;
                    collector += '&';
                    generate(token::AND);
                }
                else if ('=' == peek) {
                    ++position;
                    collector += '=';
                    generate(token::ASSIGN_BITAND);
                }
                else { generate(token::BITAND); }
            }
            else if ('|' == current) {
                if ('|' == peek) {
                    ++position;
                    collector += '|';
                    generate(token::OR);
                }
                else if ('=' == peek) {
                    ++position;
                    collector += '=';
                    generate(token::ASSIGN_BITOR);
                }
                else { generate(token::BITOR); }
            }
            else if ('=' == current) {
                if ('=' == peek) {
                    ++position;
                    collector += '=';
                    generate(token::EQ);
                }
                else { generate(token::ASSIGN); }
            }
            else if ('!' == current) {
                if ('=' == peek) {
                    ++position;
                    collector += '=';
                    generate(token::NE);
                }
                else { generate(token::NOT); }
            }
            else if ('<' == current) {
                if ('=' == peek) {
                    ++position;
                    collector += '=';
                    generate(token::LE);
                }
                else if ('<' == peek) {
                    if ('=' == jump(2)) {
                        position += 2;
                        collector += '<';
                        collector += '=';
                        generate(token::ASSIGN_LSH);
                    }
                    else {
                        ++position;
                        collector += '<';
                        generate(token::LSH);
                    }
                }
                else { generate(token::LT); }
            }
            else if ('>' == current) {
                if ('=' == peek) {
                    ++position;
                    collector += '=';
                    generate(token::GE);
                }
                else if ('>' == peek) {
                    if ('=' == jump(2)) {
                        position += 2;
                        collector += '>';
                        collector += '=';
                        generate(token::ASSIGN_RSH);
                    }
                    else {
                        ++position;
                        collector += '>';
                        generate(token::RSH);
                    }
                }
                else { generate(token::GT); }
            }
            else if ('+' == current) {
                if ('=' == peek) {
                    ++position;
                    collector += '=';
                    generate(token::ASSIGN_ADD);
                }
                else if ('+' == peek) {
                    ++position;
                    collector += '+';
                    generate(token::INC);
                }
                else { generate(token::ADD); }
            }
            else if ('-' == current) {
                if ('=' == peek) {
                    ++position;
                    collector += '=';
                    generate(token::ASSIGN_SUB);
                }
                else if ('-' == peek) {
                    ++position;
                    collector += '-';
                    generate(token::DEC);
                }
                else { generate(token::SUB); }
            }
            else if ('*' == current) {
                if ('=' == peek) {
                    ++position;
                    collector += '=';
                    generate(token::ASSIGN_MUL);
                }
                else if ('*' == peek) {
                    if ('=' == jump(2)) {
                        position += 2;
                        collector += '*';
                        collector += '=';
                        generate(token::ASSIGN_POW);
                    }
                    else {
                        ++position;
                        collector += '*';
                        generate(token::POW);
                    }
                }
                else { generate(token::MUL); }
            }
            else if ('/' == current) {
                if ('=' == peek) {
                    ++position;
                    collector += '=';
                    generate(token::ASSIGN_DIV);
                }
                // line comment
                else if ('/' == peek) {
                    position += 2; // skip '//'
                    collector.clear();
                    while (!is_ostter(clam = roll))
                        collector += clam;
                    --position;
                    // out("# line comment: " + collector)
                    // generate(token::COMMENT_LINE);
                    ++lineno;
                    rollerate();
                }
                // block comment
                else if ('*' == peek) {
                    position += 2; // skip '//'
                    collector.clear();
                    while (!('*' == (clam = roll) && '/' == cell)) {
                        if ('\n' == clam) {
                            ++lineno;
                            offset = 0;
                        }
                        collector += clam;
                    }
                    ++position; // skip '/'
                    // just for check the last '\n'
                    if ('\n' == clam) {
                        ++lineno;
                        offset = 0;
                    }

                    // out("# block comment: " + collector)
                    // generate(token::COMMENT_BLOCK);
                    ++lineno;
                    rollerate();
                }
                else { generate(token::DIV); }
            }
            else if ('%' == current) {
                if ('=' == peek) {
                    ++position;
                    collector += '=';
                    generate(token::ASSIGN_MOD);
                }
                else { generate(token::MOD); }
            }

            out("unknowed symbol: " + collector);
            destroy();
            continue;
        }

        ++offset;
    }
};
#undef generate
#undef cell
#undef peek
#undef roll
#undef next
#undef jump
