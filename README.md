# Fun4All based E1039 offline software

A doxygen page of the seaquest-offline is [here](https://e1039-collaboration.github.io/seaquest-offline-doc/index.html) hosted by GitHub Pages.

Each package included a "CMakeLists.txt"
Using the shell script "build.sh" to build all packages. 

```
git clone https://github.com/E1039-Collaboration/seaquest-offline.git
mkdir <build-dir>
mkdir <install-dir>
export MY_INSTALL=<install-dir> # Using the MY_INSTALL variable is suggested
# change build.sh the 'src' line to use your source dir
cd <build-dir>
./build.sh # this will build all packages in correct order
./build.sh <package-name> # this will build a specific package 
```
