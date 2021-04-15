#undef UNICODE

#define WIN32_LEAN_AND_MEAN
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <winsock2.h>
#include <ws2tcpip.h>

// Need to link with lws2_32 aka Ws2_32.lib

#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "80"

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

// logic variable
std::string baseDir;

std::map<std::string, std::string> Contents;

void checkError(std::string type) {

	if (iResult != 0) {
		std::cout << type << " failed with error: " << iResult << " " << WSAGetLastError() << std::endl;
		return;
	}
}

std::string filetype(const std::string source, const std::string find) {
	std::vector<std::string> res;
	std::string haystack(source);

	int pos = 0;
	std::string token;
	while ((pos = haystack.find(find)) != std::string::npos) {
		token = haystack.substr(0, pos);
		res.insert(res.end(), token);
		haystack.erase(0, pos + find.length());
	}
	res.insert(res.end(), haystack);

	return res.back();
}

void initTypes();
void init();
void resolve(std::string ip);
void genListenSock();
void setupListenSock();
void acceptClientSock();
std::string compileHeader(std::vector<std::array<std::string, 2>> hOptions);
void compileMessage(const char* request, std::string& message);
void urldecode2(char* dst, const char* src);

int __cdecl main(int argc, char** argv) {

	std::string localIp;

	// Initialize Winsock
	init();

	initTypes();

	if (argc > 2 && argv[2] != "/") {
		localIp = argv[2];
	} else {
		localIp = "127.0.0.1";
	}

	if (argc > 1) {
		baseDir = argv[1];
	} else {
		baseDir = "./";
	}


	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	resolve(localIp);

	// Create a SOCKET for connecting to server
	genListenSock();

	// Setup the TCP listening socket
	setupListenSock();

	std::cout << "Server now listenig on " << localIp << ":" << DEFAULT_PORT << std::endl;
	std::cout << "On folder              " << baseDir << std::endl;

	// Accept a client socket
	acceptClientSock();

	// No longer need server socket
	// closesocket(ListenSocket);

	// Receive until the peer shuts down the connection
	while (true) {

		// recv return number of bytes received, if 0 it received nothing
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);

		if (iResult > 0) {

			std::cout << "\nREQUEST ARRIVED//////////////////////////////////////////////////////////////////////" << std::endl;
			std::cout << "\nBytes received: " << iResult << std::endl;
			std::cout << recvbuf << std::endl;

			std::cout << "\nHEADER SENT**************************************************************************" << std::endl;

			std::string message;

			compileMessage(recvbuf, message);

			// std::cout << message.c_str() << std::endl;

			std::cout << "\nREQUEST SATISFIED////////////////////////////////////////////////////////////////////" << std::endl;

			// acknowledge the segment back to the sender
			iSendResult = send(ClientSocket, message.c_str(), message.length(), 0);

			if (iSendResult == SOCKET_ERROR) {
				std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			std::cout << "Bytes sent: " << iSendResult << std::endl;
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

// What a terrible idea, will make a data base
void initTypes() {

	// Text
	Contents["1d-interleaved-parityfec"] = "text/1d-interleaved-parityfec";
	Contents["cache-manifest"] = "text/cache-manifest";
	Contents["calendar"] = "text/calendar";
	Contents["cql"] = "text/cql";
	Contents["cql-expression"] = "text/cql-expression";
	Contents["cql-identifier"] = "text/cql-identifier";
	Contents["css"] = "text/css";
	Contents["csv"] = "text/csv";
	Contents["csv-schema"] = "text/csv-schema";
	Contents["directory"] = "text/directory";
	Contents["dns"] = "text/dns";
	Contents["ecmascript"] = "text/ecmascript";
	Contents["encaprtp"] = "text/encaprtp";
	Contents["enriched"] = "";
	Contents["example"] = "text/example";
	Contents["fhirpath"] = "text/fhirpath";
	Contents["flexfec"] = "text/flexfec";
	Contents["fwdred"] = "text/fwdred";
	Contents["gff3"] = "text/gff3";
	Contents["grammar-ref-list"] = "text/grammar-ref-list";
	Contents["html"] = "text/html";
	Contents["javascript"] = "text/javascript";
	Contents["jcr-cnd"] = "text/jcr-cnd";
	Contents["markdown"] = "text/markdown";
	Contents["mizar"] = "text/mizar";
	Contents["n3"] = "text/n3";
	Contents["parameters"] = "text/parameters";
	Contents["parityfec"] = "text/parityfec";
	Contents["plain"] = "";
	Contents["provenance-notation"] = "text/provenance-notation";
	Contents["prs.fallenstein.rst"] = "text/prs.fallenstein.rst";
	Contents["prs.lines.tag"] = "text/prs.lines.tag";
	Contents["prs.prop.logic"] = "text/prs.prop.logic";
	Contents["raptorfec"] = "text/raptorfec";
	Contents["RED"] = "text/RED";
	Contents["rfc822-headers"] = "text/rfc822-headers";
	Contents["richtext"] = "";
	Contents["rtf"] = "text/rtf";
	Contents["rtp-enc-aescm128"] = "text/rtp-enc-aescm128";
	Contents["rtploopback"] = "text/rtploopback";
	Contents["rtx"] = "text/rtx";
	Contents["SGML"] = "text/SGML";
	Contents["shaclc"] = "text/shaclc";
	Contents["spdx"] = "text/spdx";
	Contents["strings"] = "text/strings";
	Contents["t140"] = "text/t140";
	Contents["tab-separated-values"] = "text/tab-separated-values";
	Contents["troff"] = "text/troff";
	Contents["turtle"] = "text/turtle";
	Contents["ulpfec"] = "text/ulpfec";
	Contents["uri-list"] = "text/uri-list";
	Contents["vcard"] = "text/vcard";
	Contents["vnd.a"] = "text/vnd.a";
	Contents["vnd.abc"] = "text/vnd.abc";
	Contents["vnd.ascii-art"] = "text/vnd.ascii-art";
	Contents["vnd.curl"] = "text/vnd.curl";
	Contents["vnd.debian.copyright"] = "text/vnd.debian.copyright";
	Contents["vnd.DMClientScript"] = "text/vnd.DMClientScript";
	Contents["vnd.dvb.subtitle"] = "text/vnd.dvb.subtitle";
	Contents["vnd.esmertec.theme-descriptor"] = "text/vnd.esmertec.theme-descriptor";
	Contents["vnd.ficlab.flt"] = "text/vnd.ficlab.flt";
	Contents["vnd.fly"] = "text/vnd.fly";
	Contents["vnd.fmi.flexstor"] = "text/vnd.fmi.flexstor";
	Contents["vnd.gml"] = "text/vnd.gml";
	Contents["vnd.graphviz"] = "text/vnd.graphviz";
	Contents["vnd.hans"] = "text/vnd.hans";
	Contents["vnd.hgl"] = "text/vnd.hgl";
	Contents["vnd.in3d.3dml"] = "text/vnd.in3d.3dml";
	Contents["vnd.in3d.spot"] = "text/vnd.in3d.spot";
	Contents["vnd.IPTC.NewsML"] = "text/vnd.IPTC.NewsML";
	Contents["vnd.IPTC.NITF"] = "text/vnd.IPTC.NITF";
	Contents["vnd.latex-z"] = "text/vnd.latex-z";
	Contents["vnd.motorola.reflex"] = "text/vnd.motorola.reflex";
	Contents["vnd.ms-mediapackage"] = "text/vnd.ms-mediapackage";
	Contents["vnd.net2phone.commcenter.command"] = "text/vnd.net2phone.commcenter.command";
	Contents["vnd.radisys.msml-basic-layout"] = "text/vnd.radisys.msml-basic-layout";
	Contents["vnd.senx.warpscript"] = "text/vnd.senx.warpscript";
	Contents["vnd.si.uricatalogue - OBSOLETED by request"] = "text/vnd.si.uricatalogue";
	Contents["vnd.sun.j2me.app-descriptor"] = "text/vnd.sun.j2me.app-descriptor";
	Contents["vnd.sosi"] = "text/vnd.sosi";
	Contents["vnd.trolltech.linguist"] = "text/vnd.trolltech.linguist";
	Contents["vnd.wap.si"] = "text/vnd.wap.si";
	Contents["vnd.wap.sl"] = "text/vnd.wap.sl";
	Contents["vnd.wap.wml"] = "text/vnd.wap.wml";
	Contents["vnd.wap.wmlscript"] = "text/vnd.wap.wmlscript";
	Contents["vtt"] = "text/vtt";
	Contents["xml"] = "text/xml";
	Contents["xml-external-parsed-entity"] = "text/xml-external-parsed-entity";
	// images
	Contents["aces"] = "image/aces";
	Contents["avci"] = "image/avci";
	Contents["avcs"] = "image/avcs";
	Contents["avif"] = "image/avif";
	Contents["bmp"] = "image/bmp";
	Contents["cgm"] = "image/cgm";
	Contents["dicom-rle"] = "image/dicom-rle";
	Contents["emf"] = "image/emf";
	Contents["example"] = "image/example";
	Contents["fits"] = "image/fits";
	Contents["g3fax"] = "image/g3fax";
	Contents["gif"] = "image/gif";
	Contents["heic"] = "image/heic";
	Contents["heic-sequence"] = "image/heic-sequence";
	Contents["heif"] = "image/heif";
	Contents["heif-sequence"] = "image/heif-sequence";
	Contents["hej2k"] = "image/hej2k";
	Contents["hsj2"] = "image/hsj2";
	Contents["jls"] = "image/jls";
	Contents["jp2"] = "image/jp2";
	Contents["jpeg"] = "image/jpeg";
	Contents["jpg"] = "image/jpg";
	Contents["jph"] = "image/jph";
	Contents["jphc"] = "image/jphc";
	Contents["jpm"] = "image/jpm";
	Contents["jpx"] = "image/jpx";
	Contents["jxr"] = "image/jxr";
	Contents["jxrA"] = "image/jxrA";
	Contents["jxrS"] = "image/jxrS";
	Contents["jxs"] = "image/jxs";
	Contents["jxsc"] = "image/jxsc";
	Contents["jxsi"] = "image/jxsi";
	Contents["jxss"] = "image/jxss";
	Contents["ktx"] = "image/ktx";
	Contents["ktx2"] = "image/ktx2";
	Contents["naplps"] = "image/naplps";
	Contents["png"] = "image/png";
	Contents["prs.btif"] = "image/prs.btif";
	Contents["prs.pti"] = "image/prs.pti";
	Contents["pwg-raster"] = "image/pwg-raster";
	Contents["svg+xml"] = "image/svg+xml";
	Contents["t38"] = "image/t38";
	Contents["tiff"] = "image/tiff";
	Contents["tiff-fx"] = "image/tiff-fx";
	Contents["vnd.adobe.photoshop"] = "image/vnd.adobe.photoshop";
	Contents["vnd.airzip.accelerator.azv"] = "image/vnd.airzip.accelerator.azv";
	Contents["vnd.cns.inf2"] = "image/vnd.cns.inf2";
	Contents["vnd.dece.graphic"] = "image/vnd.dece.graphic";
	Contents["vnd.djvu"] = "image/vnd.djvu";
	Contents["vnd.dwg"] = "image/vnd.dwg";
	Contents["vnd.dxf"] = "image/vnd.dxf";
	Contents["vnd.dvb.subtitle"] = "image/vnd.dvb.subtitle";
	Contents["vnd.fastbidsheet"] = "image/vnd.fastbidsheet";
	Contents["vnd.fpx"] = "image/vnd.fpx";
	Contents["vnd.fst"] = "image/vnd.fst";
	Contents["vnd.fujixerox.edmics-mmr"] = "image/vnd.fujixerox.edmics-mmr";
	Contents["vnd.fujixerox.edmics-rlc"] = "image/vnd.fujixerox.edmics-rlc";
	Contents["vnd.globalgraphics.pgb"] = "image/vnd.globalgraphics.pgb";
	Contents["vnd.microsoft.icon"] = "image/vnd.microsoft.icon";
	Contents["vnd.mix"] = "image/vnd.mix";
	Contents["vnd.ms-modi"] = "image/vnd.ms-modi";
	Contents["vnd.mozilla.apng"] = "image/vnd.mozilla.apng";
	Contents["vnd.net-fpx"] = "image/vnd.net-fpx";
	Contents["vnd.pco.b16"] = "image/vnd.pco.b16";
	Contents["vnd.radiance"] = "image/vnd.radiance";
	Contents["vnd.sealed.png"] = "image/vnd.sealed.png";
	Contents["vnd.sealedmedia.softseal.gif"] = "image/vnd.sealedmedia.softseal.gif";
	Contents["vnd.sealedmedia.softseal.jpg"] = "image/vnd.sealedmedia.softseal.jpg";
	Contents["vnd.svf"] = "image/vnd.svf";
	Contents["vnd.tencent.tap"] = "image/vnd.tencent.tap";
	Contents["vnd.valve.source.texture"] = "image/vnd.valve.source.texture";
	Contents["vnd.wap.wbmp"] = "image/vnd.wap.wbmp";
	Contents["vnd.xiff"] = "image/vnd.xiff";
	Contents["vnd.zbrush.pcx"] = "image/vnd.zbrush.pcx";
	Contents["wmf"] = "image/wmf";
}

void init() {
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	checkError("WSAstartup");
}

void resolve(std::string ip) {
	iResult = getaddrinfo(ip.c_str(), DEFAULT_PORT, &hints, &result);
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

	for (int i = hOptions.size() - 1; i >= 0; i--) {
		res += hOptions[i][0] + ": " + hOptions[i][1] + "\n";
	}
	return res;
}

void compileMessage(const char* request, std::string& message) {

	std::string Srequest = request;

	// find the file requested
	// GET ****** HTTP/ i need to get the (***) of unkown lenght, i skip GET and take a subtring from index 4 to index ?

	// need the end index of the file requested
	int endIndex = Srequest.find("HTTP/");

	// actually get the filename with the heading slash (/), -5 cus std::string::find return the index of the last char of the string given

	std::string file = Srequest.substr(4, endIndex - 5);

	char* dst = (char*) (file.c_str());

	urldecode2(dst, file.c_str());

	file = dst;

	file = baseDir + file;
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
		hOptions.insert(hOptions.begin(), {"HTTP/1.1", "200 OK"});
		std::string temp = filetype(file, ".");

		std::array<std::string, 2> content_type = {"Content-Type", Contents[temp]};
		if (content_type[1] == "") {
			content_type[1] = "text/plain";
		}

		hOptions.insert(hOptions.begin(), content_type);

	} else {
		hOptions.insert(hOptions.begin(), {"HTTP/1.1", "404 Not Found"});
		hOptions.insert(hOptions.begin(), {"Content-Type", "text/html; charset=UTF-8"});
		// get the missing page file
		std::fstream ifs("source/404.html", std::ios::binary | std::ios::in);

		content = std::string((std::istreambuf_iterator<char>(ifs)),
							  (std::istreambuf_iterator<char>()));
	}

	hOptions.insert(hOptions.begin(), {"Connection", "close"});
	hOptions.insert(hOptions.begin(), {"Content-Lenght", std::to_string(content.length())});
	hOptions.insert(hOptions.begin(), {"Server", "LeoCustom"});

	std::string header = compileHeader(hOptions);

	std::cout << header << std::endl;

	std::string SendString = header + "\n" + content;

	message = SendString;
}

// Thank you ThomasH, https://stackoverflow.com/users/2012498/thomash at https://stackoverflow.com/questions/2673207/c-c-url-decode-library/2766963,
void urldecode2(char* dst, const char* src) {
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