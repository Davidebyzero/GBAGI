# Run on Debian: apt install mingw-w64
# TODO: 64-bit using x86_64-w64-mingw32-g++ 
i686-w64-mingw32-windres addgame.rc -o addgame.rc.o
i686-w64-mingw32-windres main.rc -o main.rc.o
i686-w64-mingw32-windres gbinjectb.rc -o gbinjectb.rc.o
i686-w64-mingw32-g++ gbinjectb.cpp addgame.cpp addgame.form.cpp commands.cpp decompress.cpp dirdialog.cpp main.cpp main.form.cpp makerom.cpp verdef.cpp vcl-shim/vcl-shim.cpp vcl-shim/form.cpp gbinjectb.rc.o addgame.rc.o main.rc.o -Ivcl-shim -lcomdlg32 -static-libgcc -static-libstdc++ -mwindows -o gbinjectb.exe
