@echo off

:: How to install emscripten
:: 
::cd /d C:\dev\_opensource
::git clone https://github.com/emscripten-core/emsdk.git
::cd emsdk
::emsdk install latest
::emsdk activate latest
::
:: Open NEW cmd and then:
::cd /d C:\dev\_opensource\emsdk
::emsdk_env.bat
:: 

:: Docs: https://emscripten.org/docs/api_reference/emscripten.h.html
:: SDL2 example: https://main.lv/writeup/web_assembly_sdl_example.md
:: 


call build_emscripten_SDL2_gfx.bat

set em_root=C:\dev\_opensource\emsdk

cd /d %em_root%
call emsdk_env.bat
cd /d %~dp0

echo Building ...

call emcc tic_tac_toe.cpp ^
	-O2 ^
	-s USE_SDL=2 ^
	-ID:\Downloads\SDL2_gfx-1.0.4\bin\include ^
	-LD:\Downloads\SDL2_gfx-1.0.4\bin\lib -lSDL2_gfx ^
	-std=c++14 ^
	-o tic_tac_toe_win.js

echo Running...
::emrun --no_browser --port 8080 .
::tic_tac_toe_win.html
emrun tic_tac_toe_win.html
