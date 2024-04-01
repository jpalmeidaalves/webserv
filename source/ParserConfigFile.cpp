#include "../headers/ParserConfigFile.hpp"
#include "../headers/Server.hpp"

/* -------------------------------------------------------------------------- */
/*                          Constructors & Destructor                          */
/* -------------------------------------------------------------------------- */

ParserConfFile::ParserConfFile(std::string path) : _path(path), _servers_count(0) {}
ParserConfFile::~ParserConfFile() {}
ParserConfFile::ParserConfFile() {}
ParserConfFile::ParserConfFile(const ParserConfFile &src) { (void)src; }

/* -------------------------------------------------------------------------- */
/*                              Getters & Setters                             */
/* -------------------------------------------------------------------------- */
std::vector<Server> &ParserConfFile::get_servers() { return this->_servers; }

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
        if (line.find("#") != std::string::npos) {
            line = line.substr(0, line.find("#"));
        }
        std::istringstream ss(line);
        std::string member;
        std::string curr;
        while (ss >> curr) {

            std::size_t found = curr.find_first_of("{};");

            while (found != std::string::npos) {
                if (found == 0) {
                    // if brackets are at the start of the string
                    _tokens.push_back(curr.substr(0, 1));
                    curr.erase(0, 1);
                } else {
                    // brackets are in the middle of the string
                    _tokens.push_back(curr.substr(0, found));
                    curr.erase(0, found);
                }
                // find the next occurrence
                found = curr.find_first_of("{}");
            }

            if (curr.size())
                _tokens.push_back(curr);
        }
    }

    // print_vector(this->_tokens);

    if (check_brackets_integrity())
        return 1;
    if (this->extract_server()) {
        return 1;
    }

    return 0;
}

std::vector<Server> &ParserConfFile::extract_servers_data() { return (this->_servers); }

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
                while (it != this->_tokens.end() && *it != ";") {
                    s.server_names.push_back(*it);
                    ++it;
                }
            } else if (*it == "error_page") {
                ++it;
                std::vector<std::string> temp;
                while (it != this->_tokens.end() && *it != ";") {
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
                s.has_listen = true;
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
                ++it;
            } else if (*it == "root") {
                s.has_root = true;
                ++it;
                s.root = *it;
                ++it;
            } else if (*it == "client_max_body_size") {
                ++it;
                std::string tmp = *it;
                char unit = '\0';
                try {
                    unit = tmp.at(tmp.size() - 1);
                    if (!isdigit(unit)) {
                        tmp.erase(tmp.size() - 1);
                    } else 
                        unit = '\0';
                } catch(const std::exception& e) {
                    unit = '\0';
                }
                if (tmp == "") {
                    std::cerr << "Error: 'client_max_body_size' invalid format" << std::endl;
                    return (1);
                }
                int size = ft_stoi(tmp);
                unit = std::toupper(unit);
                
                if (unit == 'K') {
                    size *= 1024;
                } else if (unit == 'M') {
                    size *= (1024 * 1024);
                } else if (unit != '\0'){
                    std::cerr << "Error: 'client_max_body_size' invalid unit" << std::endl;
                    return 1;
                }

                s.client_max_body_size = size;
                ++it;
            } else if (*it == "index") {
                s.has_index = true;
                it++;
                while(it != this->_tokens.end()) {
                    if (it->find_first_of("{};") != std::string::npos)
                        break;
                    s.index_pages.push_back(*it);
                    it++;
                }

            } else if (*it == "location") {
                it++;
                if (this->extract_location(it, s))
                    return 1;
            } else {
                std::cerr << "Error: unknown directive " << *it << std::endl;
                return 1;
            }
            if (it == this->_tokens.end() || it->find_first_of("{};") == std::string::npos) {
                std::cerr << "Error: invalid syntax after " << *(it - 1) << std::endl;
                return 1;
            }
            ++it;
        }
    } else {
        std::cerr << "Error: Invalid syntax in config file" << std::endl;
        return 1;
    }
    if(s.has_listen && s.has_root && s.has_index && s.server_names.size() > 0)
        return 0;
    else {
        std::cerr << "Error: missing directive in server block" << std::endl;
        return 1;
    }
}

