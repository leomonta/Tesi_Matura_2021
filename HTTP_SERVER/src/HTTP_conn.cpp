#include "HTTP_conn.hpp"

#ifdef _DEBUG
const char* server_init_file = "C:/Users/Leonardo/Desktop/workspace/vs-c++/TESI_MONTAGNER_MATURA/server_options.ini";
#else
const char* server_init_file = "../server_options.ini";
#endif

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
void HTTP_conn::compileHeader(std::map<std::string, std::string>* headerOptions, std::string& result) {
	result;

	// Always the response code fisrt
	result += "HTTP/1.1 " + (*headerOptions)["HTTP/1.1"] + "\n";

	for (auto const& [key, val] : *headerOptions) {
		// already wrote response code
		if (key != "HTTP/1.1") {
			// header option name :	header option value
			result += key + ": " + val + "\n";
		}
	}
}

/**
* the http method, set the value of result as the header that would have been sent in a GET response
*/
void HTTP_conn::HEAD(const char* filename, std::string& result) {

	std::map<std::string, std::string> header;
	composeHeader(filename, header);

	// i know that i'm loading an entire file, if i find a better solution i'll use it
	std::string content = getFile(filename);
	header["Content-Lenght"] = std::to_string(content.length());

	compileHeader(&header, result);

}

/**
* get the file to a string and if its empty return the page 404.html
*/
std::string HTTP_conn::getFile(const char* file) {

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
* compose the header given the file requested
*/
void HTTP_conn::composeHeader(const char* filename, std::map<std::string, std::string>& result) {

	// I use map to easily manage key : value, the only problem is when i compile the header, the response must be at the top

	// requsted file actually exist
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
		result["Content-Type"] = "text/html; charset=UTF-8";
	}

	// various header options

	result["Date"] = getUTC();
	result["Content-Encoding"] = "gzip";
	result["Connection"] = "close";
	result["Vary"] = "Accept-Encoding";
	result["Server"] = "LeonardoCustom/2.1 (Win64)";
}

/**
* Compose Header, read the requested file, compile the final message and set the given message as it
*/
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
	free(dst);

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

/**
* given the raw header deconstruct it in a map with key : value
*/
std::map<std::string, std::string> HTTP_conn::decompileHeader(const char* rawHeader, size_t size) {
	std::vector<std::string> options = split(std::string(rawHeader, size), "\n");
	std::map<std::string, std::string> res;
	std::vector<std::string> temp;

	for (size_t i = 0; i < options.size(); i++) {

		temp = split(options[i], ":");
		// first header option "METHOD file HTTP/version"
		if (temp.size() < 2) {
			temp = split(options[i], " ");
		}

		// the last header option is \n
		if (temp.size() > 1) {
			res[temp[0]] = temp[1];
		}
	}

	return res;
}

/**
* Query the database for the correct file type
*/
void HTTP_conn::getContentType(const std::string* filetype, std::string& result) {
	result = "text/html";
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

/**
* Simply get the time formetted following RFC822 regulation on GMT time
*/
std::string HTTP_conn::getUTC() {

	// Get the date in UTC/GMT
	time_t unixtime;
	tm UTC;
	char buffer[80];

	// set unix time on unixtime
	time(&unixtime);
	gmtime_s(&UTC, &unixtime);

	// format based on rfc822 revision rfc1123
	strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S GMT", &UTC);

	return std::string(buffer);
}

/**
* compress data to gzip
* used this (https://github.com/mapbox/gzip-hpp/blob/master/include/gzip/compress.hpp) as a reference
*/
void HTTP_conn::compressGz(std::string& output, const char* data, std::size_t size) {

	z_stream deflate_s;
	deflate_s.zalloc = Z_NULL;
	deflate_s.zfree = Z_NULL;
	deflate_s.opaque = Z_NULL;
	deflate_s.avail_in = 0;
	deflate_s.next_in = Z_NULL;

	constexpr int window_bits = 15 + 16; // gzip with windowbits of 15

	constexpr int mem_level = 8;

	if (deflateInit2(&deflate_s, Z_BEST_COMPRESSION, Z_DEFLATED, window_bits, mem_level, Z_DEFAULT_STRATEGY) != Z_OK) {
		throw std::runtime_error("deflate init failed");
	}

	deflate_s.next_in = (Bytef*) data;
	deflate_s.avail_in = (uInt) size;

	std::size_t size_compressed = 0;
	do {
		size_t increase = size / 2 + 1024;
		if (output.size() < (size_compressed + increase)) {
			output.resize(size_compressed + increase);
		}

		deflate_s.avail_out = static_cast<unsigned int>(increase);
		deflate_s.next_out = reinterpret_cast<Bytef*>((&output[0] + size_compressed));

		deflate(&deflate_s, Z_FINISH);
		size_compressed += (increase - deflate_s.avail_out);
	} while (deflate_s.avail_out == 0);

	deflateEnd(&deflate_s);
	output.resize(size_compressed);
}
