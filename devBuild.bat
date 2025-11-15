@echo off
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
