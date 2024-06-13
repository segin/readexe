# readexe

`readexe` is a command-line tool for inspecting various Microsoft PC EXE formats.

Travis CI status: [![Build Status](https://travis-ci.com/segin/readexe.svg?branch=master)](https://travis-ci.com/segin/readexe)

## File types supported

|Status|Magic|Type|
|-|-|-|
|✅|`MZ`|MS-DOS Executable|
|❕|`NE`|16-bit New Executable|
|❌|`LE`/`LX`|32-bit Linear Executable (.vxd/.386)|
|❌|`PE`|32/64-bit Portable Executable|

NE format is the current work-in-progress.

## Building

`readexe` uses the GNU Autotools (for now) to build. You'll need to generate the distributed build artifacts (the `configure` script itself, et. al.) before you can use them. Use the include `autogen.sh` script to generate the build artifacts. You can build `readexe` using the following: 

```
./autogen.sh
./configure
make
```

The previous `Makefile` has been renamed `Makefile.unix` and may be removed in the future, or may be rolled back to. 

If your OS does not have the BSD `err()` family functions, add `-I.` to `CFLAGS` in `Makefile.unix` or in the `CFLAGS` envionment variable when invoking `./configure`. 

If you are building for MS-DOS or FreeDOS using [GCC-IA16](https://github.com/tkchia/gcc-ia16) or [Sourcery CodeBench Lite for IA16](https://blogs.mentor.com/embedded/blog/2017/04/01/announcing-sourcery-codebench-lite-for-ia16/), use `Makefile.dos` like so: 

```
make -f Makefile.dos
```

Attempting to make DOS builds using other compilers is likely to fail. `readexe` is written using ISO C99 and ISO C11 features that the majority of DOS compilers predate. Modern DJGPP releases, also built upon GCC, can produce 32-bit extended DOS builds. 

Building for Windows CE is experimental. You will need to use the CeGCC toolchain, I recommend the Enlyze fork (as it is a commercial fork started in 2024 and as well-supported as one will get) - https://github.com/enlyze/cegcc-build

## Sample output

An output sample [can be found here](https://gist.github.com/segin/9130ff2e1a671c7a1aa71aabfb58e502).
