# iris-build
simple build system with memorable and easy syntax, which uses ninja fast parser!! keyword buzzwor

iris config (to check options for iris)
iris out (build args here, like buildtype etc)
iris -Disable-(subproject) to not add bloat
iris optimize (do in build directory, ontop of iris out sets preconfigued -march=native stuff for cpu and yeah makes better)

for example, manual building would look like:
```BASH
$ mkdir build 
$ cd build
$ iris config .. 
$ iris out -build=release -Disable-libloat -Enable-usefullib -strip -With-optionallib -Disable-Docs .. && iris optimize profile speed
$ ninja
$ sudo ninja install 
``` 


