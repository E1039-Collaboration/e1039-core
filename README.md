# Fun4All based E1039 offline software

A doxygen page of the seaquest-offline is [here](https://e1039-collaboration.github.io/seaquest-offline-doc/index.html) hosted by GitHub Pages.

## install

Each package included a 'CMakeLists.txt'
Using the shell script "build.sh" to build all packages. 

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


