#include "pch.h"

#include "Stallout/serialization.h"
#include "Stallout/utils.h"
#include <inttypes.h>
#include "os/io.h"

#define MAGIC_STR_TEXT_MODE "@STALLOUT_ARCHIVE\n"
#define MAGIC_STR_BINARY_MODE "~M&/F#9|*G)_"

NS_BEGIN(stallout);
NS_BEGIN(serialization);


Archive::Archive() {

}
Archive::~Archive() {
    
}

void Archive::set_int(const char* key, s64 v) {
    fields[key].int_value = v;
    fields[key].type = ARCHIVE_FIELD_TYPE_INT;
}
void Archive::set_float(const char* key, f64 v) {
    fields[key].float_value = v;
    fields[key].type = ARCHIVE_FIELD_TYPE_FLOAT;
}
void Archive::set_string(const char* key, const String& v) {
    fields[key].string_value = v;
    fields[key].type = ARCHIVE_FIELD_TYPE_STRING;
}

void Archive::unset(const char* key) {
    if (contains_key(key)) fields.erase(key);
}

bool Archive::is_int(const char* key) const {
    return contains_key(key) && get_type(key) == ARCHIVE_FIELD_TYPE_INT;   
}
bool Archive::is_float(const char* key) const {
    return contains_key(key) && get_type(key) == ARCHIVE_FIELD_TYPE_FLOAT;
}
bool Archive::is_string(const char* key) const {
    return contains_key(key) && get_type(key) == ARCHIVE_FIELD_TYPE_STRING;
}

s64 Archive::get_int(const char* key) const {
    ST_DEBUG_ASSERT(is_int(key), "Archive field type mismatch. Requested 'int', is '{}", archive_file_type_string(get_type(key)));
    return fields.at(key).int_value;
}
f64 Archive::get_float(const char* key) const {
    ST_DEBUG_ASSERT(is_float(key), "Archive field type mismatch. Requested 'float', is '{}'", archive_file_type_string(get_type(key)));
    return fields.at(key).float_value;
}
String Archive::get_string(const char* key) const {
    ST_DEBUG_ASSERT(is_string(key), "Archive field type mismatch. Requested 'string', is '{}", archive_file_type_string(get_type(key)));
    return fields.at(key).string_value;
}

bool Archive::contains_key(const char* key) const {
    return fields.contains(key);    
}

Archive_Field_Type Archive::get_type(const char* key) const  {
    ST_DEBUG_ASSERT(contains_key(key), "No such key '{}'", key);
    return fields.at(key).type;
}

