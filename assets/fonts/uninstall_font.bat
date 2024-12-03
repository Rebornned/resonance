@echo off
setlocal

REM Define o nome do arquivo da fonte e o nome do registro da fonte
set "FONT_NAME=%~1"
set "FONT_REG_NAME=%~n1"

REM Remove a fonte do diretório de fontes do Windows
del "%WINDIR%\Fonts\%FONT_NAME%"

REM Remove a fonte do registro do Windows
reg delete "HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts" /v "%FONT_REG_NAME% (TrueType)" /f

REM Notifica o sistema sobre a mudança de fonte
RUNDLL32.EXE user32.dll,UpdatePerUserSystemParameters

endlocal
