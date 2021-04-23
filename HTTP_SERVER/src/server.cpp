#include <thread>
#include <mutex>
// my headers
#include "HTTP_conn.hpp"

/**
* Todos: HTTP METHODS: POST, HEAD, and maybe PUT, OPTIONS,
* Todos: multithreading, each accepted socket a thread
* Todos: caching, keep in memory files
*/

// active threads
// std::vector<std::thread> threads;
// for controlling debug prints
std::mutex mtx;

void resolveRequest(SOCKET clientSocket, HTTP_conn* http_);

int __cdecl main() {

	// initialize winsock and the server options
	HTTP_conn http;

	// used for controlling 
	int iResult;
	SOCKET client;
	std::string request;

	// Receive until the peer shuts down the connection
	while (true) {

		client = http.acceptClientSock();
		if (client == INVALID_SOCKET) {
			continue;
		} else {
			//resolveRequest(client, &http);
			std::thread(resolveRequest, client, &http).detach();
		}

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

void resolveRequest(SOCKET clientSocket, HTTP_conn* http_) {

	mtx.lock();
	std::cout << "thread for socket: " << clientSocket << " started" << std::endl;
	mtx.unlock();

	int iResult;
	int iSendResult;
	std::string request;
	std::string message;

	#ifdef _DEBUG
	// console color control
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	#endif

	while (true) {

		iResult = http_->receiveRequest(&clientSocket, request);
		http_->decompileHeader(request.c_str(), request.size());

		// received some bytes
		if (iResult > 0) {

			#ifdef _DEBUG
			SetConsoleTextAttribute(hConsole, 10);
			std::cout << "\nREQUEST ARRIVED---------------------------------------------------------------------------------------------------------------------" << std::endl;

			SetConsoleTextAttribute(hConsole, 2);
			std::cout << "\nByes received: " << iResult << std::endl;
			std::cout << request.c_str() << std::endl;

			SetConsoleTextAttribute(hConsole, 12);
			std::cout << "\nHEADER SENT**************************************************************************" << std::endl;
			#endif // _DEBUG

			// message body and header

			#ifdef _DEBUG
			// message compprehend both header and body
			std::cout << "thread: " << clientSocket << std::endl;
			#endif // _DEBUG
			http_->compileMessage(request.c_str(), message);

			// acknowledge the segment back to the sender
			iSendResult = http_->sendResponse(&clientSocket, &message);

			// send failed, close socket and close program
			if (iSendResult == SOCKET_ERROR) {
				std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
				http_->closeClientSock(&clientSocket);
				WSACleanup();
				break;
			}

			#ifdef _DEBUG
			SetConsoleTextAttribute(hConsole, 4);

			std::cout << "Bytes sent: " << iSendResult << std::endl;

			SetConsoleTextAttribute(hConsole, 6);
			std::cout << "\nREQUEST SATISFIED////////////////////////////////////////////////////////////////////\n\n\n\n\n" << std::endl;
			#endif // _DEBUG

			http_->shutDown(&clientSocket);
			break;
		}

		// received an error
		if (iResult < 0) {
			iResult = 0;
			std::cout << "Error! Cannot keep on listening" << WSAGetLastError();

			http_->shutDown(&clientSocket);
			break;
		}

		// nothing received, depend on the request
		if (iResult == 0) {
			break;
		}
	}

	mtx.lock();
	std::cout << "thread for socket: " << clientSocket << " finished" << std::endl;
	mtx.unlock();

}

