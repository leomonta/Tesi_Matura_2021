#undef UNICODE

// system headers
#define WIN32_LEAN_AND_MEAN
#include <array>
#include <direct.h> // needed to get the current directory 
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>

// my headers
#include "DB_conn.hpp"

#ifdef _DEBUG
const char* server_init_file = "C:/Users/Leonardo/Desktop/workspace/vs-c++/TESI_MONTAGNER_MATURA/server_options.ini";
#define DBG_FUNC //
#else
const char* server_init_file = "./server_options.ini";
#define DBG_FUNC //
#endif

// Need to link with lws2_32 aka Ws2_32.lib
#define DEFAULT_BUFLEN 1024

// connection variable
WSADATA wsaData;
int iResult;

SOCKET ListenSocket = INVALID_SOCKET;
SOCKET ClientSocket = INVALID_SOCKET;

struct addrinfo* result = NULL;
struct addrinfo hints;

int iSendResult;
char recvbuf[DEFAULT_BUFLEN];
int recvbuflen = DEFAULT_BUFLEN;

// variables for initialization
// http
std::string HTTP_Basedir = ".";
std::string HTTP_LocalIP = "127.0.0.1";
std::string HTTP_Default_IP = "0.0.0.0";
std::string HTTP_Port = "80";
// Database
std::string DB_Port = "3306";
std::string DB_Host = "localhost";
std::string DB_Username = "root";
std::string DB_Password = "password";
std::string DB_Name = "";


void checkError(std::string type);
void init();
void resolve(std::string ip);
void genListenSock();
void setupListenSock();
void acceptClientSock();
std::string compileHeader(std::vector<std::array<std::string, 2>> hOptions);
void compileMessage(const char* request, std::string* message, std::string* header);
void urlDecode(char* dst, const char* src);
std::vector<std::string> split(const std::string source, const std::string find);
std::string getContentType(std::string* filetype);

int __cdecl main() {

	// initialize winsock and the server options
	init();

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	resolve(HTTP_LocalIP);

	// Create a SOCKET for connecting to server
	genListenSock();

	// Setup the TCP listening socket
	setupListenSock();

	std::cout << "Server now listenig on " << HTTP_LocalIP << ":" << HTTP_Port << std::endl;
	std::cout << "On folder              " << HTTP_Basedir << "/" << std::endl;

	// Accept a client socket
	acceptClientSock();

	// No longer need server socket
	// closesocket(ListenSocket);

	// Receive until the peer shuts down the connection
	while (true) {

		// recv return number of bytes received, if 0 it received nothing
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);

		if (iResult > 0) {

			HANDLE  hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

			SetConsoleTextAttribute(hConsole, 10);
			std::cout << "\nREQUEST ARRIVED---------------------------------------------------------------------------------------------------------------------" << std::endl;

			SetConsoleTextAttribute(hConsole, 2);
			std::cout << "\nHeader received, length in bytes: " << iResult << std::endl;
			std::cout << recvbuf << std::endl;

			SetConsoleTextAttribute(hConsole, 12);
			std::cout << "\nHEADER SENT**************************************************************************" << std::endl;

			std::string message;
			std::string header;

			compileMessage(recvbuf, &message, &header);


			// acknowledge the segment back to the sender
			iSendResult = send(ClientSocket, message.c_str(), (int) (message.length()), 0);

			if (iSendResult == SOCKET_ERROR) {
				std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}

			SetConsoleTextAttribute(hConsole, 4);
			std::cout << "Bytes sent: " << iSendResult << std::endl;

			std::cout << header.c_str() << std::endl;

			SetConsoleTextAttribute(hConsole, 6);
			std::cout << "\nREQUEST SATISFIED////////////////////////////////////////////////////////////////////\n\n\n\n\n" << std::endl;
			shutdown(ClientSocket, SD_SEND);

			// std::cout << "Wait for further communications" << std::endl;
			acceptClientSock();
		}

		if (iResult < 0) {
			//printf("recv paused : %d\n", WSAGetLastError());
			//closesocket(ClientSocket);
			//WSACleanup();
			//return 1;
			iResult = 0;
			std::cout << "Error! Keep on listening" << WSAGetLastError();

			iResult = shutdown(ClientSocket, SD_SEND);
			ClientSocket = accept(ListenSocket, NULL, NULL);
		}
	}

	if (iResult == SOCKET_ERROR) {
		std::cout << "shutdown failed with error: " << WSAGetLastError() << std::endl;
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}

