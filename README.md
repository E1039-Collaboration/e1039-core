# Fun4All based E1039 offline software

A doxygen page of the seaquest-offline is [here](https://haiwangyu.github.io/seaquest-offline-doc/) hosted by GitHub Pages.

## Build
### cmake
Each package included a "CMakeLists.txt"
Using the shell script "build.sh" to build all packages. 

### automake
Use [automake](https://www.gnu.org/software/automake/), [autoconf](https://www.gnu.org/software/autoconf/autoconf.html) and [libtool](https://www.gnu.org/software/libtool/) to build the code. Each package should contain 3 files: 'autogen.sh', 'configure.ac' and 'MakeFile.am'

```
mkdir [path_to_build]
cd [path_to_build]
[path_to_package]/autogen.sh --prefix=[path_to_install]
make -j 4
make install
```

Follow the package order listed in the "build.sh"
