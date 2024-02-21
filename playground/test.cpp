// #include <map>
// #include <iostream>

// int main() {
//     std::map<std::string, std::string> active_connects;
//     active_connects["1 one"] = "first";
//     active_connects["2 two"] = "second";
//     active_connects["3 three"] = "third";
//     active_connects["4 four"] = "fourth";


//     std::map<std::string, std::string>::iterator it;

//     for( it = active_connects.begin(); it != active_connects.end(); it++) {
//         if (it->first == "2 two" || it->first == "4 four"){
//             active_connects.erase(it);

//             it = active_connects.begin();
//         }
//     }

//     for( it = active_connects.begin(); it != active_connects.end(); ++it) {
//         std::cout << it->first << " : " << it->second << std::endl;
//     }
// }

#include <iostream>
#include <ctime>

int main() {
    // Record the start time
    clock_t startTime = clock();

    // Perform some operations or tasks
    for (int i = 0; i < 1000000; ++i) {
        // Do some work
    }

    // Record the end time
    clock_t endTime = clock();

    // Calculate the elapsed time
    double elapsedTime = static_cast<double>(endTime - startTime) / CLOCKS_PER_SEC;

    std::cout << "Elapsed time: " << endTime << " seconds." << std::endl;

    return 0;
}