int ParserConfFile::extract_location(std::vector<std::string>::iterator &it, Server &s) {
    int brackets_count = 0;

    if (it->find("/") != 0 && it->find("*") != 0)
        return 1;

    std::string location = *it;
    s.locations[location] = LocationOptions();
    s.locations[location].autoindex = false;
    
    it++;

    while(it != this->_tokens.end()) {
        if (*it == "{")
            brackets_count++;
        else if (*it == "}") {
            brackets_count--;
            if (brackets_count == 0)
                break;
        } else if (*it == "root") {
            s.has_root = true;
            it++;
            s.locations[location].root = *it;
            it++;
            if (*it != ";") {
                std::cerr << "Error: invalid syntax, missing(0) ;" << std::endl;
                return 1;
            }
        }else if (*it == "cgi_pass") {
            it++;
            s.locations[location].cgi_pass = *it;
            it++;
            if (*it != ";") {
                std::cerr << "Error: invalid syntax, missing(2) ;" << std::endl;
                return 1;
            }
        } else if (*it == "autoindex") {
            it++;
            if (*it == "on")
                s.locations[location].autoindex = true;
            else if (*it == "off")
                s.locations[location].autoindex = false;
            else {
                std::cerr << "Error: invalid value in autoindex" << std::endl;
                return 1;
            }
            it++;
            if (*it != ";") {
                std::cerr << "Error: invalid syntax, missing(3) ;" << std::endl;
                return 1;
            }
        } else if (*it == "allowed_methods") {
            it++;
            while(it != this->_tokens.end()) {
                if (it->find_first_of("{};") != std::string::npos)
                    break;
                s.locations[location].allowed_methods.push_back(*it);
                it++;
            }
            if (*it != ";") {
                std::cerr << "Error: invalid syntax, missing(4) ;" << std::endl;
                return 1;
            }
        } else if (*it == "return") {
            it++;
            if (it->size() == 0 || it->size() > 3 || it->find_first_not_of("0123456789") != std::string::npos) {
                
                std::cerr << "Error: invalid status code in redirect config file" << std::endl;
                return 1;
            }

            s.locations[location].redirect.first = *it;
            it++;

            if (it->find_first_of("{};") != std::string::npos) {
                std::cerr << "Error: syntax error in config file" << std::endl;
                return 1;
            }

            s.locations[location].redirect.second = *it;
            it++;

            if (*it != ";") {
                std::cerr << "Error: invalid syntax, missing(5) ;" << std::endl;
                return 1;
            }
        } else if (*it == "index") {
            it++;
            while(it != this->_tokens.end()) {
                if (it->find_first_of("{};") != std::string::npos)
                    break;
                s.locations[location].index_pages.push_back(*it);
                it++;
            }
            if (*it != ";") {
                std::cerr << "Error: invalid syntax, missing(6) ;" << std::endl;
                return 1;
            }
        } else if (*it == "client_body_temp_path") {
            it++;
            if (it->find_first_of("{};") != std::string::npos) {
                std::cerr << "Error: 'client_body_temp_path' has invalid value" << std::endl;
                return 1;
            }

            if (*it != "") {
                s.locations[location].client_body_temp_path = *it;
                if (!has_suffix(s.locations[location].client_body_temp_path, "/"))
                    s.locations[location].client_body_temp_path += "/";

                if (s.locations[location].client_body_temp_path.find("./") == 0) {
                    s.locations[location].client_body_temp_path = "." + s.locations[location].client_body_temp_path;
                } else if (s.locations[location].client_body_temp_path.find("../") == 0) {
                    s.locations[location].client_body_temp_path = "../" + s.locations[location].client_body_temp_path;
                }
                it++;

                if (*it != ";") {
                    std::cerr << "Error: invalid syntax, missing(7) ;" << std::endl;
                    return 1;
                }
            }

        }

        it++;
    }
    
    if (brackets_count != 0)
        return 1;
    return 0;
}

int ParserConfFile::extract_server() {
    std::vector<std::string>::iterator it;
    bool inside_http = false;
    int brackets_count = 0;
    for (it = _tokens.begin(); it != _tokens.end();) {
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
            _servers_count++;
            _servers.push_back(s);
        } else if (*it == "{" || *it == "}")
            it++;
        else {
            std::cout << "Error: bad syntax" << std::endl;
            return 1;}
        if (brackets_count < 0)
            return 1;
    }
    return 0;
}

void ParserConfFile::printMembers(void) const {
    std::vector<Server> tmp = _servers;
    std::vector<Server>::iterator ite;

    for (ite = tmp.begin(); ite != tmp.end(); ite++) {
        std::cout << "----------------------------------------------------------" << std::endl;
        std::cout << "Host: " << ite->host << std::endl;
        std::cout << "Port: " << ite->port << std::endl;
        std::cout << "Server name: ";
        print_vector_with_space(ite->server_names);
        std::cout << std::endl;
        std::cout << "Max body size: " << ite->client_max_body_size << std::endl;
        std::cout << "Root: " << ite->root << std::endl;
        std::cout << "Index pages: ";
        print_vector_with_space(ite->index_pages);
        std::map<std::string, struct LocationOptions>::iterator loc_it;

        for (loc_it = ite->locations.begin(); loc_it != ite->locations.end(); loc_it++) {
            std::cout << "  location: " << loc_it->first << std::endl;
            std::cout << "      client_body_temp_path: " << loc_it->second.client_body_temp_path << std::endl;
            std::cout << "      autoindex: " << loc_it->second.autoindex << std::endl;
            std::cout << "      cgi_pass: " << loc_it->second.cgi_pass << std::endl;
        
            std::cout << "      allowed_methods: ";
            print_vector_with_space(loc_it->second.allowed_methods);

            std::cout << "      redirect: " << loc_it->second.redirect.first << " to " << loc_it->second.redirect.second << std::endl;
            std::cout << "      root: " << loc_it->second.root << std::endl; 
            std::cout << "      index_pages: ";
            print_vector_with_space(loc_it->second.index_pages);
        
        }
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
    for (it = this->_servers.begin(); it != this->_servers.end(); it++) {
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

int ParserConfFile::check_brackets_integrity() {
    int brackets_count = 0;
    std::vector<std::string>::iterator it;
    for (it = _tokens.begin(); it != _tokens.end(); it++) {
        if (*it == "{")
            brackets_count++;
        else if (*it == "}")
            brackets_count--;
        if (brackets_count < 0) {
            std::cerr << "Error: Config File Invalid Sintax" << std::endl;
            return 1;
        }
    }
    if (brackets_count != 0) {
        std::cerr << "Error: Config File Invalid Sintax" << std::endl;
        return 1;
    }
    return 0;
}

bool ParserConfFile::is_directive(const std::string &to_find) {
    std::string arr[] = {"listen", "server_name", "error_page",
    "location", "root", "client_max_body_size", "index", "autoindex", "allowed_methods", 
    "cgi_pass", "client_body_temp_path"};
    std::vector<std::string> directives;
    directives.assign(arr, arr + sizeof(arr) / sizeof(arr[0])); 
    if (std::find(directives.begin(), directives.end(), to_find) != directives.end()){
        return true;
    }
    return false;
}

int ParserConfFile::get_server_count(void) {
    return this->_servers.size();
}