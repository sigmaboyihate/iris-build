#include "cache.hpp"
#include "../util/hash.hpp"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

namespace iris::core {

Cache::Cache(const std::string& cache_dir) : m_cache_dir(cache_dir) {
    fs::create_directories(m_cache_dir);
    load();
}

Cache::~Cache() {
    if (m_dirty) {
        save();
    }
}

void Cache::set_cache_dir(const std::string& dir) {
    m_cache_dir = dir;
    fs::create_directories(m_cache_dir);
}

bool Cache::is_up_to_date(const std::string& target,
                          const std::string& input_hash,
                          const std::string& command_hash) const {
    auto it = m_entries.find(target);
    if (it == m_entries.end()) {
        return false;
    }

    const auto& entry = it->second;
    if (entry.input_hash != input_hash || entry.command_hash != command_hash) {
        return false;
    }

    // Check if outputs still exist
    for (const auto& output : entry.outputs) {
        if (!fs::exists(output)) {
            return false;
        }
    }

    return true;
}

void Cache::store(const std::string& target,
                  const std::string& input_hash,
                  const std::string& command_hash,
                  const std::vector<std::string>& outputs) {
    CacheEntry entry;
    entry.input_hash = input_hash;
    entry.command_hash = command_hash;
    entry.outputs = outputs;
    entry.timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    m_entries[target] = entry;
    m_dirty = true;
}

std::optional<CacheEntry> Cache::get(const std::string& target) const {
    auto it = m_entries.find(target);
    if (it != m_entries.end()) {
        return it->second;
    }
    return std::nullopt;
}

void Cache::invalidate(const std::string& target) {
    m_entries.erase(target);
    m_dirty = true;
}

void Cache::clear() {
    m_entries.clear();
    m_dirty = true;
}

std::string Cache::get_manifest_path() const {
    return m_cache_dir + "/manifest.json";
}

void Cache::load() {
    std::string path = get_manifest_path();
    if (!fs::exists(path)) {
        return;
    }

    std::ifstream file(path);
    // Simple JSON parsing (would use a proper library in production)
    // For now, just load what we saved
    std::string line;
    std::string current_target;
    
    while (std::getline(file, line)) {
        // Simplified parsing
        if (line.find("\"target\":") != std::string::npos) {
            size_t start = line.find(":") + 3;
            size_t end = line.rfind("\"");
            current_target = line.substr(start, end - start);
        }
        // ... more parsing would go here
    }
}

void Cache::save() const {
    std::ofstream file(get_manifest_path());
    file << "{\n";
    file << "  \"entries\": [\n";

    bool first = true;
    for (const auto& [target, entry] : m_entries) {
        if (!first) file << ",\n";
        first = false;

        file << "    {\n";
        file << "      \"target\": \"" << target << "\",\n";
        file << "      \"input_hash\": \"" << entry.input_hash << "\",\n";
        file << "      \"command_hash\": \"" << entry.command_hash << "\",\n";
        file << "      \"timestamp\": " << entry.timestamp << ",\n";
        file << "      \"outputs\": [";
        
        for (size_t i = 0; i < entry.outputs.size(); i++) {
            if (i > 0) file << ", ";
            file << "\"" << entry.outputs[i] << "\"";
        }
        
        file << "]\n";
        file << "    }";
    }

    file << "\n  ]\n";
    file << "}\n";
}

} // namespace iris::core