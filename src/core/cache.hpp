#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>

namespace iris::core {

struct CacheEntry {
    std::string input_hash;
    std::string command_hash;
    std::vector<std::string> outputs;
    int64_t timestamp;
};

class Cache {
public:
    Cache(const std::string& cache_dir = ".iris-cache");
    ~Cache();

    void set_cache_dir(const std::string& dir);

    bool is_up_to_date(const std::string& target,
                       const std::string& input_hash,
                       const std::string& command_hash) const;

    void store(const std::string& target,
               const std::string& input_hash,
               const std::string& command_hash,
               const std::vector<std::string>& outputs);

    std::optional<CacheEntry> get(const std::string& target) const;

    void invalidate(const std::string& target);
    void clear();

    void load();
    void save() const;

private:
    std::string m_cache_dir;
    std::map<std::string, CacheEntry> m_entries;
    bool m_dirty = false;

    std::string get_manifest_path() const;
};

} // namespace iris::core