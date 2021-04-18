// my headers
#include "DB_conn.hpp"
#include "HTTP_conn.hpp"

/**
* Todos: HTTP METHODS: POST, HEAD, and maybe PUT, OPTIONS,
* Todos: multithreading, each accepted socket a thread
* Todos: caching, keep in memory files
*/


void resolveRequest(SOCKET* clientSocket, HTTP_conn* http_);

int __cdecl main() {

	// initialize winsock and the server options
	HTTP_conn http;

	// used for controlling 
	int iResult;
	int iSendResult;
	SOCKET client;
	std::string request;

	// Receive until the peer shuts down the connection
	while (true) {

		client = http.acceptClientSock();
		if (client == INVALID_SOCKET) {
			continue;
		}

		resolveRequest(&client, &http);

	}

	if (iResult == SOCKET_ERROR) {
		std::cout << "shutdown failed with error: " << WSAGetLastError() << std::endl;
		http.closeClientSock(&client);
		WSACleanup();
		return 1;
	}

	// cleanup
	http.closeClientSock(&client);
	WSACleanup();

	return 0;
}

void resolveRequest(SOCKET* clientSocket, HTTP_conn* http_) {

	int iResult;
	int iSendResult;
	std::string request;

	while (true) {

		iResult = http_->receiveRequest(clientSocket, &request);

		// received some bytes
		if (iResult > 0) {

			#ifdef _DEBUG
			// console color control
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

			SetConsoleTextAttribute(hConsole, 10);
			std::cout << "\nREQUEST ARRIVED---------------------------------------------------------------------------------------------------------------------" << std::endl;

			SetConsoleTextAttribute(hConsole, 2);
			std::cout << "\nHeader received" << iResult << " bytes" << std::endl;
			std::cout << request.c_str() << std::endl;

			SetConsoleTextAttribute(hConsole, 12);
			std::cout << "\nHEADER SENT**************************************************************************" << std::endl;
			#endif // _DEBUG

			// message body and header
			std::string message;
			std::string header;

			// message compprehend both header and body
			http_->compileMessage(request.c_str(), &message, &header);

			// acknowledge the segment back to the sender
			iSendResult = http_->sendResponse(clientSocket, &message);

			// send failed, close socket and close program
			if (iSendResult == SOCKET_ERROR) {
				std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
				http_->closeClientSock(clientSocket);
				WSACleanup();
				return;
			}

			#ifdef _DEBUG
			SetConsoleTextAttribute(hConsole, 4);

			std::cout << "Bytes sent: " << iSendResult << std::endl;

			// print header 
			std::cout << header.c_str() << std::endl;

			SetConsoleTextAttribute(hConsole, 6);
			std::cout << "\nREQUEST SATISFIED////////////////////////////////////////////////////////////////////\n\n\n\n\n" << std::endl;
			#endif // _DEBUG

			http_->shutDown(clientSocket);
			return;
		}

		// received an error
		if (iResult < 0) {
			iResult = 0;
			std::cout << "Error! Cannot keep on listening" << WSAGetLastError();

			http_->shutDown(clientSocket);
			return;
		}

		// nothing received, depend on the request
		//if (iResult == 0) {
		//	http.closeClientSock(client);
		// receiveRequest return number of bytes received, if 0 it received nothing
		//}
	}
}
