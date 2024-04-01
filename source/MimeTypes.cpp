#include "../headers/MimeTypes.hpp"

supported_types MimeTypes::init_types() {
    supported_types mimeMap;

    mimeMap[".html"] = "text/html";
    mimeMap[".htm"] = "text/html";
    mimeMap[".shtml"] = "text/html";
    mimeMap[".css"] = "text/css";
    mimeMap[".xml"] = "text/xml";
    mimeMap[".gif"] = "image/gif";
    mimeMap[".jpeg"] = "image/jpeg";
    mimeMap[".jpg"] = "image/jpeg";
    mimeMap[".js"] = "application/javascript";
    mimeMap[".atom"] = "application/atom+xml";
    mimeMap[".rss"] = "application/rss+xml";
    mimeMap[".mml"] = "text/mathml";
    mimeMap[".txt"] = "text/plain";
    mimeMap[".jad"] = "text/vnd.sun.j2me.app-descriptor";
    mimeMap[".wml"] = "text/vnd.wap.wml";
    mimeMap[".htc"] = "text/x-component";
    mimeMap[".avif"] = "image/avif";
    mimeMap[".png"] = "image/png";
    mimeMap[".svg"] = "image/svg+xml";
    mimeMap[".svgz"] = "image/svg+xml";
    mimeMap[".tif"] = "image/tiff";
    mimeMap[".tiff"] = "image/tiff";
    mimeMap[".wbmp"] = "image/vnd.wap.wbmp";
    mimeMap[".webp"] = "image/webp";
    mimeMap[".ico"] = "image/x-icon";
    mimeMap[".jng"] = "image/x-jng";
    mimeMap[".bmp"] = "image/x-ms-bmp";
    mimeMap[".woff"] = "font/woff";
    mimeMap[".woff2"] = "font/woff2";
    mimeMap[".jar"] = "application/java-archive";
    mimeMap[".war"] = "application/java-archive";
    mimeMap[".ear"] = "application/java-archive";
    mimeMap[".json"] = "application/json";
    mimeMap[".hqx"] = "application/mac-binhex40";
    mimeMap[".doc"] = "application/msword";
    mimeMap[".pdf"] = "application/pdf";
    mimeMap[".ps"] = "application/postscript";
    mimeMap[".eps"] = "application/postscript";
    mimeMap[".ai"] = "application/postscript";
    mimeMap[".rtf"] = "application/rtf";
    mimeMap[".m3u8"] = "application/vnd.apple.mpegurl";
    mimeMap[".kml"] = "application/vnd.google-earth.kml+xml";
    mimeMap[".kmz"] = "application/vnd.google-earth.kmz";
    mimeMap[".xls"] = "application/vnd.ms-excel";
    mimeMap[".eot"] = "application/vnd.ms-fontobject";
    mimeMap[".ppt"] = "application/vnd.ms-powerpoint";
    mimeMap[".odg"] = "application/vnd.oasis.opendocument.graphics";
    mimeMap[".odp"] = "application/vnd.oasis.opendocument.presentation";
    mimeMap[".ods"] = "application/vnd.oasis.opendocument.spreadsheet";
    mimeMap[".odt"] = "application/vnd.oasis.opendocument.text";
    mimeMap[".pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
    mimeMap[".xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
    mimeMap[".docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
    mimeMap[".wmlc"] = "application/vnd.wap.wmlc";
    mimeMap[".wasm"] = "application/wasm";
    mimeMap[".7z"] = "application/x-7z-compressed";
    mimeMap[".cco"] = "application/x-cocoa";
    mimeMap[".jardiff"] = "application/x-java-archive-diff";
    mimeMap[".jnlp"] = "application/x-java-jnlp-file";
    mimeMap[".run"] = "application/x-makeself";
    mimeMap[".pl"] = "application/x-perl";
    mimeMap[".pm"] = "application/x-perl";
    mimeMap[".prc"] = "application/x-pilot";
    mimeMap[".pdb"] = "application/x-pilot";
    mimeMap[".rar"] = "application/x-rar-compressed";
    mimeMap[".rpm"] = "application/x-redhat-package-manager";
    mimeMap[".sea"] = "application/x-sea";
    mimeMap[".swf"] = "application/x-shockwave-flash";
    mimeMap[".sit"] = "application/x-stuffit";
    mimeMap[".tcl"] = "application/x-tcl";
    mimeMap[".tk"] = "application/x-tcl";
    mimeMap[".der"] = "application/x-x509-ca-cert";
    mimeMap[".pem"] = "application/x-x509-ca-cert";
    mimeMap[".crt"] = "application/x-x509-ca-cert";
    mimeMap[".xpi"] = "application/x-xpinstall";
    mimeMap[".xhtml"] = "application/xhtml+xml";
    mimeMap[".xspf"] = "application/xspf+xml";
    mimeMap[".zip"] = "application/zip";
    mimeMap[".bin"] = "application/octet-stream";
    mimeMap[".exe"] = "application/octet-stream";
    mimeMap[".dll"] = "application/octet-stream";
    mimeMap[".deb"] = "application/octet-stream";
    mimeMap[".dmg"] = "application/octet-stream";
    mimeMap[".iso"] = "application/octet-stream";
    mimeMap[".img"] = "application/octet-stream";
    mimeMap[".msi"] = "application/octet-stream";
    mimeMap[".msp"] = "application/octet-stream";
    mimeMap[".mid"] = "audio/midi";
    mimeMap[".midi"] = "audio/midi";
    mimeMap[".kar"] = "audio/midi";
    mimeMap[".mp3"] = "audio/mpeg";
    mimeMap[".ogg"] = "audio/ogg";
    mimeMap[".m4a"] = "audio/x-m4a";
    mimeMap[".ra"] = "audio/x-realaudio";
    mimeMap[".3gpp"] = "video/3gpp";
    mimeMap[".3gp"] = "video/3gpp";
    mimeMap[".ts"] = "video/mp2t";
    mimeMap[".mp4"] = "video/mp4";
    mimeMap[".mpeg"] = "video/mpeg";
    mimeMap[".mpg"] = "video/mpeg";
    mimeMap[".mov"] = "video/quicktime";
    mimeMap[".webm"] = "video/webm";
    mimeMap[".flv"] = "video/x-flv";
    mimeMap[".m4v"] = "video/x-m4v";
    mimeMap[".mng"] = "video/x-mng";
    mimeMap[".asx"] = "video/x-ms-asf";
    mimeMap[".asf"] = "video/x-ms-asf";
    mimeMap[".wmv"] = "video/x-ms-wmv";
    mimeMap[".avi"] = "video/x-msvideo";

    return mimeMap;
}

supported_types MimeTypes::types = init_types();

std::string MimeTypes::identify(const std::string &original_path) {

    std::string path = original_path;
    std::transform(path.begin(), path.end(), path.begin(), ::tolower);

    supported_types::iterator it;
    std::size_t found = path.find_last_of(".");
    if (found != std::string::npos) {

        std::string ext = path.substr(found);

        it = MimeTypes::types.find(ext);
        if (it != MimeTypes::types.end())
            return it->second;
    }

    return "text/html"; // unknow mime type, browser will try to assume the correct type
}

bool MimeTypes::is_binary_file(const std::string &mime) {
    if (mime.find("image/") != std::string::npos)
        return true;
    if (mime.find("video/") != std::string::npos)
        return true;
    if (mime.find("audio/") != std::string::npos)
        return true;
    if (mime.find("pdf") != std::string::npos)
        return true;
        
    return false;
}