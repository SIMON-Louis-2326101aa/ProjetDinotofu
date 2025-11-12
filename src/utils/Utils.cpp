#include "utils/Utils.hpp"
#include <iostream>
#include <sstream>
#include <cctype>
#include <algorithm>
#ifdef _WIN32
  #include <windows.h>
#endif
#include <cstdlib>

namespace Utils {

std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace((unsigned char)s[a])) ++a;
    while (b > a && std::isspace((unsigned char)s[b-1])) --b;
    return s.substr(a, b-a);
}

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> out;
    std::stringstream ss(s);
    std::string part;
    while (std::getline(ss, part, delim)) out.push_back(part);
    return out;
}

int askInt(const std::string& prompt, int defaut) {
    std::cout << prompt << " [" << defaut << "]: ";
    int v;
    if (!(std::cin >> v)) {
        std::cin.clear();
        std::cin.ignore(1024, '\n');
        return defaut;
    }
    std::cin.ignore(1024, '\n'); // flush line
    return v;
}

std::string askLine(const std::string& prompt) {
    std::cout << prompt;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

void clearScreen() {
#ifdef _WIN32
    (void)system("cls");
#else
    (void)system("clear");
#endif
}


void pauseConsole(const std::string& message) {
    std::cout << message;
    std::string dummy;
    std::getline(std::cin, dummy);
}

std::string toLower(std::string s) {
    for (auto& c : s) c = (char)std::tolower((unsigned char)c);
    return s;
}

} // namespace Utils
