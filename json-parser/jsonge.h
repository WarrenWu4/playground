#pragma once

#include <cstdint>
#include <iomanip>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <cmath>

namespace json {

// Forward declarations
class Value;
using Object = std::unordered_map<std::string, Value>;
using Array = std::vector<Value>;

enum class Type {
    Null,
    Bool,
    Number,
    String,
    Array,
    Object
};

class Value {
public:
    Value() : type_(Type::Null) {}
    Value(std::nullptr_t) : type_(Type::Null) {}
    Value(bool b) : type_(Type::Bool), bool_(b) {}
    Value(int i) : type_(Type::Number), number_(i) {}
    Value(double d) : type_(Type::Number), number_(d) {}
    Value(const char* s) : type_(Type::String), string_(std::make_unique<std::string>(s)) {}
    Value(const std::string& s) : type_(Type::String), string_(std::make_unique<std::string>(s)) {}
    Value(std::string&& s) : type_(Type::String), string_(std::make_unique<std::string>(std::move(s))) {}
    Value(const Array& a) : type_(Type::Array), array_(std::make_unique<Array>(a)) {}
    Value(Array&& a) : type_(Type::Array), array_(std::make_unique<Array>(std::move(a))) {}
    Value(const Object& o) : type_(Type::Object), object_(std::make_unique<Object>(o)) {}
    Value(Object&& o) : type_(Type::Object), object_(std::make_unique<Object>(std::move(o))) {}

    // Copy constructor
    Value(const Value& other) : type_(other.type_) {
        switch (type_) {
            case Type::Bool: bool_ = other.bool_; break;
            case Type::Number: number_ = other.number_; break;
            case Type::String: string_ = std::make_unique<std::string>(*other.string_); break;
            case Type::Array: array_ = std::make_unique<Array>(*other.array_); break;
            case Type::Object: object_ = std::make_unique<Object>(*other.object_); break;
            default: break;
        }
    }

    // Move constructor
    Value(Value&& other) noexcept : type_(other.type_) {
        switch (type_) {
            case Type::Bool: bool_ = other.bool_; break;
            case Type::Number: number_ = other.number_; break;
            case Type::String: string_ = std::move(other.string_); break;
            case Type::Array: array_ = std::move(other.array_); break;
            case Type::Object: object_ = std::move(other.object_); break;
            default: break;
        }
        other.type_ = Type::Null;
    }

    // Assignment operators
    Value& operator=(const Value& other) {
        if (this != &other) {
            this->~Value();
            new (this) Value(other);
        }
        return *this;
    }

    Value& operator=(Value&& other) noexcept {
        if (this != &other) {
            this->~Value();
            new (this) Value(std::move(other));
        }
        return *this;
    }

    ~Value() {
        switch (type_) {
            case Type::String: string_.reset(); break;
            case Type::Array: array_.reset(); break;
            case Type::Object: object_.reset(); break;
            default: break;
        }
    }

    Type type() const { return type_; }
    
    bool isNull() const { return type_ == Type::Null; }
    bool isBool() const { return type_ == Type::Bool; }
    bool isNumber() const { return type_ == Type::Number; }
    bool isString() const { return type_ == Type::String; }
    bool isArray() const { return type_ == Type::Array; }
    bool isObject() const { return type_ == Type::Object; }

    bool asBool() const {
        if (type_ != Type::Bool) throw std::runtime_error("Value is not a boolean");
        return bool_;
    }

    double asNumber() const {
        if (type_ != Type::Number) throw std::runtime_error("Value is not a number");
        return number_;
    }

    int asInt() const {
        if (type_ != Type::Number) throw std::runtime_error("Value is not a number");
        return static_cast<int>(number_);
    }

    const std::string& asString() const {
        if (type_ != Type::String) throw std::runtime_error("Value is not a string");
        return *string_;
    }

    const Array& asArray() const {
        if (type_ != Type::Array) throw std::runtime_error("Value is not an array");
        return *array_;
    }

    Array& asArray() {
        if (type_ != Type::Array) throw std::runtime_error("Value is not an array");
        return *array_;
    }

    const Object& asObject() const {
        if (type_ != Type::Object) throw std::runtime_error("Value is not an object");
        return *object_;
    }

    Object& asObject() {
        if (type_ != Type::Object) throw std::runtime_error("Value is not an object");
        return *object_;
    }

    // Array access
    const Value& operator[](size_t index) const {
        return asArray()[index];
    }

    Value& operator[](size_t index) {
        return asArray()[index];
    }

    // Object access
    const Value& operator[](const std::string& key) const {
        const auto& obj = asObject();
        auto it = obj.find(key);
        if (it == obj.end()) {
            static const Value null_value;
            return null_value;
        }
        return it->second;
    }