void init() {
	// init WSA for networking
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	checkError("WSAstartup");

	// retrive initilization data from the .ini file

	//char buffer[MAX_PATH] = {0};

	// this gives me the path to where the server was executed, for some reasons it also returns the pointer i gave it
	//_getcwd(buffer, MAX_PATH);
	//std::cout << buffer << std::endl;

	// compile the full path to the ini file
	//std::string ini_file = buffer;
	//ini_file += "\\";
	//ini_file += server_init_file;

	std::string buf;
	std::fstream Read(server_init_file, std::ios::in);;
	std::vector<std::string> key_val;

	// read props from the ini file and sets the important variables
	if (Read.is_open()) {
		while (std::getline(Read, buf)) {
			key_val = split(buf, "=");

			if (key_val.size() > 1) {
				// sectioon [HTTP]
				if (key_val[0] == "IP") {
					HTTP_LocalIP = key_val[1];
				} else if (key_val[0] == "HTTP_Port") {
					HTTP_Port = key_val[1];
				} else if (key_val[0] == "Base_Directory") {
					HTTP_Basedir = key_val[1];
				} else if (key_val[0] == "Default_IP") {
					HTTP_Default_IP = key_val[1];
				}
				// section [Database]
				else if (key_val[0] == "DB_Port") {
					DB_Port = key_val[1];
				} else if (key_val[0] == "host") {
					DB_Host = key_val[1];
				} else if (key_val[0] == "password") {
					DB_Password = key_val[1];
				} else if (key_val[0] == "username") {
					DB_Username = key_val[1];
				} else if (key_val[0] == "Database_Name") {
					DB_Name = key_val[1];
				}
			}
		}

		Read.close();
	}

}

void resolve(std::string ip) {
	iResult = getaddrinfo(ip.c_str(), HTTP_Port.c_str(), &hints, &result);
	checkError("getaddrinfo");
}

void genListenSock() {
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	checkError("socket");
}

void setupListenSock() {

	iResult = bind(ListenSocket, result->ai_addr, (int) result->ai_addrlen);
	checkError("bind");

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	checkError("listen");
}

void acceptClientSock() {
	ClientSocket = accept(ListenSocket, NULL, NULL);
}

std::string compileHeader(std::vector<std::array<std::string, 2>> hOptions) {

	std::string res;

	for (size_t i = 0; i < hOptions.size(); i++) {
		res += hOptions[i][0] + ": " + hOptions[i][1] + "\n";
	}
	return res;
}

