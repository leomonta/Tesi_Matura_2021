#undef UNICODE

// system headers
#define WIN32_LEAN_AND_MEAN
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <new>
#include <string>
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>

#include "zlib.h"
#include "DB_conn.hpp"

#define DEFAULT_BUFLEN 8000

class HTTP_conn {
private:

	WSADATA wsaData;
	int iResult;

	// server listen socket
	SOCKET ListenSocket = INVALID_SOCKET;

	struct addrinfo* result = NULL;
	struct addrinfo hints;

	int iSendResult;

	// variables for initialization
	// http
	std::string HTTP_Basedir = "/";
	std::string HTTP_IP = "127.0.0.1";
	std::string HTTP_Default_IP = "0.0.0.0";
	std::string HTTP_Port = "80";
	// Database
	sql::SQLString DB_Port = "3306";
	sql::SQLString DB_Host = "localhost";
	sql::SQLString DB_Username = "root";
	sql::SQLString DB_Password = "password";
	sql::SQLString DB_Name = "";

	/**
	* just print the error Winsocket error with a little formatting
	*/
	void checkError(std::string type) {

		if (iResult != 0) {
			std::cout << type << " failed with error: " << iResult << " " << WSAGetLastError() << std::endl;
			return;
		}
	}

	void init();
	void setupListenSock();
	void urlDecode(char* dst, const char* src);
	void getContentType(const std::string* filetype, std::string& result);
	std::vector<std::string> split(const std::string source, const std::string find);
	void compressGz(std::string& output, const char* data, std::size_t size);
	void compileHeader(std::map<std::string, std::string>* headerOptions, std::string& result);
	void composeHeader(const char* filename, std::map<std::string, std::string>& result);
	std::string getFile(const char* filename);
	void composeBody();
	std::string getUTC();

public:

	HTTP_conn();
	void create();
	std::map<std::string, std::string> decompileHeader(const char* rawHeader, size_t size);
	void HEAD(const char* file, std::string& result);
	void compileMessage(const char* request, std::string& result);
	int receiveRequest(SOCKET* clientSock, std::string& result);
	int sendResponse(SOCKET* clientSock, std::string* buff);
	SOCKET acceptClientSock();
	void closeClientSock(SOCKET* clientSock);
	void shutDown(SOCKET* clientSock);

};