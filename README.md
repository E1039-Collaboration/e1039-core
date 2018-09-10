# Fun4All based E1039 offline software

A doxygen page of the seaquest-offline is [here](https://e1039-collaboration.github.io/seaquest-offline-doc/index.html) hosted by GitHub Pages.

## Install

### Prerequisite
* To install on seaquestgpvm machines, simply source this macro to get all dependencies in place.
```
/e906/user/yuhw/setup.sh
```
* To install from scratch without any dependent software installed, refer to [this wiki page](https://github.com/E1039-Collaboration/e1039-wiki/wiki/Install-the-offline-software-from-scratch).

### Build
Each package included a 'CMakeLists.txt'.
A shell script "build.sh" is provided to build all packages. 

Check out the repository
```
git clone https://github.com/E1039-Collaboration/seaquest-offline.git
```


Make a build and install folder
```
mkdir <build-dir>
mkdir <install-dir>
cd <build-dir>
```

Using the MY_INSTALL variable is suggested, for this variable is used in 'CMakeLists.txt' files.
You may also put this line in your login macro.
```
export MY_INSTALL=<install-dir>
```

Then change the 'src' line of 'build.sh' to use your source path.

To build and install all packages
```
./build.sh
```

To build one specific package
```
./build.sh <package-name>
```


