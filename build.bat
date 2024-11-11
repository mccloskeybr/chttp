@echo off
cls

set DEBUG=0
set RELEASE=1
set TEST=2
set mode=%DEBUG%

FOR %%A IN (%*) DO (
  IF "%%A"=="/debug" (
    set mode=%DEBUG%
  ) ELSE IF "%%A"=="/release" (
    set mode=%RELEASE%
  ) ELSE IF "%%A"=="/test" (
    set mode=%TEST%
  ) ELSE IF "%%A"=="/+registrar" (
    set build_test_registrar="true"
  ) ELSE IF "%%A"=="/-run_test" (
    set execute_test="false"
  ) ELSE (
    echo WARNING: Unrecognized flag %%A!
  )
)

set compiler_warnings=/WX /W4 /WL /wd4201 /wd4100 /wd4189 /wd4505 /wd4127 /wd4115 /wd4456 /wd4457 /wd4244 /wd4245 /wd4701 /wd4310 /wd4702
set compiler_defs=/DWIN32 /D_CRT_SECURE_NO_WARNINGS

IF %mode%==%DEBUG% (
  echo ====================  BUILD DEBUG  ====================
  set compiler_defs=%compiler_defs% /Od /Zo /Z7 /DDEBUG /DHOTLOAD
) ELSE IF %mode%==%RELEASE% (
  echo ==================== BUILD RELEASE ====================
  set compiler_defs=%compiler_defs% /O2 /Zo- /Oi /DDEBUG
)

set compiler_flags=%compiler_defs% %compiler_warnings% /nologo /diagnostics:column /fp:fast /fp:except- /GR- /EHa- /FC /GS- /IZ:\chttp\src\
set linker_flags=/STACK:0x100000,0x100000 /incremental:no /opt:ref
set linker_libs=user32.lib kernel32.lib ws2_32.lib

pushd Z:\chttp
IF NOT EXIST .\build mkdir .\build
IF NOT EXIST .\bin mkdir .\bin
pushd .\src

cl %compiler_flags% server.c /Fo..\build\ /Fm..\build\ /Fd..\build\ /Fe..\bin\^
 /link %linker_flags% %linker_libs% /SUBSYSTEM:console

popd
popd
