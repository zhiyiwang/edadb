

# Setup

Download the repository and run the following commands to build the project.
The repository @ https://github.com/zhiyiwang/orm

```bash
git clone git@github.com:zhiyiwang/orm.git
cd orm
git submodule update --init --recursive
mkdir build
cd build
cmake ..
make
```


# Source code

## third-party

### SOCI

Downloaded SOCI source code from [SOCI](https://github.com/SOCI/soci) repository.
We currently use the commit version `b7b470e` of SOCI.

```bash
git clone git@github.com:SOCI/soci.git
git checkout b7b470e66d81be1346224f68206738e0ff872e59
```

