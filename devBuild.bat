@echo off
setlocal

rem ===== Intentar configurar VS 2022 x64 Native Tools =====
set VS2022_PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat

if exist "%VS2022_PATH%" (
    echo ===== Configurando entorno VS 2022 x64 =====
    call "%VS2022_PATH%" x64
) else (
    echo [WARNING] No se encontro VS 2022, se usara cmd normal.
)

echo.
echo ===== Iniciando build =====
cmake --preset=default
cmake --build build
if errorlevel 1 (
    echo [ERROR] Fallo al compilar el proyecto.
) else (
    echo [OK] Compilacion exitosa.
)

echo.
echo ===== Ejecutando programa =====
.\build\DuneEngine.exe
if errorlevel 1 (
    echo [ERROR] El programa termino con errores.
) else (
    echo [OK] El programa termino correctamente.
)

echo.
echo ===== Proceso finalizado =====
pause
endlocal
