#include "PersistanceLayer.h"

#include <iostream>
#include <sstream>

PersistanceLayer::PersistanceLayer() {}

bool PersistanceLayer::CreateTCPConnectionsTable() {
  const char CreateTableStr[] =
    "CREATE TABLE IF NOT EXISTS TCPConnections(Server VARCHAR,"
    "Queries INT, Min DOUBLE, Max DOUBLE, Average DOUBLE,"
    "PRIMARY KEY(Server));";

  char* ErrorMessage;
  int CreateTableRes =
    sqlite3_exec(DB, CreateTableStr, 0, 0, &ErrorMessage);

  if (CreateTableRes != SQLITE_OK) {
    sqlite3_free(ErrorMessage);
    LastError = ErrorMessage;
    return false;
  }

  return true;
}

bool PersistanceLayer::CreatePhysicalMemoryTable() {
  const char CreateTableStr[] =
    "CREATE TABLE IF NOT EXISTS PhysicalMemory("
    "Queries INT, Min INT, Max INT,"
    "Average INT);";

  char* ErrorMessage;
  int CreateTableRes =
    sqlite3_exec(DB, CreateTableStr, 0, 0, &ErrorMessage);

  if (CreateTableRes != SQLITE_OK) {
    sqlite3_free(ErrorMessage);
    LastError = ErrorMessage;
    return false;
  }

  return true;
}

bool PersistanceLayer::UpdateTCPConnectionTable(const std::string& Server,
                                                double ConnectionTime) {
  std::stringstream SelectQuery;
  SelectQuery << "SELECT * FROM TCPConnections WHERE Server='";
  SelectQuery << Server << "';";
  sqlite3_stmt* QueryStmt = 0;
  int RC =
      sqlite3_prepare_v2(DB, SelectQuery.str().c_str(), -1, &QueryStmt, 0);
  if (RC != SQLITE_OK) {
    LastError = sqlite3_errmsg(DB);
    return false;
  }
  RC = sqlite3_step(QueryStmt);
  if (RC == SQLITE_ROW) {
    // There must be a better way of updating these rows!
    // For the time being let's read the old values and update
    // them if necessary.
    int Count = sqlite3_column_int(QueryStmt, 1);
    double Min = sqlite3_column_double(QueryStmt, 2);
    double Max = sqlite3_column_double(QueryStmt, 3);
    double Avg = sqlite3_column_double(QueryStmt, 4);

    sqlite3_finalize(QueryStmt);

    // Only doing these calculations here because they are
    // trivial. More expensive computations should be moved
    // elsewhere.
    if (ConnectionTime < Min)
      Min = ConnectionTime;
    if (ConnectionTime > Max)
      Max = ConnectionTime;
    Avg = Avg * Count;
    Avg += ConnectionTime;
    Avg /= ++Count;

    // Build Update Query
    std::stringstream UpdateQuery;
    UpdateQuery << "UPDATE TCPConnections SET Queries=" << Count << ", ";
    UpdateQuery << "Min=" << Min << ", MAX=" << Max <<", Average=" << Avg << " ";
    UpdateQuery << "WHERE Server='" << Server << "';";
#if 0
    std::cout << UpdateQuery.str() << "\n";
#endif
    char* UpdateRowErrorMessage;
    int UpdateQueryRes =
        sqlite3_exec(DB, UpdateQuery.str().c_str(), 0, 0, &UpdateRowErrorMessage);
    if (UpdateQueryRes != SQLITE_OK) {
      sqlite3_free(UpdateRowErrorMessage);
      LastError = UpdateRowErrorMessage;
      return false;
    }
  }
  else {
    // No values exist for the time being let's add the first set of values.
    sqlite3_finalize(QueryStmt);
    std::stringstream InsertQuery;
    InsertQuery << "INSERT INTO TCPConnections VALUES(";
    InsertQuery << "'" << Server << "', 1, " << ConnectionTime << ", "
                << ConnectionTime << ", " << ConnectionTime << ");";
    char* ErrorMessage;
    int InsertRes =
        sqlite3_exec(DB, InsertQuery.str().c_str(), 0, 0, &ErrorMessage);
    if (InsertRes != SQLITE_OK ) {
      LastError = ErrorMessage;
      sqlite3_free(ErrorMessage);
      return false;
    }
  }
  return true;
}

bool PersistanceLayer::UpdatePhysicalMemoryTable(unsigned long PhysicalMemory) {
  std::stringstream SelectQuery;
  SelectQuery << "SELECT * FROM PhysicalMemory;";
  sqlite3_stmt* QueryStmt = 0;
  int RC =
      sqlite3_prepare_v2(DB, SelectQuery.str().c_str(), -1, &QueryStmt, 0);
  if (RC != SQLITE_OK) {
    LastError = sqlite3_errmsg(DB);
    return false;
  }
  RC = sqlite3_step(QueryStmt);
  if (RC == SQLITE_ROW) {
    // There must be a better way of updating these rows!
    // For the time being let's read the old values and update
    // them if necessary.
    int Count = sqlite3_column_int(QueryStmt, 0);
    unsigned long Min = sqlite3_column_int(QueryStmt, 1);
    unsigned long Max = sqlite3_column_int(QueryStmt, 2);
    unsigned long Avg = sqlite3_column_int(QueryStmt, 3);

    sqlite3_finalize(QueryStmt);

    // Only doing these calculations here because they are
    // trivial. More expensive computations should be moved
    // elsewhere.
    if (PhysicalMemory < Min)
      Min = PhysicalMemory;
    if (PhysicalMemory > Max)
      Max = PhysicalMemory;
    Avg = Avg * Count;
    Avg += PhysicalMemory;
    Avg /= ++Count;

    // Build Update Query
    std::stringstream UpdateQuery;
    UpdateQuery << "UPDATE PhysicalMemory SET Queries=" << Count << ", ";
    UpdateQuery << "Min=" << Min << ", MAX=" << Max <<", Average=" << Avg;
#if 0
    std::cout << UpdateQuery.str() << "\n";
#endif
    char* UpdateRowErrorMessage;
    int UpdateQueryRes =
        sqlite3_exec(DB, UpdateQuery.str().c_str(), 0, 0, &UpdateRowErrorMessage);
    if (UpdateQueryRes != SQLITE_OK) {
      sqlite3_free(UpdateRowErrorMessage);
      LastError = UpdateRowErrorMessage;
      return false;
    }
  }
  else {
    // No values exist for the time being let's add the first set of values.
    sqlite3_finalize(QueryStmt);
    std::stringstream InsertQuery;
    InsertQuery << "INSERT INTO PhysicalMemory VALUES(";
    InsertQuery << "1, " << PhysicalMemory << ", "
                << PhysicalMemory << ", " << PhysicalMemory << ");";
    char* ErrorMessage;
    int InsertRes =
        sqlite3_exec(DB, InsertQuery.str().c_str(), 0, 0, &ErrorMessage);
    if (InsertRes != SQLITE_OK ) {
      LastError = ErrorMessage;
      sqlite3_free(ErrorMessage);
      return false;
    }
  }
  return true;
}
