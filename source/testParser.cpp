#include "../headers/ParserConfigFile.hpp"

int main ()
{
    ParserConfFile obj("configFile.conf");
    obj.open_config_file();
    obj.printMembers();
}