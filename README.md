# Iris Build System

Iris is a build system for C and C++ with its own build language. The goal is a workflow similar to Meson: a fast `setup` step that generates backend files (Ninja or Make), followed by a quick incremental `build`.

Iris projects are described in an `iris.build` file using a small, readable language that mixes Ruby-style blocks (`do ... end`), Python-like expressions, and a few C like conveniences.

## Status

This repository is early stage. The language, file format, and backend generation are still evolving, and breaking changes are expected.

## Build Iris (the tool)

Requirements:

* C++17 compiler (GCC/Clang/MSVC)
* Make (or a compatible build environment)

Build:

```bash
make
```

Run:

```bash
./bin/iris --help
```

Install (optional):

```bash
sudo make install
```

## Quick start (using Iris on a project)

Create a project:

```bash
iris init --name hello --lang cpp
```

Configure:

```bash
iris setup .
```

Build:

```bash
iris build
```

Run:

```bash
iris run
```

Clean:

```bash
iris clean
iris clean --all
```

## The `iris.build` file

An Iris project is described by an `iris.build` file at the repository root.

Minimal example:

```ruby
project "hello" do
    version = "1.0.0"
    lang = :cpp
    std = "c++17"
end

executable "hello" do
    sources = ["src/main.cpp"]
end
```

### Blocks

Iris uses blocks for most configuration:

* `project "name" do ... end`
* `compiler do ... end`
* `executable "name" do ... end`
* `library "name" do ... end`
* `shared_library "name" do ... end`
* `task :name do ... end`

Example:

```ruby
project "app" do
    version = "0.1.0"
    license = "MIT"
    lang = :cpp
    std = "c++20"
end

compiler do
    warnings = ["-Wall", "-Wextra", "-Wpedantic"]
end

executable "app" do
    sources = glob("src/**/*.cpp")
    includes = ["include/"]
end
```

### Variables and assignment

Variables are created by assignment:

```ruby
name = "myapp"
flags = ["-O2"]
```

You can append to arrays with `+=`:

```ruby
flags += ["-DDEBUG"]
```

### Conditionals

`if` blocks and `unless` blocks are supported:

```ruby
if buildtype == "debug" do
    flags = ["-g", "-O0", "-DDEBUG"]
end

unless file_exists("config.h") do
    warning("config.h not found")
end
```

### Loops

A simple `for` loop iterates arrays:

```ruby
for f in glob("src/*.cpp") do
    print("source: " + f)
end
```

### Functions

Define functions with `fn`:

```ruby
fn add_warnings(extra) do
    warnings = ["-Wall", "-Wextra"]
    warnings += extra
    return warnings
end
```

Call functions like you would expect:

```ruby
compiler do
    warnings = add_warnings(["-Wpedantic"])
end
```

### Targets

#### Executables

```ruby
executable "app" do
    sources = glob("src/**/*.cpp")
    includes = ["include/"]
    flags = ["-O2"]
end
```

#### Libraries

```ruby
library "mylib" do
    sources = glob("lib/**/*.cpp")
    includes = ["include/"]
end
```

#### Shared libraries

```ruby
shared_library "mylib" do
    sources = glob("src/**/*.cpp")
    flags = ["-fPIC"]
    link_flags = ["-shared"]
end
```

### Compiler configuration

The `compiler` block sets global flags:

```ruby
compiler do
    warnings = ["-Wall", "-Wextra"]
    
    if buildtype == "release" do
        flags = ["-O3", "-DNDEBUG"]
    end
    
    if buildtype == "debug" do
        flags = ["-g", "-O0", "-DDEBUG"]
    end
end
```

`buildtype` is provided by `iris setup` (`debug` by default).

### File globs

Use `glob()` to collect source files:

```ruby
sources = glob("src/**/*.cpp")
```

## Built-in functions

Common built-ins:

* `glob(pattern)` -> array of matched files
* `file_exists(path)` -> boolean
* `read_file(path)` -> string
* `write_file(path, content)` -> boolean
* `dirname(path)`, `basename(path)`, `extension(path)`
* `platform()` -> `"linux"`, `"macos"`, `"windows"`
* `arch()` -> `"x86_64"`, `"arm64"`, etc.
* `env(name)` -> environment variable string (or empty)
* `shell(cmd)` -> command output as string
* `run(cmd)` -> exit code
* `print(...)`, `warning(msg)`, `error(msg)`
* `len(x)`, `join(arr, sep)`, `split(str, sep)`, `contains(arr, elem)`

Notes:

* `shell()` is intended for querying tools (capturing output).
* `run()` is intended for running commands where you only care about exit code.

## CLI

### `iris init`

Create a starter project.

```bash
iris init --name myproject --lang cpp
```

Options (current):

* `--name <name>`
* `--lang <c|cpp|mixed>`
* `--lib` or `--exe`

### `iris setup`

Configure a build directory and generate backend files.

```bash
iris setup . --builddir=build --backend=ninja --buildtype=debug
```

Options (current):

* `--builddir <dir>` (default: `build`)
* `--backend <ninja|make>` (default: `ninja`)
* `--buildtype <debug|release|minsize|relwithdebinfo>` (default: `debug`)
* `--prefix <path>` (default: `/usr/local`)

### `iris build`

Build from an existing configured build directory.

```bash
iris build -j8
iris build --target=myapp
```

Options (current):

* `-j, --jobs <n>`
* `--target <name>`
* `--verbose`
* `--clean`

### `iris run`

Build and run an executable.

```bash
iris run
iris run --target=myapp --args="--help"
```

### `iris clean`

Clean build products:

```bash
iris clean
iris clean --all
```

## Example project

There is a sample project under:

```
examples/hello-cpp/
```

Try it:

```bash
cd examples/hello-cpp
../../bin/iris setup .
../../bin/iris build
../../bin/iris run
```

## Project layout

Typical Iris project:

```
myproject/
    iris.build
    src/
    include/
    build/          (generated)
```

`build/` is generated by `iris setup`. Do not edit generated backend files by hand.

## Environment variables

* `CC`, `CXX` to override compilers
* `CFLAGS`, `CXXFLAGS`, `LDFLAGS` for extra flags
* `NO_COLOR` to disable colored output
* `IRIS_CACHE_DIR` to override the cache directory (when supported)

## License

GPLv3. See `LICENSE`.

By the Iris Build Team (sol and clad)

# Contributions welcome! we will try to merge pull requests as much as possible, if viable changes have been done.
