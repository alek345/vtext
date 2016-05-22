@echo off

set LIBS=-lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf
set SRC=main.c buffer.c utils.c ui.c manager.c
set OUTPUT=vtext

REM windres vtext.rc -O coff -o vtext.res

gcc -g %SRC% %LIBS% -o %OUTPUT% vtext.res