#include "../headers/ParserConfigFile.hpp"
#include "../headers/Server.hpp"

/* -------------------------------------------------------------------------- */
/*                          Constructors & Destructor                          */
/* -------------------------------------------------------------------------- */

ParserConfFile::ParserConfFile(std::string path) : _path(path) { servers_count = 0; }
ParserConfFile::~ParserConfFile() {}
ParserConfFile::ParserConfFile() {}
ParserConfFile::ParserConfFile(const ParserConfFile &src) { (void)src; }

/* -------------------------------------------------------------------------- */
/*                              Getters & Setters                             */
/* -------------------------------------------------------------------------- */
std::vector<Server> &ParserConfFile::get_servers() { return this->servers; }

/* -------------------------------------------------------------------------- */
/*                                   Methods                                  */
/* -------------------------------------------------------------------------- */

int ParserConfFile::open_config_file() {
    std::ifstream in_file_stream(this->_path.c_str());
    if (!in_file_stream.is_open()) {
        std::cerr << "Error opening config file" << std::endl;
        return 1;
    }
    std::string line;
    while (std::getline(in_file_stream, line)) {
        std::istringstream ss(line);
        std::string member;
        std::string curr;
        while (ss >> curr) {

            std::size_t found = curr.find_first_of("{};");

            while (found != std::string::npos) {
                if (found == 0) {
                    // if brackets are at the start of the string
                    tokens.push_back(curr.substr(0, 1));
                    curr.erase(0, 1);
                } else {
                    // brackets are in the middle of the string
                    tokens.push_back(curr.substr(0, found));
                    curr.erase(0, found);
                }
                // find the next occurrence
                found = curr.find_first_of("{}");
            }

            if (curr.size())
                tokens.push_back(curr);
        }
    }
    // printVector(this->tokens);
    if (this->extract_server()) {
        std::cerr << "Error: Config File Invalid Sintax" << std::endl;
        return 1;
    }
    // printVector(tokens);
    // exit(0);
    // std::cout << "host=>" << servers[0].host << std::endl;
    // std::cout << "port=>" << servers[1].port << std::endl;
    return 0;
}

std::vector<Server> &ParserConfFile::extract_servers_data() { return (this->servers); }

int ParserConfFile::get_serv_data(std::vector<std::string>::iterator &it, Server &s) {
    int brackets_count = 0;
    ++it;
    if (*it == "{") {
        brackets_count++;
        ++it;
        while (brackets_count) {
            if (*it == "{")
                brackets_count++;
            else if (*it == "}")
                brackets_count--;
            else if (*it == "server_name") {
                ++it;
                while (*it != ";") {
                    s.server_names.push_back(*it);
                    ++it;
                }
            } else if (*it == "error_page") {
                ++it;

                // error_page 404 custom.html;
                std::vector<std::string> temp;
                while (*it != ";") {
                    temp.push_back(*it);
                    ++it;
                }

                if (temp.size() < 2) {
                    print_error("invalid error_page");
                    return 1;
                }

                // get the last word
                std::string error_file_path = temp.back();
                // remove from the vector
                temp.pop_back();

                // we're dealing with something like this
                // temp = [500, 502, 503]
                // error_file_path = "404.html"

                std::vector<std::string>::iterator it2;
                for (it2 = temp.begin(); it2 != temp.end(); it2++) {
                    s.update_error_page(*it2, error_file_path);
                }

            } else if (*it == "listen") {
                ++it;
                std::size_t pos = (*it).find(':');
                if (pos == std::string::npos) {
                    s.port = *it;
                    s.sin_port = htons(ft_stoi(*it));
                    s.host = "127.0.0.1";
                    s.s_addr = htonl(convert_str_to_uint32("127.0.0.1"));
                } else { // has ':'
                    s.host = (*it).substr(0, pos);
                    s.s_addr = htonl(convert_str_to_uint32(s.host));
                    s.port = (*it).substr(pos + 1);
                    s.sin_port = htons(ft_stoi(s.port));
                }

            } else if (*it == "root") {
                ++it;
                s.root = *it;
            } else if (*it == "client_max_body_size") {
                ++it;
                std::stringstream ss((*it));
                ss >> s.client_max_body_size;
            }
            ++it;
        }
    } else {
        std::cerr << "Invalid syntax" << std::endl;
        return 1;
    }
    return 0;
}

int ParserConfFile::extract_server() {
    std::vector<std::string>::iterator it;
    bool inside_http = false;
    int brackets_count = 0;
    for (it = tokens.begin(); it != tokens.end();) {
        if (*it == "http" && *(it + 1) == "{") {
            inside_http = true;
            it++;
            continue;
        }
        if (*it == "{")
            brackets_count++;
        else if (*it == "}")
            brackets_count--;
        if (brackets_count == 0)
            inside_http = false;
        if (*it == "server" && inside_http) {
            Server s;
            if (get_serv_data(it, s))
                return 1;
            servers_count++;
            servers.push_back(s);
        } else
            it++;
        if (brackets_count < 0)
            return 1;
    }
    return 0;
}
void ParserConfFile::print_server_data() {}

void ParserConfFile::printMembers(void) const {
    std::vector<Server> tmp = servers;
    std::vector<Server>::iterator ite;

    for (ite = tmp.begin(); ite != tmp.end(); ite++) {
        std::cout << "----------------------------------------------------------" << std::endl;
        std::cout << "Host: " << ite->host << std::endl;
        std::cout << "Port: " << ite->port << std::endl;
        // std::cout << "test first: " << ite->server_names[0] << std::endl;
        std::cout << "Server name: " << std::endl;
        printVector(ite->server_names);
        std::cout << std::endl;
        std::cout << "Max body size: " << ite->client_max_body_size << std::endl;
        std::cout << "Root: " << ite->root << std::endl;
    }
}

std::ostream &operator<<(std::ostream &out, const ParserConfFile &obj) {
    obj.printMembers();
    return (out);
}

ParserConfFile &ParserConfFile::operator=(const ParserConfFile &src) {
    (void)src;
    return *this;
}

std::vector<struct sockaddr_in> ParserConfFile::get_unique_addresses() {
    std::vector<struct sockaddr_in> uniques;

    std::vector<Server>::iterator it;
    for (it = this->servers.begin(); it != this->servers.end(); it++) {
        struct sockaddr_in curr;

        ft_memset(&(curr), 0, sizeof(curr));

        // Define curr struct
        curr.sin_family = AF_INET;
        curr.sin_port = it->sin_port;      // host to network short
        curr.sin_addr.s_addr = it->s_addr; // ip

        std::vector<struct sockaddr_in>::iterator it2;
        for (it2 = uniques.begin(); it2 != uniques.end(); it2++) {
            if (it2->sin_addr.s_addr == curr.sin_addr.s_addr && it2->sin_port == curr.sin_port)
                break;
        }

        // if it2 is pointing to the end means it didnt found in the uniques
        if (it2 == uniques.end())
            uniques.push_back(curr);
    }

    return uniques;
}