bool Archive::dump(const char* path, Serialize_Mode mode) const {
    if (mode == SERIALIZE_MODE_TEXT) {
        String str(strlen(MAGIC_STR_TEXT_MODE));
        for (int i = 0; i < strlen(MAGIC_STR_TEXT_MODE); i++) {
            str.concat(MAGIC_STR_TEXT_MODE[i]);
        }
        
        for(auto& [key, value] : fields) {
            if (value.type == ARCHIVE_FIELD_TYPE_INT) {
                str.concat("%s=%" PRId64 "\n", key.str, value.int_value);
            } else if (value.type == ARCHIVE_FIELD_TYPE_FLOAT) {
                str.concat("%s=%f\n", key.str, value.float_value);
            } else if (value.type == ARCHIVE_FIELD_TYPE_STRING) {
                str.concat("%s=\"%s\"\n", key.str, value.string_value.str);
            } else {
                INTENTIONAL_CRASH("Unhandeled enum");
            }
        }

        return os::io::write_string(path, str) == IO_STATUS_OK;

    } else if (mode == SERIALIZE_MODE_BINARY) {
        stallout::Array<byte_t> bytes;

        bytes.reserve(strlen(MAGIC_STR_BINARY_MODE));
        // Set first magic bytes to signify binary mode
        for (int i = 0; i < strlen(MAGIC_STR_BINARY_MODE); i++) {
            bytes.push_back(MAGIC_STR_BINARY_MODE[i]);
        }

        for(auto& [key, value] : fields) {
            bytes.push_back((byte_t)value.type);

            size_t key_len = strlen(key.str);
            stallout::utils::append_bytes(bytes, &key_len, sizeof(key_len));
            stallout::utils::append_bytes(bytes, key.str, key.len());

            if (value.type == ARCHIVE_FIELD_TYPE_INT) {
                auto val = value.int_value;
                stallout::utils::append_bytes(bytes, &val, sizeof(val));
            } else if (value.type == ARCHIVE_FIELD_TYPE_FLOAT) {
                auto val = value.float_value;
                stallout::utils::append_bytes(bytes, &val, sizeof(val));
            } else if (value.type == ARCHIVE_FIELD_TYPE_STRING) {
                char* val = value.string_value.str;
                size_t val_len = strlen(val);

                stallout::utils::append_bytes(bytes, &val_len, sizeof(val_len));
                stallout::utils::append_bytes(bytes, val, val_len);

            } else {
                INTENTIONAL_CRASH("Unhandeled enum");
            }
        }

        return os::io::write_bytes(path, bytes.data(), bytes.size()) == IO_STATUS_OK;
    }
    return false;
}
bool Archive::load(const char* path, Serialize_Mode *mode) {
    os::io::File_Info fifo;
    if (os::io::get_file_info(path, &fifo) != IO_STATUS_OK) {
        return false;
    }
    
    byte_t* bytes = (byte_t*)ST_MEM(fifo.file_size + 1); // +1 in case it's text mode and we need null terminator
    bytes[fifo.file_size] = 0;

    if (os::io::read_all_bytes(path, bytes, fifo.file_size) != IO_STATUS_OK) {
        return false;
    }

    if (fifo.file_size >= strlen(MAGIC_STR_TEXT_MODE) && memcmp(bytes, MAGIC_STR_TEXT_MODE, strlen(MAGIC_STR_TEXT_MODE)) == 0) {
        String str = (char*)(bytes + strlen(MAGIC_STR_TEXT_MODE));
        if (mode) *mode = SERIALIZE_MODE_TEXT;

        Array<String> lines;

        split_string(str, '\n', &lines);

        for (auto& line : lines) {
            s64 eq_idx = line.first_index('=');
            if (eq_idx == -1) {
                ST_FREE(bytes, fifo.file_size + 1);
                return false;
            }

            String key = line.sub_string(0, eq_idx);
            String value_str = line.sub_string(eq_idx + 1);

            if (is_numeric(value_str.str)) {

                if (value_str.contains('.')) {
                    f64 val;
                    if (!try_parse_f64(value_str.str, &val)) {
                        ST_FREE(bytes, fifo.file_size + 1);
                        return false;
                    }
                    set_float(key.str, val);
                } else {
                    s64 val;
                    if (!try_parse_int(value_str.str, &val)) {
                        ST_FREE(bytes, fifo.file_size + 1);
                        return false;
                    }
                    set_int(key.str, val);
                }

            } else {
                String strvalue = value_str.sub_string(value_str.first_index('\"') + 1, value_str.last_index('\"')-1);
                set_string(key.str, strvalue);
            }
        }


        ST_FREE(bytes, fifo.file_size + 1);
        return true;
    } else if (fifo.file_size >= strlen(MAGIC_STR_BINARY_MODE) && memcmp(bytes, MAGIC_STR_BINARY_MODE, strlen(MAGIC_STR_BINARY_MODE)) == 0){
        byte_t* ptr = bytes + strlen(MAGIC_STR_BINARY_MODE);
        if (mode) *mode = SERIALIZE_MODE_BINARY;

        while(ptr < bytes + fifo.file_size) {
            Archive_Field_Type type = (Archive_Field_Type)*ptr++;
            
            size_t key_len;
            memcpy(&key_len, ptr, sizeof(key_len));
            ptr += sizeof(key_len);

            char* key = new char[key_len + 1]; // +1 for null terminator
            memcpy(key, ptr, key_len);
            key[key_len] = '\0'; // null terminate
            ptr += key_len;

            if (type == ARCHIVE_FIELD_TYPE_INT) {
                s64 int_val;
                memcpy(&int_val, ptr, sizeof(int_val));
                ptr += sizeof(int_val);

                set_int(key, int_val);

            } else if (type == ARCHIVE_FIELD_TYPE_FLOAT) {
                f64 float_val;
                memcpy(&float_val, ptr, sizeof(float_val));
                ptr += sizeof(float_val);

                set_float(key, float_val);

            } else if (type == ARCHIVE_FIELD_TYPE_STRING) {
                size_t val_len;
                memcpy(&val_len, ptr, sizeof(val_len));
                ptr += sizeof(val_len);

                String str_val(val_len + 1);
                memcpy(str_val.str, ptr, val_len);
                str_val.str[val_len] = '\0'; // null terminate
                ptr += val_len;

                set_string(key, str_val);

            } else {
                ST_FREE(bytes, fifo.file_size + 1);
                return false;
            }
        }

        ST_FREE(bytes, fifo.file_size + 1);
        return true;
    } else {
        ST_FREE(bytes, fifo.file_size + 1);
        return false;
    }

}

NS_END(serialization);
NS_END(stallout);