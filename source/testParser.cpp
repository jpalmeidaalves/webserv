#include "../headers/ParserConfigFile.hpp"

int main(int ac, char **av) {
    if (ac != 2) {
        std::cerr << "Usage: " << av[0] << " <config_file>" << std::endl;
        return 1;
    }
    ParserConfFile obj(av[1]);
    if (obj.open_config_file()) {
        std::cerr << "Finishing program" << std::endl;
        return 1;
    }
    obj.printMembers();
}

// compile for testing:
// c++ -Wall -Werror -Wextra ParserConfigFile.cpp testParser.cpp utils.cpp Server.cpp Request.cpp HTTP.cpp Response.cpp MimeTypes.cpp StatusCode.cpp -o testConfFile