    Value& operator[](const std::string& key) {
        if (type_ == Type::Null) {
            *this = Value(Object{});
        }
        return asObject()[key];
    }

    std::string toString() const {
        std::ostringstream oss;
        serialize(oss);
        return oss.str();
    }

private:
    Type type_;
    union {
        bool bool_;
        double number_;
        std::unique_ptr<std::string> string_;
        std::unique_ptr<Array> array_;
        std::unique_ptr<Object> object_;
    };

    void serialize(std::ostringstream& oss) const {
        switch (type_) {
            case Type::Null:
                oss << "null";
                break;
            case Type::Bool:
                oss << (bool_ ? "true" : "false");
                break;
            case Type::Number:
                if (std::floor(number_) == number_ && number_ >= INT64_MIN && number_ <= INT64_MAX) {
                    oss << static_cast<int64_t>(number_);
                } else {
                    oss << number_;
                }
                break;
            case Type::String:
                oss << '"';
                for (char c : *string_) {
                    switch (c) {
                        case '"': oss << "\\\""; break;
                        case '\\': oss << "\\\\"; break;
                        case '\b': oss << "\\b"; break;
                        case '\f': oss << "\\f"; break;
                        case '\n': oss << "\\n"; break;
                        case '\r': oss << "\\r"; break;
                        case '\t': oss << "\\t"; break;
                        default:
                            if (c < 0x20) {
                                oss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)(unsigned char)c;
                            } else {
                                oss << c;
                            }
                            break;
                    }
                }
                oss << '"';
                break;
            case Type::Array:
                oss << '[';
                for (size_t i = 0; i < array_->size(); ++i) {
                    if (i > 0) oss << ',';
                    (*array_)[i].serialize(oss);
                }
                oss << ']';
                break;
            case Type::Object:
                oss << '{';
                bool first = true;
                for (const auto& [key, value] : *object_) {
                    if (!first) oss << ',';
                    first = false;
                    oss << '"' << key << "\":";
                    value.serialize(oss);
                }
                oss << '}';
                break;
        }
    }
};

class Parser {
public:
    static Value parse(const std::string& json) {
        Parser parser(json);
        return parser.parseValue();
    }

private:
    const char* ptr_;
    const char* end_;

    explicit Parser(const std::string& json) 
        : ptr_(json.c_str()), end_(ptr_ + json.size()) {}

    void skipWhitespace() {
        while (ptr_ < end_ && (*ptr_ == ' ' || *ptr_ == '\t' || *ptr_ == '\n' || *ptr_ == '\r')) {
            ++ptr_;
        }
    }

    void expect(char c) {
        if (ptr_ >= end_ || *ptr_ != c) {
            throw std::runtime_error(std::string("Expected '") + c + "'");
        }
        ++ptr_;
    }

    Value parseValue() {
        skipWhitespace();
        if (ptr_ >= end_) {
            throw std::runtime_error("Unexpected end of input");
        }

        switch (*ptr_) {
            case 'n': return parseNull();
            case 't': case 'f': return parseBool();
            case '"': return parseString();
            case '[': return parseArray();
            case '{': return parseObject();
            case '-': case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                return parseNumber();
            default:
                throw std::runtime_error(std::string("Unexpected character: ") + *ptr_);
        }
    }

    Value parseNull() {
        if (end_ - ptr_ >= 4 && std::strncmp(ptr_, "null", 4) == 0) {
            ptr_ += 4;
            return Value();
        }
        throw std::runtime_error("Invalid null value");
    }

    Value parseBool() {
        if (end_ - ptr_ >= 4 && std::strncmp(ptr_, "true", 4) == 0) {
            ptr_ += 4;
            return Value(true);
        }
        if (end_ - ptr_ >= 5 && std::strncmp(ptr_, "false", 5) == 0) {
            ptr_ += 5;
            return Value(false);
        }
        throw std::runtime_error("Invalid boolean value");
    }

