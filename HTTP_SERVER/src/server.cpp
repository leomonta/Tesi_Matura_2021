#include <thread>
#include <mutex>
#include <filesystem>
// my headers
#include "HTTP_conn.hpp"
#include "HTTP_message.hpp"
#include "DB_conn.hpp"
#include "utils.hpp"

#ifdef _DEBUG
const char* server_init_file = "C:/Users/Leonardo/Desktop/workspace/vs-c++/TESI_MONTAGNER_MATURA/server_options.ini";
#else
const char* server_init_file = "../server_options.ini";
#endif
/**
* Todos: HTTP METHODS: POST, HEAD, and maybe PUT, OPTIONS,
* HTTPs: using ssl to encript trasmission
*/

// Http Server
std::string HTTP_Basedir = "/";
std::string HTTP_IP = "127.0.0.1";
std::string HTTP_Port = "80";
// Database Server
sql::SQLString DB_Port = "3306";
sql::SQLString DB_Host = "localhost";
sql::SQLString DB_Username = "root";
sql::SQLString DB_Password = "password";
sql::SQLString DB_Name = "";

// for controlling debug prints
std::mutex mtx;

void readIni();
void resolveRequest(SOCKET clientSocket, HTTP_conn* http_);
void Head(HTTP_message& inbound, HTTP_message& outbound);
void Get(HTTP_message& inbound, HTTP_message& outbound);
void composeHeader(const char* filename, std::map<std::string, std::string>& result);
std::string getFile(const char* file);
void getContentType(const std::string* filetype, std::string& result);

int __cdecl main() {

	readIni();
	// initialize winsock and the server options
	HTTP_conn http(HTTP_Basedir.c_str(), HTTP_IP.c_str(), HTTP_Port.c_str());

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
	HTTP_message response;

	#ifdef _DEBUG
	// console color control
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	#endif

	while (true) {

		// ---------------------------------------------------------------------- RECEIVE
		iResult = http_->receiveRequest(&clientSocket, request);

		// received some bytes
		if (iResult > 0) {

			HTTP_message mex(request);

			switch (mex.method) {
			case HTTP_HEAD:
				Head(mex, response);
				break;

			case HTTP_GET:
				Get(mex, response);
				break;

			case HTTP_POST:
				Post(mex, response);
				break;
			}

			// make the message a single formatted string
			response.compileMessage();

			#ifdef _DEBUG
			SetConsoleTextAttribute(hConsole, 10);
			std::cout << "\nREQUEST ARRIVED---------------------------------------------------------------------------------------------------------------------" << std::endl;

			SetConsoleTextAttribute(hConsole, 2);
			std::cout << "\nByes received: " << iResult << std::endl;
			std::cout << request.c_str() << std::endl;

			SetConsoleTextAttribute(hConsole, 12);
			std::cout << "\nHEADER SENT**************************************************************************" << std::endl;

			std::cout << "thread: " << clientSocket << std::endl;
			#endif // _DEBUG

			// ------------------------------------------------------------------ SEND
			// acknowledge the segment back to the sender
			iSendResult = http_->sendResponse(&clientSocket, &response.message);

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

/**
* retrive initilization data from the .ini file
*/
void readIni() {

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
					HTTP_IP = key_val[1];
				} else if (key_val[0] == "HTTP_Port") {
					HTTP_Port = key_val[1];
				} else if (key_val[0] == "Base_Directory") {
					HTTP_Basedir = key_val[1];
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
	} else {
		std::cout << "server_options.ini file not found!\n Proceeding with default values!" << std::endl;
	}
}

/**
* the http method, set the value of result as the header that would have been sent in a GET response
*/
void Head(HTTP_message& inbound, HTTP_message& outbound) {

	char* dst = (char*) (inbound.filename.c_str());
	urlDecode(dst, inbound.filename.c_str());

	// re set the filename as the base directory and the decoded filename
	std::string file = HTTP_Basedir + dst;

	// usually to request index.html browsers does not specify it, they usually use /, if thats the case I scpecify index.html
	// back access the last char of the string
	if (file.back() == '/') {
		file += "index.html";
	}

	// insert in the outbound message the necessaire header options, filename is used to determine the response code
	composeHeader(file.c_str(), outbound.headerOptions);

	// i know that i'm loading an entire file, if i find a better solution i'll use it
	std::string content = getFile(file.c_str());
	outbound.headerOptions["Content-Lenght"] = std::to_string(content.length());
	outbound.filename = file;
}

/**
* the http method, get both the header and the body
*/
void Get(HTTP_message& inbound, HTTP_message& outbound) {

	// I just need to add the body to the head, 
	Head(inbound, outbound);

	// i know that i'm loading an entire file, if i find a better solution i'll use it
	std::string content = getFile(outbound.filename.c_str());

	std::string compressed;
	compressGz(compressed, content.c_str(), content.length());
	// set the content of the message
	outbound.rawBody = compressed;

	outbound.headerOptions["Content-Lenght"] = std::to_string(content.length());
}

/**
* compose the header given the file requested
*/
void composeHeader(const char* filename, std::map<std::string, std::string>& result) {

	// I use map to easily manage key : value, the only problem is when i compile the header, the response code must be at the top

	// if the requested file actually exist
	if (std::filesystem::exists(filename)) {

		// status code OK
		result["HTTP/1.1"] = "200 OK";

		// get the file extension, i'll use it to get the content type
		std::string temp = split(filename, ".").back();

		// get the content type
		std::string content_type;
		getContentType(&temp, content_type);

		// fallback if finds nothing
		if (content_type == "") {
			content_type = "text/plain";
		}

		// and actually add it in
		result["Content-Type"] = content_type;

	} else {
		// status code Not Found
		result["HTTP/1.1"] = "404 Not Found";
		result["Content-Type"] = "text/html";
	}

	// various header options

	result["Date"] = getUTC();
	result["Content-Encoding"] = "gzip";
	result["Connection"] = "close";
	result["Vary"] = "Accept-Encoding";
	result["Server"] = "LeonardoCustom/2.1 (Win64)";
}

/**
* get the file to a string and if its empty return the page 404.html
* https://stackoverflow.com/questions/5840148/how-can-i-get-a-files-size-in-c maybe better
*/
std::string getFile(const char* file) {

	// get the required file
	std::fstream ifs(file, std::ios::binary | std::ios::in);

	// read the file in one go to a string
	//							start iterator							end iterator
	std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));


	// if the file does not exist i load a default 404.html 
	if (content.empty()) {
		std::fstream ifs("404.html", std::ios::binary | std::ios::in);
		std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
	}

	ifs.close();

	return content;
}

/**
* Query the database for the correct file type
*/
void getContentType(const std::string* filetype, std::string& result) {
	// create connection
	sql::SQLString l_host("tcp://" + DB_Host + ":" + DB_Port);

	Database_connection conn(&l_host, &DB_Username, &DB_Password, &DB_Name);

	// build query 1, don't accept anything but the exact match

	sql::SQLString query = "SELECT * FROM types WHERE type LIKE '" + *filetype + "'";

	// execute query

	sql::ResultSet* res;
	res = conn.Query(&query);
	// i only get the first result
	if (res->next()) {
		result = res->getString("content");
	} else {
		// build query 2, anything before the type is accepted
		query = "SELECT * FROM types WHERE type LIKE '%" + *filetype + "'";

		// execute query
		res = conn.Query(&query);

		// check first result
		if (res->next()) {
			result = res->getString("content");
		} else {
			// default type
			result = "text/plain";
		}
	}

	free(res);
}
