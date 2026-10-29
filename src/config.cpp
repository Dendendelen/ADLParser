#include "config.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <ostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>


Config::Config(std::string filename): default_entries({
        {"MET", {"PuppiMET", std::string(17,' '), "# What object will be referred to as MET? This object will automatically get a 4-vector initialization"}}, 
        {"infile", {"infile.root", std::string(11,' '), "# Input ROOT file"}},
        {"outfile", {"outfile.root", std::string(9,' '), "# Output ROOT file for histograms"}},
        {"cutflow", {"all", std::string(18,' '), "# Display cutflows? Options are 'all', 'none', 'last'"}},
        {"eventlist", {"none", std::string(15,' '), "# Display event lists? Options are 'all', 'none', 'last'"}},
        {"cutflowout", {"stdout", std::string(12,' '), "# Display cutflow info into separate file? Options are 'stdout', or 'file' which will generate a txt file with the same name as the outfile"}},
        {"eventlistout", {"stdout", std::string(10,' '), "# Display event list info into separate file? Options are 'stdout', or 'file' which will generate a txt file with the same name as the outfile"}},
        {"format", {"nanoaod", std::string(15,' '), "# Format of TTree input. Options are 'nanoAOD' or 'delphes'"}}
    }) {
    read_config_file(filename);
}

void Config::write_out_file(const std::unordered_map<std::string, std::vector<std::string>> & in_map, std::string filename) {
    std::ofstream file(filename);

    for (auto it = in_map.begin(); it != in_map.end(); ++it) {
        file << it->first; 
        for (std::string consequent : it->second) {
            file << " " << consequent;
        }
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
        // config_entries = default_entries;
    }

    std::ifstream read_file(filename);
    std::string content;


    int i = 0;
    
    while (std::getline(read_file, content)) {
        i++;

        std::stringstream ss;
        std::regex e("[\\s]+");
        ss << std::regex_replace(content, e, "\x1d");

        std::vector<std::string> args;
        std::string token;
        while (std::getline(ss, token, '\x1d')) {
            if (token == "") continue;
            if (token[0] == '#') break; //kill the parsing of this line if a comment is found
            args.push_back(token);
        }

        if (args.size() == 0) continue;
        
        if (args[0][0] == '#') {
            // in this case we have a comment line, we don't need to do anything here
            continue;
        }

        if (args.size() != 2) {
            throw std::runtime_error("Incorrect amount of arguments in line " + std::to_string(i) + " of config file");
        }

        config_entries.insert(std::pair(args[0], args[1]));
    }

}

std::string Config::get_argument(std::string in) {
    if (!config_entries.contains(in)) return "";
    std::string config = config_entries[in];
    return config;
}
