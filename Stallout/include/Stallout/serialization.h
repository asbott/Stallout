#pragma once //

#include "Stallout/containers.h"
#include "Stallout/strings.h"

NS_BEGIN(stallout);
NS_BEGIN(serialization);



enum Serialize_Mode {
    SERIALIZE_MODE_BINARY,
    SERIALIZE_MODE_TEXT
};
enum Archive_Field_Type {
    ARCHIVE_FIELD_TYPE_INT, // s64
    ARCHIVE_FIELD_TYPE_FLOAT, // f64
    ARCHIVE_FIELD_TYPE_STRING
};
constexpr const char* archive_file_type_string(Archive_Field_Type v) {
    switch (v) {
        case ARCHIVE_FIELD_TYPE_INT: return "int";
        case ARCHIVE_FIELD_TYPE_FLOAT: return "float";
        case ARCHIVE_FIELD_TYPE_STRING: return "string";
        default: INTENTIONAL_CRASH("Unhandled enum"); return "";
    };
}

struct ST_API Archive {
    struct Value {
        Value() {
            
        }
        ~Value() {}

        Value(const Value& src) { 
            type = src.type;

            if (type == ARCHIVE_FIELD_TYPE_STRING) {
                string_value = src.string_value;
            } else {
                memcpy(&int_value, &src.int_value, sizeof(int_value));
            }
        }

        Value& operator =(const Value& src) {
            type = src.type;

            if (type == ARCHIVE_FIELD_TYPE_STRING) {
                string_value = src.string_value;
            } else {
                memcpy(&int_value, &src.int_value, sizeof(int_value));
            }
            return *this;
        }
        union {
            s64 int_value;
            f64 float_value;
            String string_value = "";
        };
        Archive_Field_Type type;
    };
    stallout::Hash_Map<String, Value> fields;

    Archive();
    ~Archive();

    void set_int(const char* key, s64 v);
    void set_float(const char* key, f64 v);
    void set_string(const char* key, const String& v);

    void unset(const char* key);

    bool is_int(const char* key) const;
    bool is_float(const char* key) const;
    bool is_string(const char* key) const;

    s64 get_int(const char* key) const;
    f64 get_float(const char* key) const;
    String get_string(const char* key) const;

    bool contains_key(const char* key) const;

    Archive_Field_Type get_type(const char* key) const;

    void clear() { fields.clear(); }

    template <typename type_t>
    void set(const char* key, type_t v) {
        if constexpr (std::is_integral_v<type_t>) {
            set_int(key, (s64)v);
        } else if constexpr (std::is_floating_point_v<type_t>) {
            set_float(key, (f64)v);
        } else if constexpr (std::is_same_v<type_t, char*> || std::is_same_v<type_t, const char*> || std::is_same_v<type_t, String>) {
            set_string(key, v);
        } else  {
            //INTENTIONAL_CRASH("Invalid type");
        }
    }
    

    bool dump(const char* path, Serialize_Mode mode = SERIALIZE_MODE_TEXT) const;
    bool load(const char* path, Serialize_Mode *mode = NULL);
};



NS_END(serialization);
NS_END(stallout);