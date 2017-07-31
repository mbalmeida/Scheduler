#ifndef PERSISTANCE_LAYER_H
#define PERSISTANCE_LAYER_H

#include <string>
#include <sqlite3.h>

/// Very simple abstractions to interact with a sqlite3
/// application .db
class PersistanceLayer {
public:
  PersistanceLayer();

  /// Connect to DataBase.
  bool Connect(const std::string& DBLoc)
  {
    int OpenRes = sqlite3_open(DBLoc.c_str(), &DB);
    if (OpenRes != SQLITE_OK) {
      LastError = sqlite3_errmsg(DB);
      return false;
    }

    return true;
  }

  bool CreateTCPConnectionsTable();
  bool CreatePhysicalMemoryTable();

  /// Check if DB is connected.
  /// Return true on success and false on error.
  bool IsConnected() {
    return DB != NULL;
  }

  /// Return last error.
  std::string GetLastError() {
    return LastError;
  }

  bool UpdateTCPConnectionTable(const std::string& Server,
                                double ConnectionTime);

  bool UpdatePhysicalMemoryTable(unsigned long PhysicalMemory);

private:
  sqlite3* DB;
  std::string LastError;
};

#endif

