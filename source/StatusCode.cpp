#include "../headers/StatusCode.hpp"

status_codes StatusCode::init_codes() {
    status_codes codes_map;

    codes_map["200"] = "OK";
    codes_map["201"] = "Created";
    codes_map["400"] = "Bad Request";
    codes_map["401"] = "Unauthorized";
    codes_map["403"] = "Forbidden";
    codes_map["404"] = "Not Found";
    codes_map["500"] = "Internal Server Error";
    codes_map["501"] = "Not Implemented";

    return codes_map;
}

status_codes StatusCode::codes = init_codes();

std::string StatusCode::get_code(const std::string &code) {
    status_codes::iterator it;

    it = StatusCode::codes.find(code);
    if (it != StatusCode::codes.end())
        return ( code + " " + it->second);

    return ("500 Internal Server Error");
}
