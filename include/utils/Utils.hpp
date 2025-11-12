#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

namespace Utils {

// Nettoyages simples
std::string trim(const std::string& s);
std::vector<std::string> split(const std::string& s, char delim);

// I/O basique (implémentés avec iostream dans le .cpp)
int    askInt(const std::string& prompt, int defaut = 0);
std::string askLine(const std::string& prompt);

// console cross-plateforme (best effort)
void clearScreen();
void pauseConsole(const std::string& message = "Appuie sur Entrée...");

// formatage
std::string toLower(std::string s);

} // namespace Utils

#endif