/**
* Compose Header, read the requested file, compile the final message and set the given message as it
*/
void compileMessage(const char* request, std::string* message, std::string* header = nullptr) {

	std::string Srequest = request;

	// find the file requested
	// GET ****** HTTP/ i need to get the (***) of unkown lenght, i skip GET and take a subtring from index 4 to index ?

	// need the end index of the file requested
	size_t endIndex = Srequest.find("HTTP/");

	// actually get the filename with the heading slash (/), -5 cus std::string::find return the index of the last char of the string given

	std::string file = Srequest.substr(4, endIndex - 5);

	char* dst = (char*) (file.c_str());

	urlDecode(dst, file.c_str());

	file = dst;

	file = HTTP_Basedir + file;
	if (file.back() == '/') {
		file += "index.html";
	}

	std::cout << "File requested: " << file << std::endl;

	// Start Compiling Header

	std::vector<std::array<std::string, 2>> hOptions;

	// get the required file
	std::fstream ifs(file.c_str(), std::ios::binary | std::ios::in);

	std::string content((std::istreambuf_iterator<char>(ifs)),
						(std::istreambuf_iterator<char>()));

	if (!content.empty()) {
		hOptions.insert(hOptions.end(), {"HTTP/1.1", "200 OK"});
		std::string temp = split(file, ".").back();

		std::array<std::string, 2> content_type = {"Content-Type", getContentType(&temp)};
		if (content_type[1] == "") {
			content_type[1] = "text/plain";
		}

		hOptions.insert(hOptions.end(), content_type);

	} else {
		hOptions.insert(hOptions.end(), {"HTTP/1.1", "404 Not Found"});
		hOptions.insert(hOptions.end(), {"Content-Type", "text/html; charset=UTF-8"});
		// get the missing page file
		std::fstream ifs("source/404.html", std::ios::binary | std::ios::in);

		content = std::string((std::istreambuf_iterator<char>(ifs)),
							  (std::istreambuf_iterator<char>()));
	}

	hOptions.insert(hOptions.end(), {"Connection", "close"});
	hOptions.insert(hOptions.end(), {"Content-Lenght", std::to_string(content.length())});
	hOptions.insert(hOptions.end(), {"Server", "LeoCustom"});

	std::string Head = compileHeader(hOptions);

	std::cout << Head << std::endl;

	std::string SendString = Head + "\n" + content;

	if (header != nullptr) {
		*header = Head;
	}
	*message = SendString;
}

// Thank you ThomasH, https://stackoverflow.com/users/2012498/thomash at https://stackoverflow.com/questions/2673207/c-c-url-decode-library/2766963,
void urlDecode(char* dst, const char* src) {
	char a, b;
	while (*src) {
		if ((*src == '%') &&
			((a = src[1]) && (b = src[2])) &&
			(isxdigit(a) && isxdigit(b))) {
			if (a >= 'a')
				a -= 'a' - 'A';
			if (a >= 'A')
				a -= ('A' - 10);
			else
				a -= '0';
			if (b >= 'a')
				b -= 'a' - 'A';
			if (b >= 'A')
				b -= ('A' - 10);
			else
				b -= '0';
			*dst++ = 16 * a + b;
			src += 3;
		} else if (*src == '+') {
			*dst++ = ' ';
			src++;
		} else {
			*dst++ = *src++;
		}
	}
	*dst++ = '\0';
}

/**
* Split the given string with a single token, and return the vector of the splitted strings
*/
std::vector<std::string> split(const std::string source, const std::string find) {
	std::vector<std::string> res;
	std::string haystack(source);

	size_t pos = 0;
	std::string token;
	while ((pos = haystack.find(find)) != std::string::npos) {
		token = haystack.substr(0, pos);
		res.insert(res.end(), token);
		haystack.erase(0, pos + find.length());
	}
	res.insert(res.end(), haystack);

	return res;
}

/**
* just print the error with a little formatting
*/
void checkError(std::string type) {

	if (iResult != 0) {
		std::cout << type << " failed with error: " << iResult << " " << WSAGetLastError() << std::endl;
		return;
	}
}

/**
* Query the database for the correct file type
*/
std::string getContentType(std::string* filetype) {

	// create connection
	std::string host("tcp://" + DB_Host + ":" + DB_Port);
	sql::SQLString l_host(host.c_str());
	sql::SQLString l_uname(DB_Username.c_str());
	sql::SQLString l_pwd(DB_Password.c_str());
	sql::SQLString l_name(DB_Name.c_str());

	Database_connection conn(&l_host, &l_uname, &l_pwd, &l_name);

	// build query
	std::string query = "SELECT * FROM types WHERE type LIKE '";
	query += (*filetype).c_str();
	query += "'";

	// execute query
	sql::ResultSet* res;
	sql::SQLString t = query.c_str();
	res = conn.Query(&t);


	// i only get the first result
	std::string result;
	if (res->next()) {
		result = res->getString("content");
	}
	return result;

}