#include "HTTP_message.hpp"
#include "utils.hpp"

HTTP_message::HTTP_message(std::string& raw_message, unsigned int dir) {
	message = raw_message;
	direction = dir;

	decompileMessage();
}

/**
* deconstruct the raw header in a map with key (option) -> value
*/
void HTTP_message::decompileHeader() {
	std::vector<std::string> options = split(rawHeader, "\r\n");
	std::vector<std::string> temp;

	for (size_t i = 0; i < options.size(); i++) {

		temp = split(options[i], ":");
		// first header option "METHOD file HTTP/version"
		if (temp.size() <= 1 && temp[0] != "") {

			temp = split(options[i], " ");
			parseMethod(temp[0]);
			filename = temp[1];
			HTTP_version = temp[2];
		}

		// the last header option is \n
		if (temp.size() >= 2) {
			headerOptions[temp[0]] = temp[1];
		}
	}

}

/**
* decompile message in header and body
*/
void HTTP_message::decompileMessage() {
	// body and header are divided by two newlines
	std::vector<std::string> mex = split(message, "\r\n\r\n");

	rawHeader = mex[0];
	rawBody = mex[1];
	decompileHeader();
}

/**
* format the header from the array to a string
*/
void HTTP_message::compileHeader() {
	// Always the response code fisrt
	rawHeader += "HTTP/1.1 " + headerOptions["HTTP/1.1"] + "\r\n";

	for (auto const& [key, val] : headerOptions) {
		// already wrote response code
		if (key != "HTTP/1.1") {
			// header option name :	header option value
			rawHeader += key + ": " + val + "\r\n";
		}
	}
}

/**
* unite the header and the body in a single message
*/
void HTTP_message::compileMessage() {
	compileHeader();
	message = rawHeader + "\r\n" + rawBody;
}

/**
* given the string containing the request method the the property method to it's correct value
*/
void HTTP_message::parseMethod(std::string& requestMethod) {
	// Yep that's it, such logic
	if (requestMethod == "GET") {
		method = HTTP_GET;
	} else if (requestMethod == "HEAD") {
		method = HTTP_HEAD;
	} else if (requestMethod == "POST") {
		method = HTTP_POST;
	} else if (requestMethod == "PUT") {
		method = HTTP_PUT;
	} else if (requestMethod == "DELETE") {
		method = HTTP_DELETE;
	} else if (requestMethod == "OPTIONS") {
		method = HTTP_OPTIONS;
	} else if (requestMethod == "CONNECT") {
		method = HTTP_CONNECT;
	} else if (requestMethod == "TRACE") {
		method = HTTP_TRACE;
	} else if (requestMethod == "PATCH") {
		method = HTTP_PATCH;
	}
}

/**
* simply add the given key -> value pair to the headerOption map
*/
void HTTP_message::addHeaderOptions(std::string& key, std::string& value) {
	headerOptions[key] = value;
}
