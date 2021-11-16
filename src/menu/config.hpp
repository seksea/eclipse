#include <map>
#include <string>
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <utility>
#include "imgui/imgui.h"

#define CONFIGINT(name) Config::getConfigItem(name, Config::INT)->intValue
#define CONFIGFLOAT(name) Config::getConfigItem(name, Config::INT)->floatValue
#define CONFIGBOOL(name) Config::getConfigItem(name, Config::BOOL)->boolValue
#define CONFIGSTR(name) Config::getConfigItem(name, Config::STR)->strValue
#define CONFIGCOL(name) Config::getConfigItem(name, Config::COLOR)->colValue

namespace Config {
    enum ConfigItemType {
        INT,
        FLOAT,
        BOOL,
        STR,
        COLOR
    };

	class ConfigItem {
	public:
        ConfigItem(int value) {
            type = Config::INT;
            intValue = value;
        }
        ConfigItem(float value) {
            type = Config::FLOAT;
            floatValue = value;
        }
        ConfigItem(bool value) {
            type = Config::BOOL;
            boolValue = value;
        }
        ConfigItem(std::string value) {
            type = Config::STR;
            strValue = value;
        }
        ConfigItem(ImColor value) {
            type = Config::COLOR;
            colValue = value;
        }
        ConfigItemType type;
        int intValue = -1;
        float floatValue = -1;
        bool boolValue = false;
        std::string strValue = "";
        ImColor colValue = ImColor(255, 255, 255, 255);
	};

    inline std::map<std::string, ConfigItem> configItems;

    // Gets a config item, if it does not exist it will create it with default values
    inline ConfigItem* getConfigItem(const char* name, ConfigItemType type) {
        std::map<std::string, ConfigItem>::iterator it = configItems.find(name);
        if (it != configItems.end())
            return &it->second;

        switch (type) {
            case Config::INT: configItems.insert(std::pair<std::string, ConfigItem>(name, 0)); break;
            case Config::FLOAT: configItems.insert(std::pair<std::string, ConfigItem>(name, 0.f)); break;
            case Config::BOOL: configItems.insert(std::pair<std::string, ConfigItem>(name, false)); break;
            case Config::STR: configItems.insert(std::pair<std::string, ConfigItem>(name, "")); break;
            case Config::COLOR: configItems.insert(std::pair<std::string, ConfigItem>(name, ImColor(0, 0, 0))); break;
        }

        return getConfigItem(name, type);
    }

    inline std::vector<std::string> cfgFiles;
    inline char selectedCfg[128];
    inline void refreshConfigList() {
        cfgFiles.clear();
        char path[512];
        strcpy(path, getenv("HOME"));
        strcat(path, "/.csgo-cheat/");
        std::filesystem::create_directory(path);
        strcat(path, "configs/");
        std::filesystem::create_directory(path);
        for (const auto &entry : std::filesystem::directory_iterator(path)) {
            cfgFiles.push_back(entry.path().string().substr(strlen(path)));
        }
        std::sort(cfgFiles.begin(), cfgFiles.end());
    }

    inline void saveConfig(const char* filename) {
        nlohmann::json j;

        for (auto configItem : configItems) {
            switch (configItem.second.type) {
            case Config::INT: j[configItem.first] = configItem.second.intValue; break;
            case Config::FLOAT: j[configItem.first] = configItem.second.floatValue; break;
            case Config::BOOL: j[configItem.first] = configItem.second.boolValue; break;
            case Config::STR: j[configItem.first] = configItem.second.strValue; break;
            case Config::COLOR: {
                j[configItem.first]["r"] = configItem.second.colValue.Value.x;
                j[configItem.first]["g"] = configItem.second.colValue.Value.y;
                j[configItem.first]["b"] = configItem.second.colValue.Value.z;
                j[configItem.first]["a"] = configItem.second.colValue.Value.w; 
                break;
            }
            };
        }

        char path[512];
        strcpy(path, getenv("HOME"));
        strcat(path, "/.csgo-cheat/");
        std::filesystem::create_directory(path);
        strcat(path, "configs/");
        std::filesystem::create_directory(path);

        char filepath[512];
        strcpy(filepath, path);
        strcat(filepath, filename);
        std::ofstream file(filepath);

        if (filename[0] == '\0')
            return;

        file << j.dump().c_str();

        file.close();
    }

    inline void loadConfig(const char* filename) {
        char path[512];
        strcpy(path, getenv("HOME"));
        strcat(path, "/.csgo-cheat/");
        std::filesystem::create_directory(path);
        strcat(path, "configs/");
        std::filesystem::create_directory(path);

        char filepath[512];
        strcpy(filepath, path);
        strcat(filepath, filename);

        if (filename[0] == '\0' || (!std::filesystem::exists(filepath)))
            return;
        
        std::ifstream file(filepath);

        nlohmann::json j = nlohmann::json::parse(file);
        for (nlohmann::json::iterator it = j.begin(); it != j.end(); ++it) {
            if (it.value().is_number()) {
                CONFIGINT(it.key().c_str()) = it.value();
            }
            if (it.value().is_number_float()) {
                CONFIGFLOAT(it.key().c_str()) = it.value();
            }
            if (it.value().is_boolean()) {
                CONFIGBOOL(it.key().c_str()) = it.value();
            }
            if (it.value().is_string()) {
                CONFIGSTR(it.key().c_str()) = it.value();
            }
            if (it.value().is_structured()) {
                CONFIGCOL(it.key().c_str()).Value.x = it.value()["r"];
                CONFIGCOL(it.key().c_str()).Value.y = it.value()["g"];
                CONFIGCOL(it.key().c_str()).Value.z = it.value()["b"];
                CONFIGCOL(it.key().c_str()).Value.w = it.value()["a"];
            }
        }
    }
}