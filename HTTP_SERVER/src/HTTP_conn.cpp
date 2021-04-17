#include "HTTP_conn.hpp"
#include "DB_conn.hpp"

#ifdef _DEBUG
const char* server_init_file = "C:/Users/Leonardo/Desktop/workspace/vs-c++/TESI_MONTAGNER_MATURA/server_options.ini";
#define DEBUG_FUNC
#else
const char* server_init_file = "./server_options.ini";
#define DEBUG_FUNC //
#endif

/**
* Decode url character (%20 => " ") to ascii character, NOT MINE, JUST COPY PASTED
* Thank you ThomasH, https://stackoverflow.com/users/2012498/thomash at https://stackoverflow.com/questions/2673207/c-c-url-decode-library/2766963,
*/
void HTTP_conn::urlDecode(char* dst, const char* src) {
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
std::vector<std::string> HTTP_conn::split(const std::string source, const std::string find) {
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
* initialize WSAscoket and connections vars
*/
void HTTP_conn::init() {
	// init WSA for networking
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	checkError("WSAstartup");

	// retrive initilization data from the .ini file

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

/**
* Resolve the given Ip and compute the options for addressinfo result
*/
void HTTP_conn::setupListenSock() {

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
	std::cout << "On folder              " << HTTP_Basedir << "/" << std::endl;

	// Now the server is ready to accept any client with this socket
}

/**
* Shortcut to accept any client socket, to be used multiple time by thread
*/
SOCKET HTTP_conn::acceptClientSock() {
	sockaddr client_addr;
	client_addr.sa_family = AF_INET;
	int claddr_size = sizeof(client_addr);
	SOCKET result = accept(ListenSocket, &client_addr, &claddr_size);

	if (result != INVALID_SOCKET) {
		struct sockaddr_in* temp = (struct sockaddr_in*) &client_addr;
		char* s = (char*) malloc(INET_ADDRSTRLEN);

		if (s != nullptr) {
			inet_ntop(AF_INET, &(temp->sin_addr), s, INET_ADDRSTRLEN);
			std::cout << "accepted client : " << s << std::endl;
		}
	}
	return result;
}

/**
* Shortcup to close the current client socket
*/
void HTTP_conn::closeClientSock(SOCKET clientSock) {
	closesocket(clientSock);
}

HTTP_conn::HTTP_conn() {
	create();
}

void HTTP_conn::create() {
	init();

	setupListenSock();
}

/**
* format the header from the array to a string
*/
void HTTP_conn::compileHeader(std::vector<std::array<std::string, 2>>* headerOptions, std::string* result) {
	std::string temp_result;

	for (size_t i = 0; i < (*headerOptions).size(); i++) {
		//				header option name		 :		 header option value
		temp_result += (*headerOptions)[i][0] + ": " + (*headerOptions)[i][1] + "\n";
	}

	// write the result on the given string
	*result = temp_result;
}

/**
* Compose Header, read the requested file, compile the final message and set the given message as it
*/
void HTTP_conn::compileMessage(const char* request, std::string* message, std::string* header = nullptr) {

	// i use a string for compatibility and comodity
	std::string Srequest = request;

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

	DEBUG_FUNC std::cout << "File requested: " << file << std::endl;

	// ------------------------------------------------------------------------------------------------ Start Compiling Header

	// i use an array instead if a map cus a map(even if unordered) has its own internal order and i need the 
	// response code to ALWAYS be fisrt obviously, and i can achieve this by using hOptions.begin()
	std::vector<std::array<std::string, 2>> hOptions;

	// get the required file
	std::fstream ifs(file.c_str(), std::ios::binary | std::ios::in);

	// read the file in one go to a string
	//							start iterator							end iterator
	std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

	// if the file does not exist i get an empty string
	if (!content.empty()) {

		// reqeusted file exist

		// status code OK
		hOptions.insert(hOptions.begin(), {"HTTP/1.1", "200 OK"});

		// get the file extension, i'll use it to get the content type
		std::string temp = split(file, ".").back();

		// get the content type
		std::array<std::string, 2> content_type = {"Content-Type", getContentType(&temp)};
		if (content_type[1] == "") {
			content_type[1] = "text/plain";
		}

		hOptions.insert(hOptions.end(), content_type);

	} else {
		hOptions.insert(hOptions.begin(), {"HTTP/1.1", "404 Not Found"});
		hOptions.insert(hOptions.end(), {"Content-Type", "text/html; charset=UTF-8"});
		// get the missing page file
		std::fstream ifs("source/404.html", std::ios::binary | std::ios::in);

		content = std::string((std::istreambuf_iterator<char>(ifs)),
							  (std::istreambuf_iterator<char>()));
	}

	hOptions.insert(hOptions.end(), {"Connection", "close"});
	hOptions.insert(hOptions.end(), {"Content-Lenght", std::to_string(content.length())});
	hOptions.insert(hOptions.end(), {"Server", "LeoCustom"});

	std::string Head;
	compileHeader(&hOptions, &Head);

	DEBUG_FUNC std::cout << Head << std::endl;

	std::string SendString = Head + "\n" + content;

	if (header != nullptr) {
		*header = Head;
	}
	*message = SendString;
}

/**
* Query the database for the correct file type
*/
std::string HTTP_conn::getContentType(std::string* filetype) {

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
	} else {
		result = "text/plain";
	}

	return result;

}

/**
* copy the incoming message requested to buff and return the bytes received
*/
int HTTP_conn::receiveRequest(SOCKET* clientSock, std::string* buff) {

	char recvbuf[DEFAULT_BUFLEN];
	// result is the amount of bytes received
	int result = recv(*clientSock, recvbuf, DEFAULT_BUFLEN, 0);
	*buff = std::string(recvbuf, result);
	return result;
}

/**
* Send the buffer (buff) to the client, and return the bytes sent
*/
int HTTP_conn::sendResponse(SOCKET* clientSock, std::string* buff) {

	int size = (int) (*buff).length();
	int result = send(*clientSock, (*buff).c_str(), size, 0);

	return result;
}
