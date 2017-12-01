# Fun4All based E1039 offline software

A doxygen page of the seaquest-offline is [here](https://haiwangyu.github.io/seaquest-offline-doc/) hosted by GitHub Pages.

## Build
Use [automake](https://www.gnu.org/software/automake/), [autoconf](https://www.gnu.org/software/autoconf/autoconf.html) and [libtool](https://www.gnu.org/software/libtool/) to build the code. Each package should contain 3 files: 'autogen.sh', 'configure.ac' and 'MakeFile.am'

```
mkdir [path_to_build]
cd [path_to_build]
[path_to_package]/autogen.sh --prefix=[path_to_install]
make -j 4
make install
```

Need to build using the the following order

- **framework**:
  - **phool**: Fun4All IO interface 'NodeTree' and IO manager 
  - **ffaobjects**: Sync multiple input streams.
  - **fun4all**: framework related classes
    - `Fun4AllBase`: verbosity control
    - `Fun4AllServer`: the interface of the Fun4All framework
    - `Fun4AllInputManage` 
    - `Fun4AllOutputManage`
    - `SubsystemReco`
- **interface_main**: main interface objects
- **module_example**: example modules
