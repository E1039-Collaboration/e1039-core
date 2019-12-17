# Fun4All based E1039 core software

When you are looking for the information on the classes of this package, you could refer to the doxygen page [here](https://e1039-collaboration.github.io/e1039-doc/index.html) hosted by GitHub Pages.

## Necessity of this package

This package contains the _core_ functions of the E1039 software.
A pre-compiled library of this package is available on main E1039 servers and ready to use.
Thus you often need not download nor build it for analysis, and instead download and execute the "e1039-analysis" package.

## Prerequisite

This package depends on the E1039 resource and share packages as well as some system-wide packages.
You are recommended to use the following E1039 servers, which have been already set up properly;
* spinquestgpvm01 ... for offline analysis
* spinquestana1   ... for online analysis

If you want to use another computer (like your laptop), 
you could refer to [this wiki page](https://github.com/E1039-Collaboration/e1039-wiki/wiki/Install-the-core-software-from-scratch), although it is not quite up-to-date.

## Download

You move to a working directory and check out the repository;
```
cd /path/to/your_working_directory
git clone https://github.com/E1039-Collaboration/e1039-core.git
```
If you are a member of the GitHub E1039 group, you better use the following command to obtain the write access;
```
git clone git@github.com:E1039-Collaboration/e1039-core.git
```

## Build and Install

You can first try the following commands to learn the overall procedure.
They should succeed on the supported servers.
```
cd /path/to/directory_where_you_download_e1039-core
./script/setup-install.sh auto
source ../core-inst/this-e1039.sh
./build.sh
```

It takes 5-10 minutes.
Lots of installed files appear in "../core-inst".

### Details

When you open a new shell environment (i.e. text terminal), you have to source "this-e1039.sh".

The script "setup-install.sh" sets up the environment to install, build and run this package.
It warns you if your computer is not supported for the automated setup.

The argument "auto" of "setup-install.sh" means that the installation directory is selected automatically, which is "../core-inst".
If you like to manually select it (such as `~/e1039/inst/core`), you can type it instead of "auto".

A directory that includes 'CMakeLists.txt' under "e1039-core" forms a sub-package.
The script "build.sh" builds all the sub-packages in the right order.
You can build one specific package (when it is modified) by
```
./build.sh <sub-package-name>
```

## Execute

Following the Fun4All framework, we use a ROOT macro to execute a function(s) defined in this package.
The e1039-analysis package contains various user-level macros.

This package itself contains some ROOT macros, but most of them should not be run by multiple users.
