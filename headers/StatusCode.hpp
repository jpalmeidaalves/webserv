#ifndef STATUSCODE_HPP
#define STATUSCODE_HPP

#include <iostream>
#include <map>
#include <string>

typedef std::map<std::string, std::string> status_codes;

class StatusCode {
  public:
    static status_codes codes;
    static status_codes init_codes();
    static std::string get_code(std::string code);

  private:
    StatusCode();
    ~StatusCode();
    StatusCode(const StatusCode &src);
    StatusCode &operator=(const StatusCode &rhs);
};

#endif /* STATUSCODE_HPP */
