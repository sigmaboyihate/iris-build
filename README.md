# Iris Build System

Iris is a build system for C and C++ projects. It uses a custom build language to describe project structure and compilation settings, then generates Ninja or Make files for fast, incremental builds.

The build language combines Ruby style blocks, Python like readability, and familiar C syntax. Projects are configured through an `iris.build` file in the project root.

Developed by solaris and clad.

---

## Table of Contents

1. [Installation](#installation)
2. [Quick Start](#quick-start)
3. [The Build Language](#the-build-language)
   - [Syntax Overview](#syntax-overview)
   - [Data Types](#data-types)
   - [Variables](#variables)
   - [Operators](#operators)
   - [Control Flow](#control-flow)
   - [Functions](#functions)
   - [Blocks](#blocks)
4. [Project Configuration](#project-configuration)
   - [Project Block](#project-block)
   - [Compiler Block](#compiler-block)
   - [Targets](#targets)
5. [Built-in Functions](#built-in-functions)
   - [File System](#file-system-functions)
   - [System Information](#system-information-functions)
   - [String Manipulation](#string-manipulation-functions)
   - [Output](#output-functions)
6. [Command Line Interface](#command-line-interface)
   - [iris init](#iris-init)
   - [iris setup](#iris-setup)
   - [iris build](#iris-build)
   - [iris run](#iris-run)
   - [iris clean](#iris-clean)
   - [iris install](#iris-install)
   - [iris test](#iris-test)
   - [iris info](#iris-info)
   - [iris graph](#iris-graph)
7. [Environment Variables](#environment-variables)
8. [Project Structure](#project-structure)
9. [Examples](#examples)
10. [Troubleshooting](#troubleshooting)
11. [License](#license)

---

## Installation

### Requirements

- C++17 compatible compiler (GCC 8+, Clang 7+, or MSVC 2019+)
- GNU Make
- Ninja (recommended) or Make as build backend

### Building from Source

```bash
git clone https://github.com/sigmaboyihate/iris-build
cd iris
make -j$(nproc)
````

The binary is placed in `bin/iris`.

### System Installation

```bash
sudo make install
```

This installs to `/usr/local/bin`. To install elsewhere:

```bash
./bin/iris install --prefix=/path/to/prefix
```

### User Installation

```bash
./bin/iris install --prefix=$HOME/.local
```

Ensure `$HOME/.local/bin` is in your PATH.

---

## Quick Start

### Create a New Project

```bash
iris init --name myproject --lang cpp
cd myproject
```

This creates a basic project structure with an `iris.build` file and sample source.

### Configure the Build

```bash
iris setup .
```

This parses `iris.build` and generates build files in the `build/` directory.

### Compile

```bash
iris build
```

### Run

```bash
iris run
```

### Clean

```bash
iris clean
```

---

## The Build Language

Iris uses a custom language for build configuration. Files use the `.build` extension, with `iris.build` being the main project file.

### Syntax Overview

The language uses blocks delimited by `do` and `end`:

```ruby
project "name" do
    version = "1.0.0"
end
```

Statements are separated by newlines. Semicolons are not required.

Comments begin with `#`:

```ruby
# This is a comment
version = "1.0.0"  # Inline comment
```

### Data Types

#### Strings

Double-quoted text:

```ruby
name = "hello"
path = "src/main.cpp"
```

Escape sequences: `\n`, `\t`, `\\`, `\"`

#### Numbers

Integer and floating point:

```ruby
count = 42
ratio = 3.14
negative = -10
```

#### Booleans

```ruby
enabled = true
disabled = false
```

#### Symbols

Prefixed with a colon, used for enumerated values:

```ruby
lang = :cpp
type = :release
```

Common symbols: `:c`, `:cpp`, `:mixed`, `:debug`, `:release`

#### Arrays

Ordered collections in square brackets:

```ruby
sources = ["main.cpp", "util.cpp", "parser.cpp"]
flags = ["-Wall", "-Wextra"]
empty = []
```

Arrays can span multiple lines:

```ruby
sources = [
    "src/main.cpp",
    "src/util.cpp",
    "src/parser.cpp"
]
```

#### Hash Maps

Key value pairs in curly braces:

```ruby
options = {
    optimize: true,
    debug: false
}
```

### Variables

Variables are created on first assignment:

```ruby
name = "myapp"
version = "1.0.0"
```

Variables are scoped to their containing block. Inner blocks can access outer variables.

Append to arrays with `+=`:

```ruby
flags = ["-Wall"]
flags += ["-Wextra", "-Wpedantic"]
```

### Operators

#### Arithmetic

| Operator | Description                    |
| -------- | ------------------------------ |
| `+`      | Addition, string concatenation |
| `-`      | Subtraction                    |
| `*`      | Multiplication                 |
| `/`      | Division                       |
| `%`      | Modulo                         |

#### Comparison

| Operator | Description           |
| -------- | --------------------- |
| `==`     | Equal                 |
| `!=`     | Not equal             |
| `<`      | Less than             |
| `>`      | Greater than          |
| `<=`     | Less than or equal    |
| `>=`     | Greater than or equal |

#### Logical

| Operator | Description |
| -------- | ----------- |
| `and`    | Logical AND |
| `or`     | Logical OR  |
| `not`    | Logical NOT |

### Control Flow

#### If Statements

```ruby
if condition do
    # statements
end
```

With else:

```ruby
if buildtype == "debug" do
    flags = ["-g", "-O0"]
else
    flags = ["-O3"]
end
```

Chained conditions:

```ruby
if platform() == "linux" do
    flags = ["-pthread"]
else if platform() == "macos" do
    flags = ["-framework", "CoreFoundation"]
else
    flags = []
end
```

#### Unless Statements

Executes when condition is false:

```ruby
unless file_exists("config.h") do
    warning("config.h not found, using defaults")
end
```

#### For Loops

Iterate over arrays:

```ruby
for file in glob("src/*.cpp") do
    print("Found: " + file)
end
```

### Functions

#### Defining Functions

```ruby
fn function_name(param1, param2) do
    # function body
    return result
end
```

Example:

```ruby
fn make_flags(debug) do
    flags = ["-Wall", "-Wextra"]
    if debug do
        flags += ["-g", "-O0"]
    else
        flags += ["-O3"]
    end
    return flags
end
```

#### Calling Functions

```ruby
my_flags = make_flags(true)
```

### Blocks

Blocks define configuration sections. Each block type has specific recognized fields.

#### Block Types

| Block                              | Purpose                           |
| ---------------------------------- | --------------------------------- |
| `project "name" do ... end`        | Project metadata                  |
| `compiler do ... end`              | Global compiler settings          |
| `executable "name" do ... end`     | Executable target                 |
| `library "name" do ... end`        | Static library target             |
| `shared_library "name" do ... end` | Shared/dynamic library target     |
| `task :name do ... end`            | Custom build task                 |
| `dependency "name" do ... end`     | External dependency configuration |

---

## Project Configuration

### Project Block

Defines project metadata. Required in every `iris.build` file.

```ruby
project "myproject" do
    version = "1.0.0"
    license = "MIT"
    lang = :cpp
    std = "c++17"
end
```

#### Fields

| Field     | Type   | Description                                            |
| --------- | ------ | ------------------------------------------------------ |
| `version` | string | Project version                                        |
| `license` | string | License identifier                                     |
| `lang`    | symbol | Language: `:c`, `:cpp`, or `:mixed`                    |
| `std`     | string | Language standard: `"c17"`, `"c++17"`, `"c++20"`, etc. |

### Compiler Block

Sets global compiler flags applied to all targets.

```ruby
compiler do
    warnings = ["-Wall", "-Wextra", "-Wpedantic"]
    
    if buildtype == "debug" do
        flags = ["-g", "-O0", "-DDEBUG"]
    end
    
    if buildtype == "release" do
        flags = ["-O3", "-DNDEBUG", "-march=native"]
    end
end
```

#### Fields

| Field      | Type   | Description           |
| ---------- | ------ | --------------------- |
| `flags`    | array  | Compiler flags        |
| `warnings` | array  | Warning flags         |
| `cc`       | string | C compiler override   |
| `cxx`      | string | C++ compiler override |

#### Built-in Variables

These variables are available within `compiler` and target blocks:

| Variable    | Description                                                                           |
| ----------- | ------------------------------------------------------------------------------------- |
| `buildtype` | Build type from `iris setup`: `"debug"`, `"release"`, `"minsize"`, `"relwithdebinfo"` |

### Targets

Targets define what gets built: executables, static libraries, or shared libraries.

#### Executable

```ruby
executable "myapp" do
    sources = ["src/main.cpp", "src/app.cpp"]
    includes = ["include/"]
    flags = ["-DAPP_NAME=\"myapp\""]
    link_flags = []
    deps = []
end
```

#### Static Library

```ruby
library "mylib" do
    sources = glob("lib/**/*.cpp")
    includes = ["include/"]
end
```

#### Shared Library

```ruby
shared_library "myshared" do
    sources = glob("src/**/*.cpp")
    includes = ["include/"]
    flags = ["-fPIC"]
end
```

#### Target Fields

| Field        | Type  | Description                                        |
| ------------ | ----- | -------------------------------------------------- |
| `sources`    | array | Source files (supports glob patterns)              |
| `includes`   | array | Include directories                                |
| `flags`      | array | Compiler flags for this target                     |
| `link_flags` | array | Linker flags                                       |
| `deps`       | array | Dependencies (other targets or external libraries) |
| `defines`    | array | Preprocessor definitions                           |

---

## Built-in Functions

### File System Functions

#### glob(pattern)

Returns an array of file paths matching the pattern.

```ruby
sources = glob("src/**/*.cpp")
headers = glob("include/**/*.hpp")
c_files = glob("src/*.c")
```

Pattern syntax:

* `*` matches any characters except `/`
* `**` matches any characters including `/` (recursive)
* `?` matches a single character

#### file_exists(path)

Returns `true` if the file or directory exists.

```ruby
if file_exists("config.h") do
    flags += ["-DHAS_CONFIG"]
end
```

#### read_file(path)

Returns the contents of a file as a string.

```ruby
version = read_file("VERSION").strip()
```

#### write_file(path, content)

Writes a string to a file. Returns `true` on success.

```ruby
write_file("build_info.h", "#define BUILD_TIME \"" + shell("date") + "\"")
```

#### dirname(path)

Returns the directory portion of a path.

```ruby
dir = dirname("src/util/helper.cpp")  # "src/util"
```

#### basename(path)

Returns the filename portion of a path.

```ruby
name = basename("src/util/helper.cpp")  # "helper.cpp"
```

#### extension(path)

Returns the file extension.

```ruby
ext = extension("main.cpp")  # ".cpp"
```

### System Information Functions

#### platform()

Returns the current operating system.

| Return Value | Operating System |
| ------------ | ---------------- |
| `"linux"`    | Linux            |
| `"macos"`    | macOS            |
| `"windows"`  | Windows          |
| `"freebsd"`  | FreeBSD          |

```ruby
if platform() == "linux" do
    flags += ["-pthread"]
end

if platform() == "macos" do
    link_flags += ["-framework", "CoreFoundation"]
end
```

#### arch()

Returns the CPU architecture.

| Return Value | Architecture |
| ------------ | ------------ |
| `"x86_64"`   | 64-bit x86   |
| `"x86"`      | 32-bit x86   |
| `"arm64"`    | 64-bit ARM   |
| `"arm"`      | 32-bit ARM   |

```ruby
if arch() == "arm64" do
    flags += ["-mcpu=native"]
end
```

#### env(name)

Returns the value of an environment variable, or empty string if not set.

```ruby
home = env("HOME")
custom_prefix = env("MYAPP_PREFIX")

if custom_prefix != "" do
    includes += [custom_prefix + "/include"]
end
```

#### shell(command)

Executes a shell command and returns its output.

```ruby
git_hash = shell("git rev-parse --short HEAD")
date = shell("date +%Y-%m-%d")
```

#### run(command)

Executes a shell command and returns its exit code.

```ruby
result = run("pkg-config --exists libssl")
if result == 0 do
    flags += shell("pkg-config --cflags libssl").split(" ")
end
```

### String Manipulation Functions

#### len(value)

Returns the length of a string or array.

```ruby
count = len(sources)
length = len(name)
```

#### join(array, separator)

Joins array elements into a string.

```ruby
flags_str = join(["-Wall", "-Wextra"], " ")  # "-Wall -Wextra"
```

#### split(string, separator)

Splits a string into an array.

```ruby
parts = split("one,two,three", ",")  # ["one", "two", "three"]
pkg_flags = split(shell("pkg-config --cflags gtk+-3.0"), " ")
```

#### contains(array, element)

Returns `true` if the array contains the element.

```ruby
if contains(sources, "debug.cpp") do
    flags += ["-DHAS_DEBUG"]
end
```

### Output Functions

#### print(...)

Prints a message to standard output.

```ruby
print("Building version " + version)
print("Platform: " + platform())
```

#### warning(message)

Prints a warning message.

```ruby
warning("Deprecated option used")
```

#### error(message)

Prints an error message and aborts the build.

```ruby
if not file_exists("required.h") do
    error("required.h is missing")
end
```

---

## Command Line Interface

### iris init

Creates a new project with a basic structure.

```bash
iris init [OPTIONS]
```

#### Options

| Option              | Description                   | Default                |
| ------------------- | ----------------------------- | ---------------------- |
| `-n, --name <name>` | Project name                  | Current directory name |
| `-l, --lang <lang>` | Language: `c`, `cpp`, `mixed` | `cpp`                  |
| `--lib`             | Create a library project      |                        |
| `--exe`             | Create an executable project  | (default)              |

#### Examples

```bash
iris init --name myapp --lang cpp
iris init --name mathlib --lang c --lib
```

#### Generated Files

```
myproject/
    iris.build
    src/
        main.cpp (or main.c)
    include/
    .gitignore
```

### iris setup

Parses the `iris.build` file and generates build system files.

```bash
iris setup <source_dir> [OPTIONS]
```

#### Options

| Option                 | Description                    | Default      |
| ---------------------- | ------------------------------ | ------------ |
| `-b, --builddir <dir>` | Build output directory         | `build`      |
| `--backend <backend>`  | Build backend: `ninja`, `make` | `ninja`      |
| `--buildtype <type>`   | Build type                     | `debug`      |
| `-p, --prefix <path>`  | Installation prefix            | `/usr/local` |

#### Build Types

| Type             | Description                                  |
| ---------------- | -------------------------------------------- |
| `debug`          | Debug symbols, no optimization (`-g -O0`)    |
| `release`        | Full optimization, no debug (`-O3 -DNDEBUG`) |
| `relwithdebinfo` | Optimization with debug symbols (`-O2 -g`)   |
| `minsize`        | Optimize for size (`-Os`)                    |

#### Examples

```bash
iris setup .
iris setup . --builddir=build-release --buildtype=release
iris setup /path/to/project --backend=make
```

### iris build

Compiles the project.

```bash
iris build [OPTIONS]
```

#### Options

| Option             | Description           | Default       |
| ------------------ | --------------------- | ------------- |
| `-j, --jobs <n>`   | Parallel jobs         | Auto-detected |
| `--target <name>`  | Build specific target | All targets   |
| `--builddir <dir>` | Build directory       | `build`       |
| `-v, --verbose`    | Verbose output        |               |
| `-c, --clean`      | Clean before building |               |

#### Examples

```bash
iris build
iris build -j8
iris build --target=mylib
iris build --builddir=build-release
```

### iris run

Builds the project (if needed) and runs an executable.

```bash
iris run [OPTIONS]
```

#### Options

| Option             | Description       | Default                |
| ------------------ | ----------------- | ---------------------- |
| `--target <name>`  | Executable to run | First executable found |
| `--builddir <dir>` | Build directory   | `build`                |
| `--args <args>`    | Arguments to pass |                        |

#### Examples

```bash
iris run
iris run --target=myapp
iris run --args="--config=test.conf"
```

### iris clean

Removes build artifacts.

```bash
iris clean [OPTIONS]
```

#### Options

| Option             | Description                             |
| ------------------ | --------------------------------------- |
| `-a, --all`        | Remove entire build directory and cache |
| `--builddir <dir>` | Build directory to clean                |

#### Examples

```bash
iris clean
iris clean --all
```

### iris install

Installs built targets to the system.

```bash
iris install [OPTIONS]
```

#### Options

| Option                | Description                           | Default      |
| --------------------- | ------------------------------------- | ------------ |
| `-p, --prefix <path>` | Installation prefix                   | `/usr/local` |
| `--builddir <dir>`    | Build directory                       | `build`      |
| `--destdir <dir>`     | Destination directory (for packaging) |              |
| `-n, --dry-run`       | Show what would be installed          |              |
| `--strip`             | Strip binaries before installing      |              |

#### Examples

```bash
iris install
iris install --prefix=$HOME/.local
iris install --dry-run
sudo iris install --prefix=/usr/local --strip
```

#### Installation Paths

| Target Type    | Destination     |
| -------------- | --------------- |
| Executable     | `<prefix>/bin/` |
| Static library | `<prefix>/lib/` |
| Shared library | `<prefix>/lib/` |

### iris test

Runs project tests.

```bash
iris test [OPTIONS]
```

#### Options

| Option                | Description                | Default |
| --------------------- | -------------------------- | ------- |
| `-v, --verbose`       | Verbose test output        |         |
| `--filter <pattern>`  | Run tests matching pattern |         |
| `--timeout <seconds>` | Test timeout               | `60`    |
| `--builddir <dir>`    | Build directory            | `build` |

### iris info

Displays project information.

```bash
iris info [OPTIONS]
```

#### Options

| Option      | Description          |
| ----------- | -------------------- |
| `--targets` | List all targets     |
| `--options` | List all options     |
| `--deps`    | Show dependency tree |

### iris graph

Generates a dependency graph.

```bash
iris graph [OPTIONS]
```

#### Options

| Option                | Description                  | Default     |
| --------------------- | ---------------------------- | ----------- |
| `-o, --output <file>` | Output file                  | `graph.dot` |
| `--format <format>`   | Output format: `dot`, `json` | `dot`       |

#### Examples

```bash
iris graph
iris graph --output=deps.dot
dot -Tpng deps.dot -o deps.png
```

---

## Environment Variables

| Variable         | Description                       |
| ---------------- | --------------------------------- |
| `CC`             | C compiler                        |
| `CXX`            | C++ compiler                      |
| `CFLAGS`         | Additional C compiler flags       |
| `CXXFLAGS`       | Additional C++ compiler flags     |
| `LDFLAGS`        | Additional linker flags           |
| `NO_COLOR`       | Disable colored output when set   |
| `IRIS_CACHE_DIR` | Override cache directory location |

The compiler variables (`CC`, `CXX`) override any compiler specified in the `iris.build` file. Flag variables (`CFLAGS`, etc.) are appended to flags from the build file.

---

## Project Structure

A typical Iris project:

```
myproject/
    iris.build          # Build configuration
    src/                # Source files
        main.cpp
        util.cpp
    include/            # Header files
        util.hpp
    lib/                # Library sources (if applicable)
    tests/              # Test sources
    build/              # Generated (do not edit)
        build.ninja
        iris-config.json
        obj/
```

The `build/` directory is created by `iris setup`. Do not edit files in this directory manually; they are regenerated on each setup.

---

## Examples

### Minimal C Project

```ruby
project "hello" do
    version = "1.0.0"
    lang = :c
    std = "c17"
end

executable "hello" do
    sources = ["main.c"]
end
```

### C++ Application with Libraries

```ruby
project "myapp" do
    version = "2.0.0"
    lang = :cpp
    std = "c++20"
end

compiler do
    warnings = ["-Wall", "-Wextra", "-Wpedantic"]
    
    if buildtype == "debug" do
        flags = ["-g", "-O0", "-fsanitize=address"]
        link_flags = ["-fsanitize=address"]
    end
    
    if buildtype == "release" do
        flags = ["-O3", "-DNDEBUG", "-flto"]
        link_flags = ["-flto"]
    end
end

library "core" do
    sources = glob("lib/core/**/*.cpp")
    includes = ["include/"]
end

library "utils" do
    sources = glob("lib/utils/**/*.cpp")
    includes = ["include/"]
end

executable "myapp" do
    sources = glob("src/**/*.cpp")
    includes = ["include/"]
    deps = ["core", "utils"]
end
```

### Cross-Platform Configuration

```ruby
project "crossplatform" do
    version = "1.0.0"
    lang = :cpp
    std = "c++17"
end

compiler do
    warnings = ["-Wall", "-Wextra"]
    
    if platform() == "linux" do
        flags = ["-pthread"]
        link_flags = ["-pthread"]
    end
    
    if platform() == "macos" do
        flags = ["-mmacosx-version-min=10.15"]
    end
    
    if platform() == "windows" do
        flags = ["-DWIN32", "-D_WINDOWS"]
    end
end

executable "app" do
    sources = glob("src/common/**/*.cpp")
    
    if platform() == "linux" do
        sources += glob("src/platform/linux/**/*.cpp")
    end
    
    if platform() == "macos" do
        sources += glob("src/platform/macos/**/*.cpp")
    end
    
    if platform() == "windows" do
        sources += glob("src/platform/windows/**/*.cpp")
    end
    
    includes = ["include/"]
end
```

### Library with Public Headers

```ruby
project "mathlib" do
    version = "1.0.0"
    lang = :cpp
    std = "c++17"
end

library "math" do
    sources = glob("src/**/*.cpp")
    includes = ["include/"]
    
    public_headers = glob("include/**/*.hpp")
end

executable "math_test" do
    sources = ["tests/main.cpp"]
    includes = ["include/"]
    deps = ["math"]
end
```

### Using External Libraries

```ruby
project "webapp" do
    version = "1.0.0"
    lang = :cpp
    std = "c++17"
end

compiler do
    # pkg-config integration
    if run("pkg-config --exists openssl") == 0 do
        flags = split(shell("pkg-config --cflags openssl"), " ")
    else
        error("OpenSSL not found")
    end
end

executable "server" do
    sources = glob("src/**/*.cpp")
    includes = ["include/"]
    
    if run("pkg-config --exists openssl") == 0 do
        link_flags = split(shell("pkg-config --libs openssl"), " ")
    end
end
```

### Custom Tasks

```ruby
project "myproject" do
    version = "1.0.0"
    lang = :cpp
end

executable "myapp" do
    sources = glob("src/**/*.cpp")
end

task :format do
    for file in glob("src/**/*.cpp") do
        run("clang-format -i " + file)
    end
    for file in glob("include/**/*.hpp") do
        run("clang-format -i " + file)
    end
    print("Formatted all source files")
end

task :docs do
    run("doxygen Doxyfile")
    print("Documentation generated in docs/html/")
end

task :package do
    version = "1.0.0"
    run("tar -czf myproject-" + version + ".tar.gz build/myapp")
    print("Package created")
end
```

---

## Troubleshooting

### "No iris.build found"

Ensure you are in the project root directory or specify the correct source path:

```bash
iris setup /path/to/project
```

### "No configuration found in build"

Run `iris setup` before `iris build`:

```bash
iris setup .
iris build
```

### Compiler Not Found

Set the compiler explicitly via environment variable:

```bash
CC=gcc CXX=g++ iris setup .
```

Or specify in `iris.build`:

```ruby
compiler do
    cc = "/usr/bin/gcc-12"
    cxx = "/usr/bin/g++-12"
end
```

### Ninja Not Found

Install Ninja or use the Make backend:

```bash
iris setup . --backend=make
```

### Glob Returns No Files

Check that:

1. The pattern is correct (`**` for recursive, `*` for single level)
2. Files exist in the specified location
3. Paths are relative to the `iris.build` file location

```ruby
# Correct
sources = glob("src/**/*.cpp")

# Wrong (absolute path)
sources = glob("/home/user/project/src/**/*.cpp")
```

### Permission Denied During Install

Use `sudo` for system directories:

```bash
sudo iris install --prefix=/usr/local
```

Or install to user directory:

```bash
iris install --prefix=$HOME/.local
```

---

## License

Iris is licensed under the GNU General Public License v3.0. See the LICENSE file for details.

---

## Contributing

Contributions are welcome. Please submit pull requests with clear descriptions of changes. We aim to review and merge viable contributions promptly.

Report bugs and request features through the issue tracker.

---

Developed by the Iris Build Team: solaris and clad.

Now Im gonna take a damn break - clad
