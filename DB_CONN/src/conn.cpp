
#include <stdio.h>
#include <iostream>
#include <stdlib.h>

/*
  Include directly the different
  headers from cppconn/ and mysql_driver.h + mysql_util.h
  (and mysql_connection.h). This will reduce your build time!
*/

#define CPPCONN_PUBLIC_FUNC

#include "jdbc/mysql_connection.h"

#include <jdbc/cppconn/driver.h>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>

using namespace std;

int main(void) {
	cout << endl;
	cout << "Running 'SELECT 'Hello World!' > AS _message'..." << endl;

	try {
		sql::Driver* driver;
		sql::Connection* con;
		sql::Statement* stmt;
		sql::ResultSet* res;

		/* Create a connection */
		driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1:3306", "root", "");
		/* Connect to the MySQL test database */
		con->setSchema("mimetypes");

		stmt = con->createStatement();
		res = stmt->executeQuery("SELECT * FROM types WHERE content LIKE 'image/%'");
		while (res->next()) {
			/* Access column data by alias or column name */
			printf("%-30s -> %s \n", res->getString("type").c_str(), res->getString("content").c_str());
			/* Access column data by numeric offset, 1 is the first column */
		}
		delete res;
		delete stmt;
		delete con;

	} catch (sql::SQLException& e) {
		cout << "# ERR: SQLException in " << __FILE__;
		cout << "(" << __FUNCTION__ << ") on line > "
			<< __LINE__ << endl;
		cout << "# ERR: " << e.what();
		cout << " (MySQL error code: " << e.getErrorCode();
		cout << ", SQLState: " << e.getSQLState() << " )" << endl;
	}

	cout << endl;

	return EXIT_SUCCESS;
}