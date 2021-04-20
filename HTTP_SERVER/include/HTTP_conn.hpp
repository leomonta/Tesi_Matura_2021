#undef UNICODE

// system headers
#define WIN32_LEAN_AND_MEAN
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>

#include "zlib.h"

#define DEFAULT_BUFLEN 8000

class HTTP_conn {
private:

	WSADATA wsaData;
	int iResult;

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
	std::string DB_Port = "3306";
	std::string DB_Host = "localhost";
	std::string DB_Username = "root";
	std::string DB_Password = "password";
	std::string DB_Name = "";

	/**
	* just print the error with a little formatting
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
	std::string getContentType(std::string* filetype);
	std::vector<std::string> split(const std::string source, const std::string find);
	void compressGz(std::string& output, const char* data, std::size_t size);

public:

	HTTP_conn();
	void create();
	void compileHeader(std::map<std::string, std::string>* headerOptions, std::string* result);
	void compileMessage(const char* request, std::string* message, std::string* header);
	int receiveRequest(SOCKET* clientSock, std::string* buff);
	int sendResponse(SOCKET* clientSock, std::string* buff);
	SOCKET acceptClientSock();
	void closeClientSock(SOCKET* clientSock);
	void shutDown(SOCKET* clientSock);

};