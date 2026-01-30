#include "utils.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>

#include <boost/crc.hpp>
#include <boost/uuid/detail/md5.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
// Реализация hash_block и read_block...
namespace duplicate_finder
{
    std::string hash_block(const std::vector<char>& block, const std::string& algo) {
        if (algo == "crc32") {
            boost::crc_32_type result;
            result.process_bytes(block.data(), block.size());
            return std::to_string(result.checksum());
        } else if (algo == "md5") {
            // md5 из boost::uuids::detail::md5
            boost::uuids::detail::md5 hash;
            boost::uuids::detail::md5::digest_type digest;
            hash.process_bytes(block.data(), block.size());
            hash.get_digest(digest);
            std::ostringstream oss;
            for (int i = 0; i < 4; ++i) oss << std::hex << digest[i];
            return oss.str();
        }
        return {};
    }

    void read_block(FileInfo& fi, size_t block_index, size_t block_size, const std::string& algo) {
        if (fi.hashes.size() > block_index) return; // уже считано

        std::ifstream in(fi.path.string(), std::ios::binary);
        if (!in) return;

        in.seekg(block_index * block_size);
        std::vector<char> buf(block_size, 0);
        in.read(buf.data(), block_size);

        fi.hashes.push_back(hash_block(buf, algo));
    }
    bool compare_files(FileInfo& f1, FileInfo& f2, size_t block_size, const std::string& algo) {
        std::ifstream in1(f1.path.string(), std::ios::binary);
        std::ifstream in2(f2.path.string(), std::ios::binary);
        if (!in1 || !in2) return false;

        in1.seekg(0, std::ios::end);
        in2.seekg(0, std::ios::end);
        std::size_t size1 = in1.tellg();
        std::size_t size2 = in2.tellg();
        if (size1 != size2) return false; // разные размеры → точно не дубликаты

        std::size_t blocks = (size1 + block_size - 1) / block_size;

        for (std::size_t i = 0; i < blocks; ++i) {
            read_block(f1, i, block_size, algo);
            read_block(f2, i, block_size, algo);

            if (f1.hashes[i] != f2.hashes[i]) {
                return false; // нашли различие
            }
        }
        return true; // все блоки совпали
    }
    bool match_mask(const std::string& filename, const std::vector<std::string>& masks) {
        if (masks.empty()) return true;
        for (const auto& m : masks) {
            // если маска начинается с "*." → проверяем окончание
            if (m.size() > 2 && m[0] == '*' && m[1] == '.') {
                if (boost::algorithm::iends_with(filename, m.substr(1))) {
                    return true;
                }
            } else {
                // иначе ищем вхождение подстроки (регистронезависимо)
                if (boost::algorithm::icontains(filename, m)) {
                    return true;
                }
            }
        }
        return false;
    }
}