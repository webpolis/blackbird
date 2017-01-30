#include "parameters.h"
#include "sqlite3.h"
#include <iostream>
#include <sstream>
#include <memory>


// define some helper overloads to ease sqlite resource management
using scoped_sqlite3err = std::unique_ptr<char, decltype(&sqlite3_free)>;
static sqlite3* sqlite3_open(const char *filename, int &err)
{
  sqlite3 *db;
  err = sqlite3_open(filename, &db);
  return db;
}

typedef int (*sqlite3_callback)(void*, int, char**, char**);
static char* sqlite3_exec(sqlite3 *S, const char *sql, sqlite3_callback cb, void *udata, int &err)
{
  char *errmsg;
  err = sqlite3_exec(S, sql, cb, udata, &errmsg);
  return errmsg;
}


int createDbConnection(Parameters& params)
{
  int res;
  params.dbConn.reset( sqlite3_open(params.dbFile.c_str(), res) );
  
  if (res != SQLITE_OK)
    std::cerr << sqlite3_errmsg(params.dbConn.get()) << std::endl;

  return res;
}

int createTable(std::string exchangeName, Parameters& params)
{
  std::stringstream query;
  query << "CREATE TABLE IF NOT EXISTS `" << exchangeName << "` (Datetime DATETIME NOT NULL, bid DECIMAL(8, 2), ask DECIMAL(8, 2));";
  int res;
  scoped_sqlite3err errmsg(sqlite3_exec(params.dbConn.get(), query.str().c_str(), nullptr, nullptr, res),
                           sqlite3_free);
  if (res != SQLITE_OK)
    std::cerr << errmsg.get() << std::endl;

  return res;
}

int addBidAskToDb(std::string exchangeName, std::string datetime, double bid, double ask, Parameters& params) {
  std::stringstream query;
  query << "INSERT INTO `" << exchangeName << "` VALUES ('" << datetime << "'," << bid << "," << ask << ");";
  int res;
  scoped_sqlite3err errmsg(sqlite3_exec(params.dbConn.get(), query.str().c_str(), nullptr, nullptr, res),
                           sqlite3_free);
  if (res != SQLITE_OK)
    std::cerr << errmsg.get() << std::endl;

  return res;
}
