#pragma once
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/unordered_map.hpp>

namespace fs = boost::filesystem;

namespace duplicate_finder
{
    struct FileInfo {
        boost::filesystem::path path;
        mutable std::vector<std::string> hashes;
    };

    std::string hash_block(const std::vector<char>& block, const std::string& algo);
    void read_block(FileInfo& fi, size_t block_index, size_t block_size, const std::string& algo);
    bool compare_files(FileInfo& f1, FileInfo& f2, size_t block_size, const std::string& algo);

    bool match_mask(const std::string& filename, const std::vector<std::string>& masks);
}