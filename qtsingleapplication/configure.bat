@echo off

rem
rem "Main"
rem

if not "%1"=="" (
    if not "%1"=="-library" (
        call :PrintUsage
        goto EOF
    )
)

if exist config.pri. del config.pri
if "%1"=="-library" (
    echo Configuring to build this component as a dynamic library.
    echo SOLUTIONS_LIBRARY = yes > config.pri
)

echo .
echo This component is now configured.
echo .
echo To build the component library (if requested) and example(s),
echo run qmake and your make or nmake command.
echo .
echo To remove or reconfigure, run make (nmake) distclean.
echo .

:PrintUsage
echo Usage: configure.bat [-library]
echo .
echo -library: Build the component as a dynamic library (DLL). Default is to
echo           include the component source directly in the application.
echo           A DLL may be preferable for technical or licensing (LGPL) reasons.
echo .
goto EOF


:EOF
