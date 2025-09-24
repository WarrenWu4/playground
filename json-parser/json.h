#pragma once

#include <any>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <istream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>
#include <regex>
#include <variant>
#include <cmath>
#include <limits>
#include <assert.h>

struct JsonValue;
using JsonArray = std::vector<JsonValue>;
using JsonObject = std::map<std::string, JsonValue>;
using JsonType = std::variant<
    std::nullptr_t,
    bool,
    double,
    std::string,
    JsonArray,
    JsonObject
>;

struct JsonValue {
    JsonType value;
    bool isNull() const { return std::holds_alternative<std::nullptr_t>(value); }
    bool isBool() const { return std::holds_alternative<bool>(value); }
    bool isNumber() const { return std::holds_alternative<double>(value); }
    bool isString() const { return std::holds_alternative<std::string>(value); }
    bool isArray() const { return std::holds_alternative<JsonArray>(value); }
    bool isObject() const { return std::holds_alternative<JsonObject>(value); }
    bool getBool() const {
        if (!isBool()) throw std::runtime_error("Type is not bool");
        return std::get<bool>(value);
    }
    double getNumber() const {
        if (!isNumber()) throw std::runtime_error("Type is not number");
        return std::get<double>(value);
    }
    const std::string& getString() const {
        if (!isString()) throw std::runtime_error("Type is not string");
        return std::get<std::string>(value);
    }
    const JsonArray& getArray() const {
        if (!isArray()) throw std::runtime_error("Type is not array");
        return std::get<JsonArray>(value);
    }
    const JsonObject& getObject() const {
        if (!isObject()) throw std::runtime_error("Type is not object");
        return std::get<JsonObject>(value);
    }
};

class Json {
private:
    std::string data;
    size_t end;
    size_t ptr;

    void skipWhitespace() {
        while (ptr < end && std::isspace(data[ptr])) { ptr++; }
    }

    JsonValue parseValue() {
        skipWhitespace();
        assert(ptr < end);
        switch (data[ptr]) {
            case 'n': { return parseNull(); }
            case 't': case 'f': { return parseBool(); }
            case '-': case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': { return parseNumber(); }
            case '"': { return parseString(); }
            case '[': { return parseArray(); }
            case '{': { return parseObject(); } 
            default: {
                throw std::runtime_error("Unexpected character");
            }
        }
    }
    
    JsonValue parseNull() {
        assert(ptr < end);
        if (data.substr(ptr, 4) == "null") { ptr += 4; return {nullptr}; }
        throw std::runtime_error("Cannot parse null value");
    }

    JsonValue parseBool() {
        assert(ptr < end);
        if (data.substr(ptr, 4) == "true") { ptr += 4; return {true}; }
        if (data.substr(ptr, 5) == "false") { ptr += 5; return {false}; }
        throw std::runtime_error("Cannot parse boolean value");
    }

    JsonValue parseNumber() {
        size_t start = ptr;
        while (ptr < data.length() && (isdigit(static_cast<unsigned char>(data[ptr])) || data[ptr] == '.' || data[ptr] == '-' || data[ptr] == 'e' || data[ptr] == 'E')) {
            ptr++;
        }
        return {std::stod(data.substr(start, ptr - start))};
    }

