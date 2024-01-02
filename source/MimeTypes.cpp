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
    std::cout << "found: " << found << std::endl;
    if (found != std::string::npos) {

        std::string ext = path.substr(found);

        std::cout << "ext: " << ext << std::endl;

        it = MimeTypes::types.find(ext);
        if (it != MimeTypes::types.end())
            return it->second;
    }

    return ""; // unknow mime type, browser will try to assume the correct type
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
    // if (mime.find("json") != std::string::npos)
    //     return true;

    return false;
}


/*

File: mime.types

types {
    text/html                             html htm shtml;
    text/css                              css;
    text/xml                              xml;
    image/gif                             gif;
    image/jpeg                            jpeg jpg;
    application/javascript                js;
    application/atom+xml                  atom;
    application/rss+xml                   rss;

    text/mathml                           mml;
    text/plain                            txt;
    text/vnd.sun.j2me.app-descriptor      jad;
    text/vnd.wap.wml                      wml;
    text/x-component                      htc;

    image/avif                            avif;
    image/png                             png;
    image/svg+xml                         svg svgz;
    image/tiff                            tif tiff;
    image/vnd.wap.wbmp                    wbmp;
    image/webp                            webp;
    image/x-icon                          ico;
    image/x-jng                           jng;
    image/x-ms-bmp                        bmp;

    font/woff                             woff;
    font/woff2                            woff2;

    application/java-archive              jar war ear;
    application/json                      json;
    application/mac-binhex40              hqx;
    application/msword                    doc;
    application/pdf                       pdf;
    application/postscript                ps eps ai;
    application/rtf                       rtf;
    application/vnd.apple.mpegurl         m3u8;
    application/vnd.google-earth.kml+xml  kml;
    application/vnd.google-earth.kmz      kmz;
    application/vnd.ms-excel              xls;
    application/vnd.ms-fontobject         eot;
    application/vnd.ms-powerpoint         ppt;
    application/vnd.oasis.opendocument.graphics        odg;
    application/vnd.oasis.opendocument.presentation    odp;
    application/vnd.oasis.opendocument.spreadsheet     ods;
    application/vnd.oasis.opendocument.text            odt;
    application/vnd.openxmlformats-officedocument.presentationml.presentation    pptx;
    application/vnd.openxmlformats-officedocument.spreadsheetml.sheet    xlsx;
    application/vnd.openxmlformats-officedocument.wordprocessingml.document    docx;
    application/vnd.wap.wmlc              wmlc;
    application/wasm                      wasm;
    application/x-7z-compressed           7z;
    application/x-cocoa                   cco;
    application/x-java-archive-diff       jardiff;
    application/x-java-jnlp-file          jnlp;
    application/x-makeself                run;
    application/x-perl                    pl pm;
    application/x-pilot                   prc pdb;
    application/x-rar-compressed          rar;
    application/x-redhat-package-manager  rpm;
    application/x-sea                     sea;
    application/x-shockwave-flash         swf;
    application/x-stuffit                 sit;
    application/x-tcl                     tcl tk;
    application/x-x509-ca-cert            der pem crt;
    application/x-xpinstall               xpi;
    application/xhtml+xml                 xhtml;
    application/xspf+xml                  xspf;
    application/zip                       zip;

    application/octet-stream              bin exe dll;
    application/octet-stream              deb;
    application/octet-stream              dmg;
    application/octet-stream              iso img;
    application/octet-stream              msi msp msm;

    audio/midi                            mid midi kar;
    audio/mpeg                            mp3;
    audio/ogg                             ogg;
    audio/x-m4a                           m4a;
    audio/x-realaudio                     ra;

    video/3gpp                            3gpp 3gp;
    video/mp2t                            ts;
    video/mp4                             mp4;
    video/mpeg                            mpeg mpg;
    video/quicktime                       mov;
    video/webm                            webm;
    video/x-flv                           flv;
    video/x-m4v                           m4v;
    video/x-mng                           mng;
    video/x-ms-asf                        asx asf;
    video/x-ms-wmv                        wmv;
    video/x-msvideo                       avi;
}

*/