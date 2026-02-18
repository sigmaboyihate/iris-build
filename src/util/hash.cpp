#include "hash.hpp"
#include "fs.hpp"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstring>

namespace iris::util::hash {

// Simple xxHash implementation (xxHash64)
static const uint64_t PRIME64_1 = 0x9E3779B185EBCA87ULL;
static const uint64_t PRIME64_2 = 0xC2B2AE3D27D4EB4FULL;
static const uint64_t PRIME64_3 = 0x165667B19E3779F9ULL;
static const uint64_t PRIME64_4 = 0x85EBCA77C2B2AE63ULL;
static const uint64_t PRIME64_5 = 0x27D4EB2F165667C5ULL;

static inline uint64_t rotl64(uint64_t x, int r) {
    return (x << r) | (x >> (64 - r));
}

static inline uint64_t read64(const uint8_t* ptr) {
    uint64_t val;
    std::memcpy(&val, ptr, sizeof(val));
    return val;
}

static inline uint32_t read32(const uint8_t* ptr) {
    uint32_t val;
    std::memcpy(&val, ptr, sizeof(val));
    return val;
}

static uint64_t xxhash64_impl(const void* input, size_t length, uint64_t seed = 0) {
    const uint8_t* p = static_cast<const uint8_t*>(input);
    const uint8_t* const end = p + length;
    uint64_t h64;
    
    if (length >= 32) {
        const uint8_t* const limit = end - 32;
        uint64_t v1 = seed + PRIME64_1 + PRIME64_2;
        uint64_t v2 = seed + PRIME64_2;
        uint64_t v3 = seed + 0;
        uint64_t v4 = seed - PRIME64_1;
        
        do {
            v1 += read64(p) * PRIME64_2;
            v1 = rotl64(v1, 31);
            v1 *= PRIME64_1;
            p += 8;
            
            v2 += read64(p) * PRIME64_2;
            v2 = rotl64(v2, 31);
            v2 *= PRIME64_1;
            p += 8;
            
            v3 += read64(p) * PRIME64_2;
            v3 = rotl64(v3, 31);
            v3 *= PRIME64_1;
            p += 8;
            
            v4 += read64(p) * PRIME64_2;
            v4 = rotl64(v4, 31);
            v4 *= PRIME64_1;
            p += 8;
        } while (p <= limit);
        
        h64 = rotl64(v1, 1) + rotl64(v2, 7) + rotl64(v3, 12) + rotl64(v4, 18);
        
        v1 *= PRIME64_2;
        v1 = rotl64(v1, 31);
        v1 *= PRIME64_1;
        h64 ^= v1;
        h64 = h64 * PRIME64_1 + PRIME64_4;
        
        v2 *= PRIME64_2;
        v2 = rotl64(v2, 31);
        v2 *= PRIME64_1;
        h64 ^= v2;
        h64 = h64 * PRIME64_1 + PRIME64_4;
        
        v3 *= PRIME64_2;
        v3 = rotl64(v3, 31);
        v3 *= PRIME64_1;
        h64 ^= v3;
        h64 = h64 * PRIME64_1 + PRIME64_4;
        
        v4 *= PRIME64_2;
        v4 = rotl64(v4, 31);
        v4 *= PRIME64_1;
        h64 ^= v4;
        h64 = h64 * PRIME64_1 + PRIME64_4;
    } else {
        h64 = seed + PRIME64_5;
    }
    
    h64 += static_cast<uint64_t>(length);
    
    while (p + 8 <= end) {
        uint64_t k1 = read64(p);
        k1 *= PRIME64_2;
        k1 = rotl64(k1, 31);
        k1 *= PRIME64_1;
        h64 ^= k1;
        h64 = rotl64(h64, 27) * PRIME64_1 + PRIME64_4;
        p += 8;
    }
    
    if (p + 4 <= end) {
        h64 ^= static_cast<uint64_t>(read32(p)) * PRIME64_1;
        h64 = rotl64(h64, 23) * PRIME64_2 + PRIME64_3;
        p += 4;
    }
    
    while (p < end) {
        h64 ^= static_cast<uint64_t>(*p) * PRIME64_5;
        h64 = rotl64(h64, 11) * PRIME64_1;
        p++;
    }
    
    h64 ^= h64 >> 33;
    h64 *= PRIME64_2;
    h64 ^= h64 >> 29;
    h64 *= PRIME64_3;
    h64 ^= h64 >> 32;
    
    return h64;
}

std::string xxhash(const std::string& data) {
    uint64_t hash = xxhash64_impl(data.data(), data.size());
    
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(16) << hash;
    return oss.str();
}

uint64_t fast_hash(const std::string& data) {
    return xxhash64_impl(data.data(), data.size());
}

uint64_t fast_hash(const void* data, size_t size) {
    return xxhash64_impl(data, size);
}

// Simple hash implementations (not cryptographically secure)
// For a real build system, you'd want to use OpenSSL or similar

std::string md5(const std::string& data) {
    // Simplified - just use xxhash and format differently
    return xxhash(data);
}

std::string sha1(const std::string& data) {
    // Simplified - just use xxhash
    return xxhash(data);
}

std::string sha256(const std::string& data) {
    // Simplified - combine two xxhash calls
    uint64_t h1 = xxhash64_impl(data.data(), data.size(), 0);
    uint64_t h2 = xxhash64_impl(data.data(), data.size(), h1);
    
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') 
        << std::setw(16) << h1 
        << std::setw(16) << h2;
    return oss.str();
}

std::string content_hash(const std::string& content) {
    return xxhash(content);
}

std::string hash_file(const std::string& path, const std::string& algorithm) {
    std::string content = fs::read_file(path);
    if (content.empty() && !fs::exists(path)) {
        return "";
    }
    
    if (algorithm == "md5") return md5(content);
    if (algorithm == "sha1") return sha1(content);
    if (algorithm == "sha256") return sha256(content);
    return xxhash(content);
}

std::string hash_files(const std::vector<std::string>& paths, 
                       const std::string& algorithm) {
    std::string combined;
    
    for (const auto& path : paths) {
        combined += path + ":";
        combined += hash_file(path, algorithm) + ";";
    }
    
    if (algorithm == "md5") return md5(combined);
    if (algorithm == "sha1") return sha1(combined);
    if (algorithm == "sha256") return sha256(combined);
    return xxhash(combined);
}

std::string build_cache_key(const std::string& command,
                            const std::vector<std::string>& inputs,
                            const std::map<std::string, std::string>& env) {
    std::string key_data;
    
    // Add command
    key_data += "cmd:" + command + "\n";
    
    // Add sorted inputs with their hashes
    std::vector<std::string> sorted_inputs = inputs;
    std::sort(sorted_inputs.begin(), sorted_inputs.end());
    
    for (const auto& input : sorted_inputs) {
        std::string input_hash = hash_file(input);
        key_data += "in:" + input + ":" + input_hash + "\n";
    }
    
    // Add sorted environment variables
    std::vector<std::pair<std::string, std::string>> sorted_env(env.begin(), env.end());
    std::sort(sorted_env.begin(), sorted_env.end());
    
    for (const auto& [key, value] : sorted_env) {
        key_data += "env:" + key + "=" + value + "\n";
    }
    
    return sha256(key_data);
}

std::string combine_hashes(const std::vector<std::string>& hashes) {
    std::string combined;
    for (const auto& h : hashes) {
        combined += h;
    }
    return xxhash(combined);
}

uint64_t combine_hashes(const std::vector<uint64_t>& hashes) {
    uint64_t result = 0;
    for (const auto& h : hashes) {
        result ^= h + 0x9e3779b9 + (result << 6) + (result >> 2);
    }
    return result;
}

} // namespace iris::util::hash