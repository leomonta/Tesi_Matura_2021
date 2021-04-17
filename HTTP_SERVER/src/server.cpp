// my headers
#include "DB_conn.hpp"
#include "HTTP_conn.hpp"

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

		// receiveRequest return number of bytes received, if 0 it received nothing
		iResult = http.receiveRequest(&client, &request);

		if (iResult > 0) {

			HANDLE  hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

			SetConsoleTextAttribute(hConsole, 10);
			std::cout << "\nREQUEST ARRIVED---------------------------------------------------------------------------------------------------------------------" << std::endl;

			SetConsoleTextAttribute(hConsole, 2);
			std::cout << "\nHeader received, length in bytes: " << iResult << std::endl;
			std::cout << request.c_str() << std::endl;

			SetConsoleTextAttribute(hConsole, 12);
			std::cout << "\nHEADER SENT**************************************************************************" << std::endl;

			std::string message;
			std::string header;

			http.compileMessage(request.c_str(), &message, &header);


			// acknowledge the segment back to the sender
			iSendResult = http.sendResponse(&client, &message);
			//iSendResult = 

			if (iSendResult == SOCKET_ERROR) {
				std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
				http.closeClientSock(client);
				WSACleanup();
				return 1;
			}

			SetConsoleTextAttribute(hConsole, 4);
			std::cout << "Bytes sent: " << iSendResult << std::endl;

			std::cout << header.c_str() << std::endl;

			SetConsoleTextAttribute(hConsole, 6);
			std::cout << "\nREQUEST SATISFIED////////////////////////////////////////////////////////////////////\n\n\n\n\n" << std::endl;
			shutdown(client, SD_SEND);

			// std::cout << "Wait for further communications" << std::endl;
			client = http.acceptClientSock();
		}

		if (iResult < 0) {
			//printf("recv paused : %d\n", WSAGetLastError());
			//closesocket(ClientSocket);
			//WSACleanup();
			//return 1;
			iResult = 0;
			std::cout << "Error! Cannot keep on listening" << WSAGetLastError();

			iResult = shutdown(client, SD_SEND);
			return -1;
		}
	}

	if (iResult == SOCKET_ERROR) {
		std::cout << "shutdown failed with error: " << WSAGetLastError() << std::endl;
		http.closeClientSock(client);
		WSACleanup();
		return 1;
	}

	// cleanup
	http.closeClientSock(client);
	WSACleanup();

	return 0;
}

