#include "graph.hpp"
#include <sstream>
#include <algorithm>
#include <stack>
#include <queue>

namespace iris::core {

Graph::Graph(const BuildConfig& config) {
    build_from_config(config);
}

void Graph::build_from_config(const BuildConfig& config) {
    for (const auto& target : config.targets) {
        GraphNode node;
        node.name = target.name;
        
        switch (target.type) {
            case TargetType::Executable: node.type = "executable"; break;
            case TargetType::Library: node.type = "library"; break;
            case TargetType::SharedLibrary: node.type = "shared_library"; break;
            default: node.type = "target"; break;
        }

        node.dependencies = target.dependencies;
        add_node(node);

        for (const auto& dep : target.dependencies) {
            add_edge(target.name, dep);
        }
    }
}

void Graph::add_node(const GraphNode& node) {
    m_nodes[node.name] = node;
}

void Graph::add_edge(const std::string& from, const std::string& to) {
    m_edges[from].insert(to);
}

std::vector<std::string> Graph::topological_sort() const {
    std::map<std::string, int> in_degree;
    for (const auto& [name, _] : m_nodes) {
        in_degree[name] = 0;
    }

    for (const auto& [from, tos] : m_edges) {
        for (const auto& to : tos) {
            in_degree[to]++;
        }
    }

    std::queue<std::string> queue;
    for (const auto& [name, degree] : in_degree) {
        if (degree == 0) {
            queue.push(name);
        }
    }

    std::vector<std::string> result;
    while (!queue.empty()) {
        std::string node = queue.front();
        queue.pop();
        result.push_back(node);

        if (m_edges.count(node)) {
            for (const auto& neighbor : m_edges.at(node)) {
                in_degree[neighbor]--;
                if (in_degree[neighbor] == 0) {
                    queue.push(neighbor);
                }
            }
        }
    }

    return result;
}

bool Graph::has_cycle() const {
    std::set<std::string> visited;
    std::set<std::string> rec_stack;

    for (const auto& [name, _] : m_nodes) {
        if (dfs_cycle(name, visited, rec_stack)) {
            return true;
        }
    }
    return false;
}

bool Graph::dfs_cycle(const std::string& node,
                      std::set<std::string>& visited,
                      std::set<std::string>& rec_stack) const {
    if (rec_stack.count(node)) return true;
    if (visited.count(node)) return false;

    visited.insert(node);
    rec_stack.insert(node);

    if (m_edges.count(node)) {
        for (const auto& neighbor : m_edges.at(node)) {
            if (dfs_cycle(neighbor, visited, rec_stack)) {
                return true;
            }
        }
    }

    rec_stack.erase(node);
    return false;
}

std::string Graph::to_dot() const {
    std::stringstream ss;
    ss << "digraph IrisBuild {\n";
    ss << "  rankdir=LR;\n";
    ss << "  node [shape=box, style=filled];\n\n";

    for (const auto& [name, node] : m_nodes) {
        ss << "  \"" << name << "\" [";
        if (node.type == "executable") {
            ss << "fillcolor=\"#90EE90\"";
        } else if (node.type == "library") {
            ss << "fillcolor=\"#87CEEB\"";
        } else {
            ss << "fillcolor=\"#FFE4B5\"";
        }
        ss << "];\n";
    }

    ss << "\n";

    for (const auto& [from, tos] : m_edges) {
        for (const auto& to : tos) {
            ss << "  \"" << from << "\" -> \"" << to << "\";\n";
        }
    }

    ss << "}\n";
    return ss.str();
}

std::string Graph::to_json() const {
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"nodes\": [\n";

    bool first = true;
    for (const auto& [name, node] : m_nodes) {
        if (!first) ss << ",\n";
        first = false;
        ss << "    {\"name\": \"" << name << "\", \"type\": \"" << node.type << "\"}";
    }

    ss << "\n  ],\n";
    ss << "  \"edges\": [\n";

    first = true;
    for (const auto& [from, tos] : m_edges) {
        for (const auto& to : tos) {
            if (!first) ss << ",\n";
            first = false;
            ss << "    {\"from\": \"" << from << "\", \"to\": \"" << to << "\"}";
        }
    }

    ss << "\n  ]\n";
    ss << "}\n";
    return ss.str();
}

} // namespace iris::core