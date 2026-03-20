#include <Geode/Geode.hpp>

#include "gdr.hpp"

cocos2d::CCPoint dataFromString(std::string dataString) {
    std::stringstream ss(dataString);
    std::string item;
    std::vector<std::string> vec;

    float xPos = 0.f;
    float yPos = 0.f;

    for (int i = 0; i < 3; i++) {
        std::getline(ss, item, ',');
        if (i == 1)
            xPos = geode::utils::numFromString<float>(item).unwrap();
        else if (i == 2)
            yPos = geode::utils::numFromString<float>(item).unwrap();
    }

    return { xPos, yPos };
};



geode::prelude::VersionInfo getVersion(std::vector<std::string> nums) {
    size_t major = geode::utils::numFromString<int>(nums[0]).unwrapOr(-1);
    size_t minor = geode::utils::numFromString<int>(nums[1]).unwrapOr(-1);
    size_t patch = geode::utils::numFromString<int>(nums[2]).unwrapOr(-1);
    
    geode::prelude::VersionInfo ret(major, minor, patch);

    return ret;
}
