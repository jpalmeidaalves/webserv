#include <algorithm>
#include <iostream>
#include <map>
#include <vector>

int main(int argc, char **argv) {
    if (argc != 2)
        return 1;
    std::multimap<std::string, std::string> error_pages;
    error_pages.insert({"404", "custom_404.html"});
    error_pages.insert({"500", "custom_50x.html"});
    error_pages.insert({"502", "custom_50x.html"});
    error_pages.insert({"500", "BUG.html"});
    error_pages.insert({"503", "custom_50x.html"});
    error_pages.insert({"505", "custom_50x.html"});

    std::multimap<std::string, std::string>::iterator it;
    for (it = error_pages.begin(); it != error_pages.end(); it++) {
        if (it->first == argv[1]) {

            std::cout << it->first << " " << it->second << std::endl;
            break;
        }
    }
}
// error_page 500 502 503 504 /custom_50x.html;

// int main() {
//     std::map<std::vector<std::string>, std::string> error_pages;

//     std::vector<std::string> v;
//     v.push_back("500");
//     v.push_back("502");
//     v.push_back("503");

//     error_pages.insert({v, 500});

//     std::vector<std::string> b;
//     b.push_back("400");

//     error_pages.insert({b, "custom_404.html"});

//     std::string target = "502";

//     std::map<std::vector<std::string>, std::string>::iterator it;
//     for (it = error_pages.begin(); it != error_pages.end(); it++) {
//         if (std::find(it->first.begin(), it->first.end(), target) != it->first.end()) {
//             std::cout << "found: " << it->second << std::endl;
//         }
//     }
// }