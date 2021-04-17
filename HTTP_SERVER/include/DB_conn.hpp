
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

/*
  Include directly the different
  headers from cppconn/ and mysql_driver.h + mysql_util.h
  (and mysql_connection.h). This will reduce build time!
*/

// #define CPPCONN_PUBLIC_FUNC
#define CPPCONN_LIB_BUILD

#include "jdbc/mysql_connection.h"

#include <jdbc/cppconn/driver.h>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>


class Database_connection {
private:
	sql::Connection* connection;
	sql::Driver* driver;
public:
	Database_connection(sql::SQLString* host, sql::SQLString* username, sql::SQLString* password, sql::SQLString* DBName);

	void connect(sql::SQLString* host, sql::SQLString* username, sql::SQLString* password, sql::SQLString* DBName);
	sql::ResultSet* Query(sql::SQLString* query);
	bool UQuery(sql::SQLString* query);
};