    JsonValue parseString() {
        ptr++;
        std::string result = "";
        while (ptr < data.length() && data[ptr] != '"') {
            if (data[ptr] == '\\') {
                ptr++;
                if (ptr >= data.length()) {
                    throw std::runtime_error("Invalid escape sequence");
                }
                switch (data[ptr]) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/': result += '/'; break;
                    case 'b': result += '\b'; break;
                    case 'f': result += '\f'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    default: throw std::runtime_error("Invalid escape sequence");
                }
            } else {
                result += data[ptr];
            }
            ptr++;
        }
        if (ptr >= data.length()) {
            throw std::runtime_error("Unterminated string literal");
        }
        ptr++; // Skip final '"'
        return {result};
    }

    JsonValue parseArray() {
        ptr++;
        skipWhitespace();
        JsonArray arr;
        if (data[ptr] == ']') {
            ptr++;
            return {arr};
        }
        while (true) {
            arr.push_back(parseValue());
            skipWhitespace();
            if (data[ptr] == ']') {
                ptr++;
                break;
            } else if (data[ptr] != ',') {
                throw std::runtime_error("Expected ',' or ']'");
            }
            ptr++;
        }
        return {arr};
    }

    JsonValue parseObject() {
        ptr++;
        skipWhitespace();
        JsonObject obj;
        if (data[ptr] == '}') {
            ptr++;
            return {obj};
        }
        while (true) {
            std::string key = std::get<std::string>(parseString().value);
            skipWhitespace();
            if (data[ptr] != ':') { throw std::runtime_error("Expected ':'"); }
            ptr++;
            skipWhitespace();
            obj[key] = parseValue();
            skipWhitespace();
            if (data[ptr] == '}') {
                ptr++;
                break;
            } else if (data[ptr] != ',') {
                throw std::runtime_error("Expected ',' or '}'");
            }
            ptr++;
            skipWhitespace();
        }
        return {obj};
    }

    std::string readFile(std::string path) {
        std::ifstream file(path.c_str());
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open file");
        }
        std::string res(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );
        return res;
    }

    void stringifyValue(JsonValue jv, std::string& res, int level) {
        if (jv.isNull()) {
            res += "null";
        } else if (jv.isBool()) {
            res += (jv.getBool()) ? "true" : "false";
        } else if (jv.isNumber()) {
            double jvNum = jv.getNumber();
            double integral_part;
            double fractional_part = std::modf(jvNum, &integral_part);
            if (std::fabs(fractional_part) > std::numeric_limits<double>::epsilon()) {
                res += std::to_string(jvNum);
            } else if (jvNum < std::numeric_limits<int>::min() || jvNum > std::numeric_limits<int>::max()) {
                res += std::to_string(jvNum);
            } else {
                res += std::to_string(static_cast<int>(jvNum));
            }
        } else if (jv.isString()) {
            res += "\"" + jv.getString() + "\"";
        } else if (jv.isArray()) {
            res += "[\n";
            JsonArray jvArr = jv.getArray();
            for (size_t i = 0; i < jvArr.size(); i++) {
                for (int j = 0; j < level; j++) {
                    res += "\t";
                }
                stringifyValue(jvArr.at(i), res, level+1);
                if (i < jvArr.size()-1) {
                    res += ",";
                }
                res += "\n";
            }
            for (int j = 0; j < level; j++) {
                res += "\t";
            }
            res += "]";
        } else if (jv.isObject()) {
            res += "{\n";
            JsonObject jvObj = jv.getObject();
            size_t counter = 0;
            for (const auto& pair : jvObj) {
                for (int j = 0; j < level; j++) {
                    res += "\t";
                }
                res += "\t";
                res += "\"" + pair.first + "\"" + ": ";
                stringifyValue(pair.second, res, level+1);
                if (counter < jvObj.size()-1) {
                    res += ",";
                }
                res += "\n";
                counter += 1;
            }
            for (int j = 0; j < level; j++) {
                res += "\t";
            }
            res += "}";
        } else {
            throw std::runtime_error("Invalid JsonValue.");
        }
    }

public:
    JsonValue parse(std::string text) {
        data = text;
        end = data.size();
        ptr = 0;
        return parseValue();
    }

    JsonValue parseFromFile(std::string path) {
        data = readFile(path);
        end = data.size();
        ptr = 0;
        return parseValue();
    }

    std::string stringify(JsonValue jv) {
        std::string res = "";
        assert(jv.isArray() || jv.isObject());
        stringifyValue(jv, res, 0);
        return res;
    }

    void writeToFile(std::string path, std::string json) {
        std::ofstream file(path.c_str());
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open file");
        }
        file << json;
        file.close();
    }
};
