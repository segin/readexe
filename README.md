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

Just simply do:

```
make
```

If your OS does not have the BSD `err()` family functions, add `-I.` to `CFLAGS` in `Makefile`. 

If you are building for MS-DOS or FreeDOS using [GCC-IA16](https://github.com/tkchia/gcc-ia16) or [Sourcery CodeBench Lite for IA16](https://blogs.mentor.com/embedded/blog/2017/04/01/announcing-sourcery-codebench-lite-for-ia16/), use `Makefile.dos` like so: 

```
make -f Makefile.dos
```

## Sample output

An output sample [can be found here](http://sprunge.us/QIhmFA).
