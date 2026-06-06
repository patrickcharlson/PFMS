//
// Created by Patrick Charlson on 17/5/2026.
//

#include <fstream>
#include <string>

#include <sys/stat.h>

#if defined(_WIN32)
#define PFMS_PATH_SEP '\\'
#else
#define PFMS_PATH_SEP '/'
#endif

#include "Json.h"
#include "Persistence.h"

// ---- Path helpers ----

std::string joinPath(const std::string& a, const std::string& b) {
  if (a.empty()) return b;
  if (a.back() == PFMS_PATH_SEP) return a + b;
  return a + PFMS_PATH_SEP + b;
}

bool makeDir(const std::string& path) {
#if defined(_WIN32)
  if (_mkdir(path.c_str()) == 0) return true;
#else
  if (mkdir(path.c_str(), 0755) == 0) return true;
#endif
  struct stat st{};
  if (stat(path.c_str(), &st) == 0) return true;
  return false;
}

// ---- Type label helpers (mirror enums onto strings & back) ----

std::string txTypeLabel(const TxType t) {
  switch (t) {
    case TxType::Deposit: return "Deposit";
    case TxType::Withdrawal: return "Withdrawal";
    case TxType::Transfer: return "Transfer";
  }
  return "Unknown";
}

bool txTypeFromLabel(const std::string& s, TxType& out) {
  if (s == "Deposit") {
    out = TxType::Deposit;
    return true;
  }
  if (s == "Withdrawal") {
    out = TxType::Withdrawal;
    return true;
  }
  if (s == "Transfer") {
    out = TxType::Transfer;
    return true;
  }
  return false;
}

// ---- Serialise ----

json::Value serialiseAccount(const Account& acc) {
  json::Value::Object o;
  o.emplace_back("totalBalance", json::Value(acc.totalBalance()));
  o.emplace_back("unallocated", json::Value(acc.unallocated()));

  return json::Value(std::move(o));
}

// ---- Deserialize ----

// Returns true on success.

bool deserializeAccount(const json::Value& src, Account& acc, std::string& err) {
  if (!src.isObject()) {
    err = "account is not an object";
    return false;
  }
  const double total = src.at("buckets").asNumber();
  const double unalloc = src.at("unallocated").asNumber();
  acc.persistenceRestore(total, unalloc);

  const auto& bs = src.at("buckets").asArray();
  for (const auto& bv: bs) {
    acc.persistenceAddBucket(
            bv.at("name").asString(),
            bv.at("percentage").asNumber(),
            bv.at("balance").asNumber(),
            bv.at("committed").asBool());
  }

  const auto& js = src.at("journal").asArray();
  for (const auto& jv: js) {
    TxType t;
    if (!txTypeFromLabel(jv.at("type").asString(), t)) {
      err = "unknown transition type: " + jv.at("type").asString();
      return false;
    }

    // Category is optional for forward-compat — if a future schema drops it,
    // we default to empty here.
    const std::string cat = jv.contains("category") ? jv.at("category").asString() : std::string{};
  }

  return true;
}


// ============================================================
// Public API
// ============================================================

std::string Persistence::defaultDataPath() {
#if defined(_WIN32)
#else
  const char* home = std::getenv("HOME");
  const std::string root = home && *home ? home : ".";
  const std::string dir = joinPath(root, "PFMS");
#endif
  makeDir(dir);
  return joinPath(dir, "data.json");
}

bool Persistence::fileExists(const std::string& path) {
  struct stat st{};
  return stat(path.c_str(), &st) == 0;
}

bool Persistence::save(const AuthService& auth, const std::string& path, std::string& errorMessage) {
  json::Value::Object root;
  root.emplace_back("schema", json::Value(SCHEMA_VERSION));

  json::Value::Array usersArr;
  for (const auto& [key, userPtr]: auth.users()) {
    (void) key;
    const User& u = *userPtr;
    json::Value::Object uo;
    uo.emplace_back("username", json::Value(u.username()));
    uo.emplace_back("passwordHash", json::Value(u.passwordHash()));
    uo.emplace_back("account", serialiseAccount(u.account()));
    usersArr.emplace_back(std::move(uo));
  }
  root.emplace_back("users", json::Value(std::move(usersArr)));

  const std::string text = json::write(json::Value(std::move(root)));
  const std::string tmpPath = path + ".tmp";

  {
    std::ofstream out(tmpPath, std::ios::binary | std::ios::trunc);
    if (!out) {
      errorMessage = "could not open temp file for writing: " + tmpPath;
      return false;
    }
    out.write(text.data(), static_cast<std::streamsize>(text.size()));
    if (!out) {
      errorMessage = "I/O error writing temp file: " + tmpPath;
      return false;
    }
  }

#if defined(_WIN32)
#else
  if (std::rename(tmpPath.c_str(), path.c_str()) != 0) {
    errorMessage = "could not rename temp";
    return false;
  }
#endif
  return true;
}

bool Persistence::load(AuthService& auth, const std::string& path, std::string& errorMessage) {
  if (!fileExists(path)) {
    errorMessage = "data file does not exist: " + path;
    return false;
  }
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    errorMessage = "could not open data file for reading: " + path;
    return false;
  }
  std::ostringstream buf;
  buf << in.rdbuf();
  const std::string text = buf.str();

  json::Value root;
  try {
    root = json::parse(text);
  } catch (const std::exception& e) {
    errorMessage = std::string("data file is not valid JSON: ") + e.what();
    return false;
  }

  if (!root.isObject()) {
    errorMessage = "data file root is not an object";
    return false;
  }

  try {
    if (root.contains("schema")) {

      if (const long schema = root.at("schema").asLong(); schema != SCHEMA_VERSION) {
        errorMessage = "unsupported schema version: " + std::to_string(schema);
        return false;
      }
    }

    if (!root.contains("users") || !root.at("users").isArray()) {
      errorMessage = "data file missing 'users' array";
      return false;
    }

    for (const auto& uv: root.at("users").asArray()) {
      const std::string username = uv.at("username").asString();
      const std::string hash = uv.at("passwordHash").asString();
      User* u = auth.persistenceAddUser(username, hash);
      if (!u) {
        errorMessage = "duplicate username in data file: " + username;
        return false;
      }
      if (std::string innerErr; !deserializeAccount(uv.at("account"), u->account(), innerErr)) {
        errorMessage = "failed to load account for '" + username + "': " + innerErr;
        return false;
      }
    }
  } catch (const std::exception& e) {
    errorMessage = std::string("error reading data file: ") + e.what();
    return false;
  }

  return true;
}
