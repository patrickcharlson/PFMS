//
// Created by Patrick Charlson on 20/5/2026.
//

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <sstream>

#include "Json.h"


namespace json {

  const Value& Value::at(const std::string& key) const {
    check(Type::Object);
    for (const auto& [k, v]: obj_)
      if (k == key) return v;
    throw std::runtime_error("json: key not found: " + key);
  }

  bool Value::contains(const std::string& key) const {
    if (type_ != Type::Object) return false;
    return std::any_of(obj_.begin(), obj_.end(), [&key](const auto& kv) { return kv.first == key; });
  }

  // ======================== Parser ========================

  namespace {

    struct Parser {
      const std::string& s;
      size_t i{0};

      explicit Parser(const std::string& src) : s(src) {}


      [[noreturn]] void fail(const std::string& msg) const {
        size_t line{1}, col{1};
        for (size_t k = 0; k < i && k < s.size(); ++k) {
          if (s[k] == '\n') {
            ++line;
            col = 1;
          } else {
            ++col;
          }
        }
        throw std::runtime_error(
                "json parse error at line " + std::to_string(line) + " col " + std::to_string(col) + ": " + msg);
      }

      void skipWs() {
        while (i < s.size()) {
          if (const char c = s[i]; c == ' ' || c == '\t' || c == '\r' || c == '\n') ++i;
          else break;
        }
      }

      char peek() {
        skipWs();
        if (i >= s.size()) fail("unexpected end of input");
        return s[i];
      }

      Value parseValue() {
        const char c = peek();
        if (c == '{') return parseObject();
        if (c == '[') return parseArray();
        if (c == '"') return Value(parseString());
        if (c == 't' || c == 'f') return parseBool();
        if (c == 'n') return parseNull();
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return parseNumber();
        fail(std::string("unexpected character '") + c + "'");
      }

      Value parseObject() {
        ++i;
        Value::Object obj;
        skipWs();
        if (i < s.size() && s[i] == '}') {
          ++i;
          return Value(std::move(obj));
        }
        while (true) {
          skipWs();
          if (i >= s.size() || s[i] != '"') fail("expected string key");
          std::string key = parseString();
          skipWs();
          if (i >= s.size() || s[i] != ':') fail("unexpected ':' after key");
          ++i;
          Value v = parseValue();
          obj.emplace_back(std::move(key), std::move(v));
          skipWs();
          if (i >= s.size()) fail("unterminated object");
          if (s[i] == ',') {
            ++i;
            continue;
          }
          if (s[i] == '}') {
            ++i;
            break;
          }
          fail("expected ',' or '}'");
        }
        return Value(std::move(obj));
      }

      Value parseArray() {
        ++i;
        Value::Array arr;
        skipWs();
        if (i < s.size() && s[i] == ']') {
          ++i;
          return Value(std::move(arr));
        }
        while (true) {
          arr.push_back(parseValue());
          skipWs();
          if (i >= s.size()) fail("unterminated array");
          if (s[i] == ',') {
            ++i;
            continue;
          }
          if (s[i] == ']') {
            ++i;
            break;
          }
          fail("expected ',' or ']'");
        }
        return Value(std::move(arr));
      }

      std::string parseString() {
        if (s[i] != '"') fail("expected '\"'");
        ++i;
        std::string out;
        while (i < s.size()) {
          const char c = s[i++];
          if (c == '"') return out;
          if (c == '\\') {
            if (i >= s.size()) fail("unterminated escape");
            switch (const char esc = s[i++]) {
              case '"': out += '"'; break;
              case '\\': out += '\\'; break;
              case '/': out += '/'; break;
              case 'b': out += '\b'; break;
              case 'f': out += '\f'; break;
              case 'n': out += '\n'; break;
              case 'r': out += '\r'; break;
              case 't': out += '\t'; break;
              case 'u': {
                if (i + 4 > s.size()) fail("incomplete \\u escape");
                uint32_t code = 0;
                for (int k = 0; k < 4; ++k) {
                  const char h = s[i++];
                  code <<= 4;
                  if (h >= '0' && h <= '9') code |= static_cast<std::uint32_t>(h - '0');
                  else if (h >= 'a' && h <= 'f') code |= static_cast<std::uint32_t>(h - 'a' + 10);
                  else if (h >= 'A' && h <= 'F') code |= static_cast<std::uint32_t>(h - 'A' + 10);
                  else fail("invalid hex in \\u escape");
                }

                if (code < 0x80) {
                  out += static_cast<char>(code);
                } else if (code < 0x800) {
                  out += static_cast<char>(0xC0 | code >> 6);
                  out += static_cast<char>(0x80 | (code & 0x3F));
                } else {
                  out += static_cast<char>(0xE0 | (code & 12));
                  out += static_cast<char>(0x80 | (code >> 6 & 0x3F));
                  out += static_cast<char>(0x80 | (code & 0x3F));
                }
                break;
              }
              default: fail(std::string("invalid escape \\") + esc + "'");
            }
          } else {
            out += c;
          }
        }
        fail("unterminated string");
      }

