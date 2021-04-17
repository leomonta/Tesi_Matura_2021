#include "DB_conn.hpp"

Database_connection::Database_connection(sql::SQLString* host, sql::SQLString* username, sql::SQLString* password, sql::SQLString* DBName) {
	connect(host, username, password, DBName);
}

/**
* Enstablish a connection with the database andset the database
*/
void Database_connection::connect(sql::SQLString* host, sql::SQLString* username, sql::SQLString* password, sql::SQLString* DBName) {

	try {
		driver = get_driver_instance();
		// enstablish connection
		connection = driver->connect(*host, *username, *password);
		// use the given database
		connection->setSchema(*DBName);
	} catch (sql::SQLException e) {
		std::cout << e.what() << " // " << e.getErrorCode() << " // " << e.getSQLState() << " // " << std::endl;
	}
}

/**
* Execute a query with a result (SELECT) and return it
*/
sql::ResultSet* Database_connection::Query(sql::SQLString* query) {

	sql::ResultSet* res;
	sql::Statement* stmt;

	stmt = connection->createStatement();
	res = stmt->executeQuery(*query);
	delete stmt;
	return res;
}

/**
* Execute any query and return if it was successful
*/
bool Database_connection::UQuery(sql::SQLString* query) {

	sql::Statement* stmt;

	stmt = connection->createStatement();
	bool res = stmt->execute(*query);
	delete stmt;

	return res;
}