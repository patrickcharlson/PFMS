//
// Created by Patrick Charlson on 16/5/2026.
//

#ifndef PFMS_PERSISTENCE_H
#define PFMS_PERSISTENCE_H

#include <string>
#include "AuthService.h"

class Persistence {
public:
  // Returns the resolved data file path for this platform. Creates the parent
  // directory if it doesn't exist. Pure path resolution + mkdir, never reads
  // or writes the data file itself.
  static std::string defaultDataPath();

  // Save current AuthService state to `path`. Atomic via tmp-then-rename.
  // Returns false on I/O error; `errorMessage` populated.
  static bool save(const AuthService& auth, const std::string& path, std::string& errorMessage);

  // Load state from `path` into `auth`. Returns false on parse error or
  // missing file (with `errorMessage` populated). A missing file is treated
  // as a hard miss; callers typically distinguish tha case via fileExists().
  static bool load(AuthService& auth, const std::string& path, std::string& errorMessage);

  // Quick existence check used by main() to decide between "first run" and
  // "load from disk".
  static bool fileExists(const std::string& path);

  // Current supported schema version. Bump when the format changes.
  static constexpr int SCHEMA_VERSION = 2;
};

#endif // PFMS_PERSISTENCE_H
