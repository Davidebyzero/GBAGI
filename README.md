# GBAGI

Original author: [Brian Provinciano](http://www.bripro.com/)

More resources: http://www.agiwiki.sierrahelp.com/index.php/GBAGI

## Building the source code

### Windows

To build the `gbagi.bin` ROM image:

- Option 1 - DevKitPro R4: refer to the Github Action
- Option 2 - MSYS2 + gcc-arm: refer to Linux instructions

To build the `romgui` ROM injector GUI application:

- Option 1 - MSVC: refer to `romgui/romgui.sln`
- Option 2 - MSYS2 + gcc-mingw: refer to Linux instructions

### Linux/Debian

To build the `gbagi.bin` ROM image:

```
apt-get install gcc-arm-none-eabi
bash make.sh
```

Note that the GBA linkscripts and C Runtime are sensitive to GCC/LD versions. 
The files in the repo are tested with gcc-12 on Debian Bookworm.

To build the `romgui` ROM injector GUI application:

```
apt-get install mingw-w64
cd romgui
bash make.sh
```
