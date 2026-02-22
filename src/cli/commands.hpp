#pragma once

#include <string>
#include <map>
#include <vector>

namespace iris::cli::commands {

int cmd_setup(const std::map<std::string, std::string>& options,
              const std::vector<std::string>& positional);

int cmd_build(const std::map<std::string, std::string>& options,
              const std::vector<std::string>& positional);

int cmd_clean(const std::map<std::string, std::string>& options,
              const std::vector<std::string>& positional);

int cmd_init(const std::map<std::string, std::string>& options,
             const std::vector<std::string>& positional);

int cmd_run(const std::map<std::string, std::string>& options,
            const std::vector<std::string>& positional);

int cmd_test(const std::map<std::string, std::string>& options,
             const std::vector<std::string>& positional);

int cmd_info(const std::map<std::string, std::string>& options,
             const std::vector<std::string>& positional);

int cmd_graph(const std::map<std::string, std::string>& options,
              const std::vector<std::string>& positional);

int cmd_install(const std::map<std::string, std::string>& options,
                const std::vector<std::string>& positional);

} // namespace iris::cli::commands
