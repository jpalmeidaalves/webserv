#ifndef MIMETYPES_HPP
#define MIMETYPES_HPP

#include <iostream>
#include <map>
#include <string>

typedef std::map<std::string, std::string> supported_types;

class MimeTypes {
  public:
    static supported_types types;
    static supported_types init_types();
    static std::string indentify(const std::string &path);

  private:
    MimeTypes();
    ~MimeTypes();
    MimeTypes(const MimeTypes &src);
    MimeTypes &operator=(const MimeTypes &rhs);
};

#endif /* MIMETYPES_HPP */