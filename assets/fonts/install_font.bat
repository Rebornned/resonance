@echo off
setlocal

REM Define o caminho da fonte e o nome do arquivo da fonte
set "FONT_PATH=%~1"
set "FONT_NAME=%~2"
set "FONT_REG_NAME=%~n2"

REM Copia a fonte para o diretório de fontes do Windows
copy "%FONT_PATH%" "%WINDIR%\Fonts\%FONT_NAME%"

REM Adiciona a fonte ao registro do Windows
reg add "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts" /v "%FONT_REG_NAME% (TrueType)" /t REG_SZ /d "%FONT_NAME%" /f

REM Notifica o sistema sobre a mudança de fonte
RUNDLL32.EXE user32.dll,UpdatePerUserSystemParameters

endlocal
