#include <iostream>
#include <sstream>
#include <istream>
#include <cstring>

int main() {
    std::stringstream ss;

    ss.write("ola tudo bem?\n", 14);
    ss.write("adeus\n", 6);

    std::string line;
    getline(ss, line);

    std::cout << "line " << line << std::endl;

    char buf[100 + 1];
    std::memset(buf, 0, sizeof(buf));

    ss.read(buf, 100);
    
    std::size_t bytes_read = ss.gcount();

    std::cout << "number of bytes left in ss: " << bytes_read << std::endl;
    std::cout << buf << std::endl;

    return 0;
}
