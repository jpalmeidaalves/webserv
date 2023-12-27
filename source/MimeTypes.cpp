#include "../headers/MimeTypes.hpp"

supported_types MimeTypes::init_types() {
    supported_types mimeMap;

    mimeMap[".html"] = "text/html";
    mimeMap[".txt"] = "text/plain";
    mimeMap[".css"] = "text/css";
    mimeMap[".js"] = "application/javascript";
    mimeMap[".json"] = "application/json";
    mimeMap[".xml"] = "application/xml";
    mimeMap[".jpeg"] = "image/jpeg";
    mimeMap[".png"] = "image/png";
    mimeMap[".gif"] = "image/gif";
    mimeMap[".mp3"] = "audio/mpeg";
    mimeMap[".mp4"] = "video/mp4";
    mimeMap[".pdf"] = "application/pdf";
    mimeMap[".zip"] = "application/zip";
    mimeMap[".bin"] = "application/octet-stream";
    mimeMap[".urlencoded"] = "application/x-www-form-urlencoded";
    mimeMap[".multipart"] = "multipart/form-data";
    mimeMap[".csv"] = "text/csv";
    mimeMap[".xhtml"] = "application/xhtml+xml";
    mimeMap[".rss"] = "application/rss+xml";
    mimeMap[".atom"] = "application/atom+xml";
    mimeMap[".ico"] = "image/vnd.microsoft.icon";

    return mimeMap;
}

supported_types MimeTypes::types = init_types();

std::string MimeTypes::indentify(const std::string &path) {
    supported_types::iterator it;

    std::size_t found = path.find_last_of(".");
    if (found != std::string::npos) {

        std::string ext = path.substr(found);

        it = MimeTypes::types.find(ext);
        if (it != MimeTypes::types.end())
            return it->second;
    }

    return ""; // unknow mime type, browser will try to assume the correct type
}
