#include "config.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <ostream>
#include <regex>
#include <string>
#include <vector>


Config::Config(std::string filename): default_entries({
        {"MET", "PuppiMET"}, 
        {"infile", "infile.root"},
        {"cutflow", "all"},
        {"eventlist", "none"}
    }) {
    read_config_file(filename);
}

void Config::write_out_file(const std::unordered_map<std::string, std::string> & in_map, std::string filename) {
    std::ofstream file(filename);

    for (auto it = in_map.begin(); it != in_map.end(); ++it) {
        file << it->first << " " << it->second;
        if (std::next(it) != in_map.end()) {
            file << "\n";
        }
    }

    file << std::flush;
    file.close();
}


void Config::read_config_file(std::string filename) {

    // if the file does not exist, we create a preinitialized one containing all default entries
    if (!std::filesystem::exists(filename)) {
        write_out_file(default_entries, filename);
        config_entries = default_entries;
    }

    std::ifstream read_file(filename);
    std::string content;

    while (std::getline(read_file, content)) {

        std::stringstream ss;
        std::regex e("[\\s]+");
        ss << std::regex_replace(content, e, "\x1d");

        std::vector<std::string> args;
        std::string token;
        while (std::getline(ss, token, '\x1d')) {
            args.push_back(token);
        }
        
        if (args.size() > 0 && args[0][0] == '#') {
            // in this case we have a comment line, we don't need to do anything here
            continue;
        }

        if (args.size() != 2) {
            //TODO: add proper exception
            assert(false);
        }

        config_entries.insert(std::pair(args[0], args[1]));
    }

}

std::string Config::get_argument(std::string in) {
    return config_entries[in];
}
