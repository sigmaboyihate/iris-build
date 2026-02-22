#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <map>

namespace iris::util::hash {

// hash algorithms
std::string md5(const std::string& data);
std::string sha1(const std::string& data);
std::string sha256(const std::string& data);
std::string xxhash(const std::string& data);

// file hashing
std::string hash_file(const std::string& path, const std::string& algorithm = "xxhash");
std::string hash_files(const std::vector<std::string>& paths, 
                       const std::string& algorithm = "xxhash");

// content hash
std::string content_hash(const std::string& content);

// combined hash for build cache key
std::string build_cache_key(const std::string& command,
                            const std::vector<std::string>& inputs,
                            const std::map<std::string, std::string>& env = {});

// fast hash for quick comparisons
uint64_t fast_hash(const std::string& data);
uint64_t fast_hash(const void* data, size_t size);

// combine hashes
std::string combine_hashes(const std::vector<std::string>& hashes);
uint64_t combine_hashes(const std::vector<uint64_t>& hashes);

} // namespace iris::util::hash
