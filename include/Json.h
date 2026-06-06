//
// Created by Patrick Charlson on 17/5/2026.
//

#ifndef PFMS_JSON_H
#define PFMS_JSON_H

#include <sstream>
#include <vector>

namespace json {
  enum class Type { Null, Bool, Number, String, Array, Object };

  class Value {
  public:
    using Array = std::vector<Value>;
    using Object = std::vector<std::pair<std::string, Value>>;

    Value() : type_(Type::Null) {}
    explicit Value(const bool b) : type_(Type::Bool), bool_(b) {}
    explicit Value(const int n) : type_(Type::Number), num_(static_cast<double>(n)) {}
    explicit Value(const double n) : type_(Type::Number), num_(n) {}
    explicit Value(const char* s) : type_(Type::String), str_(s) {}
    explicit Value(std::string s) : type_(Type::String), str_(std::move(s)) {}
    explicit Value(Array a) : type_(Type::Array), arr_(std::move(a)) {}
    explicit Value(Object o) : type_(Type::Object), obj_(std::move(o)) {}

    Type type() const { return type_; }
    bool isObject() const { return type_ == Type::Object; }
    bool isArray() const { return type_ == Type::Array; }


    // clang-format off
    bool asBool() const { check(Type::Bool); return bool_; }
    double asNumber() const { check(Type::Number); return num_; }
    long asLong() const { check(Type::Number); return static_cast<long>(num_); }
    const std::string& asString() const { check(Type::String); return str_; }
    const Array& asArray() const { check(Type::Array); return arr_; }
    Array& asArray() { check(Type::Array); return arr_; }
    const Object& asObject() const { check(Type::Object); return obj_; }
    Object& asObject() { check(Type::Object); return obj_; }
    // clang-format on

    const Value& at(const std::string& key) const;
    bool contains(const std::string& key) const;


  private:
    Type type_;
    bool bool_{false};
    double num_{0.0};
    std::string str_;
    Array arr_;
    Object obj_;

    void check(const Type t) const {
      if (type_ != t) throw std::runtime_error("json: type mismatch");
    }
  };

  // --- Parsing ---
  Value parse(const std::string& source);

  std::string write(const Value& v);

} // namespace json


#endif // PFMS_JSON_H
