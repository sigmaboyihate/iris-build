#pragma once

#include "engine.hpp"
#include <string>
#include <vector>
#include <map>
#include <set>

namespace iris::core {

    struct GraphNode {
        std::string name;
        std::string type;  // target, source, object, etc.
        std::vector<std::string> dependencies;
    };

    class Graph {
    public:
        Graph(const BuildConfig& config);
        ~Graph() = default;

        void add_node(const GraphNode& node);
        void add_edge(const std::string& from, const std::string& to);

        std::vector<std::string> topological_sort() const;
        bool has_cycle() const;

        std::string to_dot() const;
        std::string to_json() const;

    private:
        std::map<std::string, GraphNode> m_nodes;
        std::map<std::string, std::set<std::string>> m_edges;

        void build_from_config(const BuildConfig& config);
        bool dfs_cycle(const std::string& node,
                       std::set<std::string>& visited,
                       std::set<std::string>& rec_stack) const;
    };

} // namespace iris::core
