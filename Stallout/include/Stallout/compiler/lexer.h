#pragma once //

#include "Stallout/containers.h"
#include "Stallout/strings.h"


NS_BEGIN(stallout);
NS_BEGIN(compiler);

struct Parser;

enum Token_Flags : u64 {
    TOKEN_FLAG_LITERAL_U8 = BIT64(1),
    TOKEN_FLAG_LITERAL_S8 = BIT64(2),
    TOKEN_FLAG_LITERAL_U16 = BIT64(3),
    TOKEN_FLAG_LITERAL_S16 = BIT64(4),
    TOKEN_FLAG_LITERAL_U32 = BIT64(5),
    TOKEN_FLAG_LITERAL_S32 = BIT64(6),
    TOKEN_FLAG_LITERAL_U64 = BIT64(7),
    TOKEN_FLAG_LITERAL_S64 = BIT64(8),
    TOKEN_FLAG_LITERAL_F32 = BIT64(9),
    TOKEN_FLAG_LITERAL_F64 = BIT64(9),
    TOKEN_FLAG_LITERAL_STRING = BIT64(9),
};

enum Token_Type {
    TOKEN_TYPE_EOF = '\0',
    TOKEN_TYPE_ASCII_START,

    TOKEN_TYPE_CLOSING_PLUS = '+',
    TOKEN_TYPE_CLOSING_MINUS = '-',
    TOKEN_TYPE_CLOSING_DIVIDE = '/',
    TOKEN_TYPE_CLOSING_MULTIPLY = '*',
    TOKEN_TYPE_CLOSING_EQUALS = '=',
    TOKEN_TYPE_MORETHAN = '>',
    TOKEN_TYPE_LESSTHAN = '<',
    TOKEN_TYPE_POINT = '^',
    TOKEN_TYPE_AT = '@',
    TOKEN_TYPE_DOT = '.',
    TOKEN_TYPE_COMMA = ',',
    TOKEN_TYPE_NOT = '!',
    TOKEN_TYPE_COLON = ':',
    TOKEN_TYPE_SEMICOLON = ';',
    TOKEN_TYPE_EQUALS = '=',
    TOKEN_TYPE_OPENING_PARENTHESIS = '(',
    TOKEN_TYPE_CLOSING_PARENTHESIS = ')',
    TOKEN_TYPE_OPENING_BRACKET = '{',
    TOKEN_TYPE_CLOSING_BRACKET = '}',
    TOKEN_TYPE_QUOTE = '"',
    TOKEN_TYPE_ASCII_END,

    __ = 256,

    TOKEN_TYPE_IDENTIFIER, // Not a keyword and not one of the ascii tokens; has to start with a letter or _ and may contain numbers and _

    TOKEN_TYPE_LITERAL,

    // Keywords
    TOKEN_TYPE_RETURN, // "return"
    TOKEN_TYPE_IF, // "if"
    TOKEN_TYPE_STRUCT, // "struct"

    TOKEN_TYPE_USE, // "#use"
    TOKEN_TYPE_EXTERN, // "#extern"

    TOKEN_TYPE_UNEXPECTED,

};

struct Token_Info {
    u32 line = 0;
    u32 char_index = 0; // Index of char in line
    String filepath = "NOFILE";
    String full_line = "";
    size_t input_begin_index = 0;
    size_t input_end_index = 0;
};
typedef Token_Info Location_Info;

struct Token {
    Token() {}
    ~Token() {}
    Token_Type type = TOKEN_TYPE_EOF;
    union {
        u64 int_value = 0;
        f32 float32_value;
        f64 float64_value;
        String string_value;
    };
    Token_Info info;
};

struct Lexer {

    Parser* _parser;
    Linear_Allocator _token_allocator;
    bool _token_buffer_overflowed = false;
    String _input = "";
    Array<Token*> _tokens;
    Array<Token*> _overflow_tokens;
    Hash_Map<size_t, Token*> _input_pos_tokens;
    size_t _input_pos;
    u32 _current_line = 1;
    u32 _current_char_index = 0;
    Location_Info _location;

    // TODO: Magic number
    Lexer(Parser* parser, size_t token_buffer_size = 1024 * 1000 * 10);
    ~Lexer();

    bool add_input_from_file(const char* path);

    Token* eat();
    Token* peek();

    Token* _make_token();
};  

NS_END(compiler);
NS_END(stallout);