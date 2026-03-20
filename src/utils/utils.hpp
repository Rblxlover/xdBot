#pragma once

#include "../includes.hpp"

class Utils {
public:
    static std::time_t getFileCreationTime(const std::filesystem::path& path);

    static std::string formatTime(std::time_t time);

    static std::string getTexture();

    static std::string getSimplifiedString(std::string str);

    static void setBackgroundColor(geode::NineSlice* bg);
};