    Value parseString() {
        expect('"');
        std::string result;
        
        while (ptr_ < end_ && *ptr_ != '"') {
            if (*ptr_ == '\\') {
                ++ptr_;
                if (ptr_ >= end_) {
                    throw std::runtime_error("Unterminated string escape");
                }
                
                switch (*ptr_) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    case 'u': {
                        ++ptr_;
                        if (end_ - ptr_ < 4) {
                            throw std::runtime_error("Invalid unicode escape");
                        }
                        
                        int codepoint = 0;
                        for (int i = 0; i < 4; ++i) {
                            char c = ptr_[i];
                            int digit;
                            if (c >= '0' && c <= '9') digit = c - '0';
                            else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
                            else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
                            else throw std::runtime_error("Invalid unicode escape");
                            
                            codepoint = (codepoint << 4) | digit;
                        }
                        
                        // Convert codepoint to UTF-8
                        if (codepoint <= 0x7F) {
                            result += static_cast<char>(codepoint);
                        } else if (codepoint <= 0x7FF) {
                            result += static_cast<char>(0xC0 | (codepoint >> 6));
                            result += static_cast<char>(0x80 | (codepoint & 0x3F));
                        } else {
                            result += static_cast<char>(0xE0 | (codepoint >> 12));
                            result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                            result += static_cast<char>(0x80 | (codepoint & 0x3F));
                        }
                        ptr_ += 3; // Will be incremented at end of loop
                        break;
                    }
                    default:
                        throw std::runtime_error(std::string("Invalid escape sequence: \\") + *ptr_);
                }
            } else {
                result += *ptr_;
            }
            ++ptr_;
        }
        
        expect('"');
        return Value(std::move(result));
    }

    Value parseNumber() {
        const char* start = ptr_;
        
        // Handle negative sign
        if (*ptr_ == '-') ++ptr_;
        
        // Parse integer part
        if (ptr_ >= end_ || !isDigit(*ptr_)) {
            throw std::runtime_error("Invalid number");
        }
        
        if (*ptr_ == '0') {
            ++ptr_;
        } else {
            while (ptr_ < end_ && isDigit(*ptr_)) {
                ++ptr_;
            }
        }
        
        bool isFloat = false;
        
        // Parse fractional part
        if (ptr_ < end_ && *ptr_ == '.') {
            isFloat = true;
            ++ptr_;
            if (ptr_ >= end_ || !isDigit(*ptr_)) {
                throw std::runtime_error("Invalid number: missing digits after decimal point");
            }
            while (ptr_ < end_ && isDigit(*ptr_)) {
                ++ptr_;
            }
        }
        
        // Parse exponent
        if (ptr_ < end_ && (*ptr_ == 'e' || *ptr_ == 'E')) {
            isFloat = true;
            ++ptr_;
            if (ptr_ < end_ && (*ptr_ == '+' || *ptr_ == '-')) {
                ++ptr_;
            }
            if (ptr_ >= end_ || !isDigit(*ptr_)) {
                throw std::runtime_error("Invalid number: missing exponent digits");
            }
            while (ptr_ < end_ && isDigit(*ptr_)) {
                ++ptr_;
            }
        }
        
        std::string numStr(start, ptr_ - start);
        double value = std::stod(numStr);
        return Value(value);
    }

    Value parseArray() {
        expect('[');
        Array result;
        
        skipWhitespace();
        if (ptr_ < end_ && *ptr_ == ']') {
            ++ptr_;
            return Value(std::move(result));
        }
        
        while (true) {
            result.push_back(parseValue());
            
            skipWhitespace();
            if (ptr_ >= end_) {
                throw std::runtime_error("Unterminated array");
            }
            
            if (*ptr_ == ']') {
                ++ptr_;
                break;
            } else if (*ptr_ == ',') {
                ++ptr_;
                skipWhitespace();
            } else {
                throw std::runtime_error("Expected ',' or ']' in array");
            }
        }
        
        return Value(std::move(result));
    }

    Value parseObject() {
        expect('{');
        Object result;
        
        skipWhitespace();
        if (ptr_ < end_ && *ptr_ == '}') {
            ++ptr_;
            return Value(std::move(result));
        }
        
        while (true) {
            // Parse key
            skipWhitespace();
            if (ptr_ >= end_ || *ptr_ != '"') {
                throw std::runtime_error("Expected string key in object");
            }
            
            Value keyValue = parseString();
            std::string key = keyValue.asString();
            
            // Parse colon
            skipWhitespace();
            expect(':');
            
            // Parse value
            Value value = parseValue();
            result[key] = std::move(value);
            
            skipWhitespace();
            if (ptr_ >= end_) {
                throw std::runtime_error("Unterminated object");
            }
            
            if (*ptr_ == '}') {
                ++ptr_;
                break;
            } else if (*ptr_ == ',') {
                ++ptr_;
            } else {
                throw std::runtime_error("Expected ',' or '}' in object");
            }
        }
        
        return Value(std::move(result));
    }

    static bool isDigit(char c) {
        return c >= '0' && c <= '9';
    }
};

// Convenience functions
inline Value parse(const std::string& json) {
    return Parser::parse(json);
}

inline std::string stringify(const Value& value) {
    return value.toString();
}

} // namespace json