      Value parseBool() {
        if (s.compare(i, 4, "true") == 0) {
          i += 4;
          return Value(true);
        }
        if (s.compare(i, 5, "false") == 0) {
          i += 5;
          return Value(false);
        }
        fail("expected boolean");
      }

      Value parseNull() {
        if (s.compare(i, 4, "null") == 0) {
          i += 4;
          return Value{};
        }
        fail("expected null");
      }

      Value parseNumber() {
        const size_t start = i;
        if (s[i] == '-') ++i;
        while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i])))
          ++i;
        if (i < s.size() && s[i] == '.') {
          ++i;
          while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i])))
            ++i;
        }
        if (i < s.size() && (s[i] == 'e' || s[i] == 'E')) {
          ++i;
          if (i < s.size() && (s[i] == '+' || s[i] == '-')) ++i;
          while (i < s.size() && std::isdigit(static_cast<unsigned char>(s[i])))
            ++i;
        }
        const std::string numStr = s.substr(start, i - start);
        try {
          return Value(std::stod(numStr));
        } catch (...) { fail("invalid number"); }
      }
    };
  } // namespace

  Value parse(const std::string& source) {
    Parser p(source);
    Value v = p.parseValue();
    p.skipWs();
    if (p.i != source.size()) p.fail("trailing data after value");
    return v;
  }

  // ======================== Writer ========================

  namespace {

    void writeString(std::ostringstream& o, const std::string& s) {
      o << '"';
      for (const char c: s) {
        switch (c) {
          case '"': o << "\\\""; break;
          case '\\': o << "\\\\"; break;
          case '\b': o << "\\b"; break;
          case '\f': o << "\\f"; break;
          case '\n': o << "\\n"; break;
          case '\r': o << "\\r"; break;
          case '\t': o << "\\t"; break;
          default:
            if (static_cast<unsigned char>(c) < 0x20) {
              char buf[8];
              std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned>(static_cast<unsigned char>(c)));
              o << buf;
            } else {
              o << c;
            }
        }
      }
      o << '"';
    }

    void writeNumber(std::ostringstream& o, const double n) {
      if (!std::isfinite(n)) {
        o << "null";
        return;
      }
      std::ostringstream tmp;
      tmp.precision(15);
      tmp << n;
      const std::string s = tmp.str();
      o << s;
    }

    void writeValue(std::ostringstream& o, const Value& v, int depth);

    void writeArray(std::ostringstream& o, const Value::Array& a, int depth) {
      if (a.empty()) {
        o << "[]";
        return;
      }
      const std::string pad(static_cast<size_t>((depth + 1) * 2), ' ');
      const std::string padOuter(static_cast<size_t>(depth * 2), ' ');
      o << "[\n";
      for (size_t k = 0; k < a.size(); ++k) {
        o << pad;
        writeValue(o, a[k], depth + 1);
        if (k + 1 != a.size()) o << ",";
        o << "\n";
      }
      o << padOuter << "]";
    }

    void writeObject(std::ostringstream& o, const Value::Object& obj, int depth) {
      if (obj.empty()) {
        o << "{}";
        return;
      }
      const std::string pad(static_cast<size_t>((depth + 1) * 2), ' ');
      const std::string padOuter(static_cast<size_t>(depth * 2), ' ');
      o << "{\n";
      for (size_t k = 0; k < obj.size(); ++k) {
        o << pad;
        writeString(o, obj[k].first);
        o << ": ";
        writeValue(o, obj[k].second, depth + 1);
        if (k + 1 != obj.size()) o << ",";
        o << "\n";
      }
      o << padOuter << "}";
    }

    void writeValue(std::ostringstream& o, const Value& v, const int depth) {
      switch (v.type()) {
        case Type::Null: o << "null"; return;
        case Type::Bool: o << (v.asBool() ? "true" : "false"); return;
        case Type::Number: writeNumber(o, v.asNumber()); return;
        case Type::String: writeString(o, v.asString()); return;
        case Type::Array: writeArray(o, v.asArray(), depth + 1); return;
        case Type::Object: writeObject(o, v.asObject(), depth + 1); return;
      }
    }
  } // namespace

  std::string write(const Value& v) {
    std::ostringstream o;
    writeValue(o, v, 0);
    o << "\n";
    return o.str();
  }

} // namespace json.
