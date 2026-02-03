#pragma once

#include <vector>
#include <string>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

struct ScanParams {
    std::vector<std::string> scanDirs;
    std::vector<std::string> excludeDirs;
    int level = 0;                       // 0, -1, >0
    std::size_t minSize = 1;
    std::vector<std::string> masks;
    std::size_t blockSize = 1024;
    std::string algo = "crc32";          // "crc32" или "md5"
};

struct DuplicateGroup {
    std::size_t size;
    std::vector<fs::path> files;
};
    
std::vector<DuplicateGroup> findDuplicates(const ScanParams& params);
