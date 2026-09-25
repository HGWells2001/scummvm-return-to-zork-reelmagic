@echo off
setlocal
cd /d "%~dp0"

powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0build.ps1"
set "RC=%ERRORLEVEL%"

echo.
if not "%RC%"=="0" (
  echo ============================================================
  echo BUILD FALLITA con codice %RC%.
  echo Controlla il log indicato sopra per i dettagli.
  echo ============================================================
) else (
  echo ============================================================
  echo BUILD COMPLETATA CON SUCCESSO.
  echo.
  echo Output:
  echo   %~dp0BUILD_OUTPUT\ScummVM-RTZ-ReelMagic
  echo.
  echo Premi un tasto per chiudere questa finestra.
  echo ============================================================
)

echo.
pause
exit /b %RC%
