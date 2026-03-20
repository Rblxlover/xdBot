#include "utils.hpp"

std::time_t Utils::getFileCreationTime(const std::filesystem::path &path) {
    if (!std::filesystem::exists(path)) return 0;
    
    auto ftime = std::filesystem::last_write_time(path);
    return std::chrono::system_clock::to_time_t(
        std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - decltype(ftime)::clock::now() + std::chrono::system_clock::now()
        )
    );
} // i really wanted to use asp here but seems like you cannot do this with asp :( - slideglide

std::string Utils::formatTime(std::time_t time) {
    auto tm = geode::localtime(time);
    return fmt::format("{:%Y-%m-%d %H:%M:%S}", tm);
}

std::string Utils::getTexture() {
    cocos2d::ccColor3B color = Mod::get()->getSettingValue<cocos2d::ccColor3B>("background_color");
    
    std::string texture = color == ccc3(51, 68, 153) ? "GJ_square02.png" : "GJ_square06.png";
    
    return texture;
}

std::string Utils::getSimplifiedString(std::string str) {
    if (str.find(".") == std::string::npos) return str;
    
    while (str.back() == '0') {
        str.pop_back();
        if (str.empty())
        break;
    }
    
    if (!str.empty()) {
        if (str.back() == '.') str.pop_back();
    }
    
    return str;
}

void Utils::setBackgroundColor(geode::NineSlice *bg) {
    cocos2d::ccColor3B color = Mod::get()->getSettingValue<cocos2d::ccColor3B>("background_color");
    
    if (color == ccc3(51, 68, 153)) {
        color = ccc3(255, 255, 255);
        bg->setColor(color);
    }
}

