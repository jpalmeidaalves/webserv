// read a file into memory
#include <fstream>  // std::ifstream
#include <iostream> // std::cout

int main() {

    std::ifstream is;

    is.open("test.php", std::ifstream::binary);
    // is.open("empty.txt", std::ifstream::binary);
    if (!is)
        return 1;

    // get length of file:
    is.seekg(0, is.end);
    int length = is.tellg();
    is.seekg(0, is.beg);

    if (!length)
        return 0;

    char *buffer = new char[length];

    std::cout << "Reading " << length << " characters... ";
    // read data as a block:
    is.read(buffer, 1000);

    if (is) {
        std::cout << "all characters read successfully." << std::endl;
        std::cout << buffer << std::endl;

    } else {
        std::cout << "error: only " << is.gcount() << " could be read";
        std::cout << buffer << std::endl;
    }
    is.close();

    // ...buffer contains the entire file...

    delete[] buffer;

    return 0;
}