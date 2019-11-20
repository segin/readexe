# readexe

`readexe` is a command-line tool for inspecting various Microsoft PC EXE formats.

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

If you are building for MS-DOS or FreeDOS using GCC-IA16, use `Makefile.dos` like so: 

```
make -f Makefile.dos
```