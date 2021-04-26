#include "HTTP_conn.hpp"
#include "utils.hpp"

/*
struct MemMan {
	size_t memAlloc = 0;
	size_t memDeAlloc = 0;

	void getMemUsage() {
		std::cout << memAlloc << " - " << memDeAlloc << " -> " << memAlloc - memDeAlloc << std::endl;
	}

	void resetMemUsage() {
		memAlloc = 0;
		memDeAlloc = 0;
	}
};

static MemMan watcher;

void* operator new(size_t size) {
	watcher.memAlloc += size;

	return malloc(size);
}

void operator delete (void* chunk, size_t size) {
	watcher.memDeAlloc += size;

	free(chunk);
}
*/


/**
* Shortcut to accept any client socket, to be used multiple time by thread
*/
SOCKET HTTP_conn::acceptClientSock() {
	sockaddr client_addr;
	client_addr.sa_family = AF_INET;
	int claddr_size = sizeof(client_addr);
	SOCKET result = accept(ListenSocket, &client_addr, &claddr_size);

	// get the ip of the host 
	if (result != INVALID_SOCKET) {
		sockaddr_in* temp = (struct sockaddr_in*) &client_addr;
		char* s = (char*) new char[INET_ADDRSTRLEN];

		if (s != nullptr) {
			inet_ntop(AF_INET, &(temp->sin_addr), s, INET_ADDRSTRLEN);
			std::cout << "\naccepted client : " << s << std::endl;
		}

		delete[] s;
		temp = nullptr;
	}

	return result;
}

/**
* Shortcup to close the current client socket
*/
void HTTP_conn::closeClientSock(SOCKET* clientSock) {

	std::string temp;
	int size;
	while (true) {
		size = receiveRequest(clientSock, temp);
		if (size <= 0) {
			break;
		}
	}

	closesocket(*clientSock);
}

/**
* close the communication from the server to the client
*/
void HTTP_conn::shutDown(SOCKET* clientSock) {
	shutdown(*clientSock, SD_SEND);
}

/**
* Setup the TCP connection on the given port and ip, fails if the port isn't on the local machine and if the port is already used
* Setup the server listening socket, the one that accept incoming client requests
*/
HTTP_conn::HTTP_conn(const char* basedir, const char* ip, const char* port) {

	// init WSA for networking
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	checkError("WSAstartup");

	// set server variables 
	HTTP_IP = ip;
	HTTP_Port = port;
	HTTP_Basedir = basedir;

	// resolve the server address full infos ad setup the listening socket

	// resolve the ip and find the appropriate address
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET; // tcp/udp
	hints.ai_socktype = SOCK_STREAM; // stream, back and forth
	hints.ai_protocol = IPPROTO_TCP; // over tcp protocol
	hints.ai_flags = AI_PASSIVE; // wait for connection

	iResult = getaddrinfo(HTTP_IP.c_str(), HTTP_Port.c_str(), &hints, &result);
	checkError("getaddrinfo");

	// create the listen socket
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	checkError("socket");

	// the binf the server address to the listen socket
	iResult = bind(ListenSocket, result->ai_addr, (int) result->ai_addrlen);
	checkError("bind");

	// no longer useful
	freeaddrinfo(result);

	// start listening
	iResult = listen(ListenSocket, SOMAXCONN);
	checkError("listen");

	// the server is ready to run
	std::cout << "Server now listenig on " << HTTP_IP << ":" << HTTP_Port << std::endl;
	std::cout << "On folder              " << HTTP_Basedir << std::endl;

	// Now the server is ready to accept any client with this socket
}

/**
* Compose Header, read the requested file, compile the final message and set the given message as it
*
void HTTP_conn::compileMessage(const char* requestFile, std::string& result) {

	// i use a string for compatibility and comodity
	std::string Srequest = requestFile;

	// find the file requested
	// GET ****** HTTP/ i need to get the (***) of unkown lenght, i skip GET and take a subtring from index 4 to index ?

	// need the end index of the file requested
	size_t endIndex = Srequest.find("HTTP/");

	// actually get the filename with the heading slash (/), -5 cus std::string::find return the index of the last char of the string given
	// the problem is that thi filename might have url character unreadable from the fstream
	std::string file = Srequest.substr(4, endIndex - 5);

	// decode the url, (i need a char*, not a const char *)
	char* dst = (char*) (file.c_str());
	urlDecode(dst, file.c_str());

	// re set the filename as the base directory and the decoded filename
	file = HTTP_Basedir + dst;

	// usually to request index.html browsers does not specify it, they usually use /, if thats the case I scpecify index.html
	// back access the last char of the string
	if (file.back() == '/') {
		file += "index.html";
	}

	std::cout << "File requested: " << file << std::endl;

	// ------------------------------------------------------------------------------------------------ Start Compiling Header

	// get the header on a map, then complete it
	std::map<std::string, std::string> headerOptions;
	composeHeader(file.c_str(), headerOptions);

	std::string body = getFile(file.c_str());
	headerOptions["Content-Lenght"] = std::to_string(body.length());

	std::string head;
	compileHeader(&headerOptions, head);

	std::cout << head << std::endl;

	std::string gz_body;
	compressGz(gz_body, body.c_str(), body.size());

	result = head + "\n" + gz_body;
}
*/
/**
* copy the incoming message requested to buff and return the bytes received
*/
int HTTP_conn::receiveRequest(SOCKET* clientSock, std::string& result) {

	char recvbuf[DEFAULT_BUFLEN];
	// result is the amount of bytes received

	int res = recv(*clientSock, recvbuf, DEFAULT_BUFLEN, 0);
	if (res > 0) {
		result = std::string(recvbuf, res);
	} else {
		result = "";
	}

	return res;
}

/**
* Send the buffer (buff) to the client, and return the bytes sent
*/
int HTTP_conn::sendResponse(SOCKET* clientSock, std::string* buff) {

	int result = send(*clientSock, buff->c_str(), (int) buff->size(), 0);
	return result;
}

