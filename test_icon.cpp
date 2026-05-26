#include <iostream>
#include <filesystem>
#include <horizon/IconThemeLookup.hpp>

int main() {
    std::string p = "/usr/share/icons/hicolor/scalable/actions/pluma-fileopen.svg";
    std::cout << "exists directly? " << std::filesystem::exists(p) << std::endl;
    std::string path = horizon::IconThemeLookup::find_icon("pluma-fileopen", 32, "hicolor");
    std::cout << "pluma-fileopen path: " << path << std::endl;
    return 0;
}
