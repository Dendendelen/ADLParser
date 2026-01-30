#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>
class Config {
    private:
        std::unordered_map<std::string, std::string> config_entries;

        const std::unordered_map<std::string, std::string> default_entries;
        void write_out_file(const std::unordered_map<std::string, std::string> &in_map, std::string filename);
        void read_config_file(std::string filename);
    public:
        Config(std::string filename);
        std::string get_argument(std::string);


};

#endif