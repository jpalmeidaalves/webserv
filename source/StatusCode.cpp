#include "../headers/StatusCode.hpp"

status_codes StatusCode::init_codes() {
    status_codes codes_map;

    codes_map["100"] = "Continue";
    codes_map["101"] = "Switching Protocols";
    codes_map["102"] = "Processing";
    codes_map["103"] = "Early Hints";

    codes_map["200"] = "OK";
    codes_map["201"] = "Created";
    codes_map["202"] = "Accepted";
    codes_map["203"] = "Non-Authoritative Information";
    codes_map["204"] = "No Content";
    codes_map["205"] = "Reset Content";
    codes_map["206"] = "Partial Content";
    codes_map["207"] = "Multi-Status";
    codes_map["208"] = "Already Reported";
    codes_map["226"] = "IM Used";

    codes_map["300"] = "Multiple Choices";
    codes_map["301"] = "Moved Permanently";
    codes_map["302"] = "Found";
    codes_map["303"] = "See Other";
    codes_map["304"] = "Not Modified";
    codes_map["307"] = "Temporary Redirect";
    codes_map["308"] = "Permanent Redirect";

    codes_map["400"] = "Bad Request";
    codes_map["401"] = "Unauthorized";
    codes_map["403"] = "Forbidden";
    codes_map["404"] = "Not Found";
    codes_map["405"] = "Method Not Allowed";
    codes_map["406"] = "Not Acceptable";
    codes_map["407"] = "Proxy Authentication Required";
    codes_map["408"] = "Request Timeout";
    codes_map["409"] = "Conflict";
    codes_map["410"] = "Gone";
    codes_map["411"] = "Length Required";
    codes_map["412"] = "Precondition Failed";
    codes_map["413"] = "Content Too Large";
    codes_map["414"] = "URI Too Long";
    codes_map["415"] = "Unsupported Media Type";
    codes_map["416"] = "Range Not Satisfiable";
    codes_map["417"] = "Expectation Failed";
    codes_map["418"] = "I'm a teapot";
    codes_map["421"] = "Misdirected Request";
    codes_map["422"] = "Unprocessable Content";
    codes_map["423"] = "Locked";
    codes_map["424"] = "Failed Dependency";
    codes_map["426"] = "Upgrade Required";
    codes_map["428"] = "Precondition Required";
    codes_map["429"] = "Too Many Requests";
    codes_map["431"] = "Request Header Fields Too Large";
    codes_map["451"] = "Unavailable For Legal Reasons";

    codes_map["500"] = "Internal Server Error";
    codes_map["501"] = "Not Implemented";
    codes_map["502"] = "Bad Gateway";
    codes_map["503"] = "Service Unavailable";
    codes_map["504"] = "Gateway Timeout";
    codes_map["505"] = "HTTP Version Not Supported";
    codes_map["506"] = "Variant Also Negotiates";
    codes_map["507"] = "Insufficient Storage";
    codes_map["508"] = "Loop Detected";
    codes_map["510"] = "Not Extended";
    codes_map["511"] = "Network Authentication Required";

    return codes_map;
}

status_codes StatusCode::codes = init_codes();

std::string StatusCode::get_code(std::string code) {
    status_codes::iterator it;

    std::string res = "500 Internal Server Error";

    it = StatusCode::codes.find(code);
    if (it != StatusCode::codes.end()) {
        res = code + " " + it->second;
    }

    return (res);
}
