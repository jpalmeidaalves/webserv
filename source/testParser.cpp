#include "../headers/ParserConfFile.hpp"

int main ()
{
    ParserConfFile obj("configFile.conf");
    obj.open_config_file();
}