#include "duplicate_finder.hpp"
#include <boost/unordered_map.hpp>
#include "utils.hpp"
#include <set>

using namespace duplicate_finder;

std::vector<DuplicateGroup> findDuplicates(const ScanParams& p)
{
    boost::unordered_map<std::size_t, std::vector<FileInfo>> size_groups;

    // канонизация exclude
    std::vector<fs::path> expaths;
    for (const auto& ed : p.excludeDirs) {
        try { expaths.push_back(fs::canonical(ed)); }
        catch (...) { expaths.push_back(fs::path(ed)); }
    }

    for (const auto& root_str : p.scanDirs) {
        fs::path root(root_str);
        if (!fs::exists(root) || !fs::is_directory(root)) {
            // в библиотеке лучше кидать исключение или логировать через callback
            continue;
        }

        fs::path root_can;
        try { root_can = fs::canonical(root); }
        catch (...) { root_can = root; }

        for (fs::recursive_directory_iterator it(root), end; it != end; ++it) {
            const fs::path path = it->path();

            // исключённые директории
            bool excluded = false;
            fs::path p_can;
            try { p_can = fs::canonical(path); }
            catch (...) { p_can = path; }

            for (const auto& ex : expaths) {
                if (p_can == ex ||
                    (!ex.empty() && p_can.string().find(ex.string() + fs::path::preferred_separator) == 0)) {
                    excluded = true;
                    break;
                }
            }
            if (excluded && fs::is_directory(path)) {
                it.disable_recursion_pending();
            }

            // глубина
            if (p.level == 0 && fs::is_directory(path)) {
                it.disable_recursion_pending();
            } else if (p.level > 0 && it.depth() >= p.level && fs::is_directory(path)) {
                it.disable_recursion_pending();
            }

            if (fs::is_regular_file(path)) {
                if (p.level == 0) {
                    fs::path parent_can;
                    try { parent_can = fs::canonical(path.parent_path()); }
                    catch (...) { parent_can = path.parent_path(); }
                    if (parent_can != root_can) continue;
                }

                std::size_t sz = 0;
                try { sz = fs::file_size(path); }
                catch (...) { continue; }

                if (sz >= p.minSize && match_mask(path.filename().string(), p.masks)) {
                    size_groups[sz].push_back(FileInfo{path, {}});
                }
            }
        }
    }

    // union-find
    boost::unordered_map<fs::path, int> rank_map;
    boost::unordered_map<fs::path, fs::path> parent_map;

    boost::disjoint_sets<
        boost::associative_property_map<boost::unordered_map<fs::path,int>>,
        boost::associative_property_map<boost::unordered_map<fs::path,fs::path>>
    > ds(
        boost::make_assoc_property_map(rank_map),
        boost::make_assoc_property_map(parent_map)
    );

    for (auto& [size, files] : size_groups) {
        if (files.size() < 2) continue;
        for (auto& f : files) {
            ds.make_set(f.path);
        }
    }

    for (auto& [size, files] : size_groups) {
        if (files.size() < 2) continue;
        for (size_t i = 0; i < files.size(); ++i) {
            for (size_t j = i + 1; j < files.size(); ++j) {
                if (compare_files(files[i], files[j], p.blockSize, p.algo)) {
                    ds.union_set(files[i].path, files[j].path);
                }
            }
        }
    }

    boost::unordered_map<fs::path, std::set<fs::path>> groups;
    for (auto& [size, files] : size_groups) {
        if (files.size() < 2) continue;
        for (auto& f : files) {
            fs::path root = ds.find_set(f.path);
            groups[root].insert(f.path);
        }
    }

    std::vector<DuplicateGroup> result;
    result.reserve(groups.size());
    for (auto& [root, group] : groups) {
        if (group.size() > 1) {
            DuplicateGroup g;
            // размер можно взять из size_groups или file_size(root)
            g.size = fs::file_size(*group.begin());
            g.files.assign(group.begin(), group.end());
            result.push_back(std::move(g));
        }
    }
    return result;
}
