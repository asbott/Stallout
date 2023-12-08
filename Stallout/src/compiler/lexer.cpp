#include "pch.h"

#include "compiler/lexer.h"

#include "os/io.h"

#include "Stallout/memory.h"

bool is_whitespace(char c) {
    return c == ' ' || c == '\n' || c == '\r\n' || c == '\t';
}
bool is_linebreak(char c) {
    return c == '\n' || c == '\r\n';
}
bool is_identifier_friendly(char c) {
    return isalnum(c) || c == '_';
}
bool is_identifier_starter(char c) {
    return isalpha(c) || c == '_';
}

NS_BEGIN(stallout);
NS_BEGIN(compiler);

Lexer::Lexer(Parser* parser, size_t token_buffer_size)
    : _parser(parser), _token_allocator(token_buffer_size) {

}
Lexer::~Lexer() {
    for (auto tok : _overflow_tokens) {
        ST_DELETE(tok);
    }
}

bool Lexer::add_input_from_file(const char* path)  {
    os::io::File_Info file_info;
    if (os::io::get_file_info(path, &file_info) != IO_STATUS_OK) return false;

    //char* content = (char*)ST_MEM(file_info.file_size + 1);

    //if (os::io::read_as_string(path, content, file_info.file_size) != IO_STATUS_OK) return false;

    //_input.concat(content);

    return true;
}

Token* Lexer::eat() {
    Token* tok = peek();

    _input_pos = tok->info.input_end_index + 1;

    return tok;
}
Token* Lexer::peek() {
    if (_input_pos_tokens.contains(_input_pos)) {
        return _input_pos_tokens[_input_pos];
    }

    Token* tok = _make_token();

    size_t pos = _input_pos;

    tok->info.input_begin_index = pos;

    char c = _input[pos];
    
    if (c == '\0') {
        tok->type = TOKEN_TYPE_EOF;
    } else {
        static Array<char> word;
        word.resize(0);

        while (c != '\0') {
            char next = _input[pos + 1];

            if (c == '/' && next == '/') {
                while (!is_linebreak(c) && next != '\0') {
                    pos++;
                    c = _input[pos];
                    next = _input[pos + 1];

                    //if (c == '\0')
                }
            }
        }
    }

    tok->info.line = _current_line;

    return tok;
}

Token* Lexer::_make_token() {
    Token* tok = _token_allocator.allocate_and_construct<Token>();
    
    if (!tok) {
        tok = 0;//stnew (Token);
        _overflow_tokens.push_back(tok);

        if (!_token_buffer_overflowed) {
            _token_buffer_overflowed = true;
            log_warn("Token allocator in Lexer has overflown; using fallback global allocator.\nConsider allocating a bigger token buffer.");
        }
    }

    _tokens.push_back(tok);
    _input_pos_tokens[_input_pos] = tok;
    return tok;
}

NS_END(compiler);
NS_END(stallout);