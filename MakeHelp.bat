@echo off
REM -- First make map file from Microsoft Visual C++ generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by NetPerSec.HPJ. >"hlp\NetPerSec.hm"
echo. >>"hlp\NetPerSec.hm"
echo // Commands (ID_* and IDM_*) >>"hlp\NetPerSec.hm"
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>"hlp\NetPerSec.hm"
echo. >>"hlp\NetPerSec.hm"
echo // Prompts (IDP_*) >>"hlp\NetPerSec.hm"
makehm IDP_,HIDP_,0x30000 resource.h >>"hlp\NetPerSec.hm"
echo. >>"hlp\NetPerSec.hm"
echo // Resources (IDR_*) >>"hlp\NetPerSec.hm"
makehm IDR_,HIDR_,0x20000 resource.h >>"hlp\NetPerSec.hm"
echo. >>"hlp\NetPerSec.hm"
echo // Dialogs (IDD_*) >>"hlp\NetPerSec.hm"
makehm IDD_,HIDD_,0x20000 resource.h >>"hlp\NetPerSec.hm"
echo. >>"hlp\NetPerSec.hm"
echo // Frame Controls (IDW_*) >>"hlp\NetPerSec.hm"
makehm IDW_,HIDW_,0x50000 resource.h >>"hlp\NetPerSec.hm"
REM -- Make help for Project NetPerSec


echo Building Win32 Help files
start /wait hcw /C /E /M "hlp\NetPerSec.hpj"
if errorlevel 1 goto :Error
if not exist "hlp\NetPerSec.hlp" goto :Error
if not exist "hlp\NetPerSec.cnt" goto :Error
echo.
if exist Debug\nul copy "hlp\NetPerSec.hlp" Debug
if exist Debug\nul copy "hlp\NetPerSec.cnt" Debug
if exist Release\nul copy "hlp\NetPerSec.hlp" Release
if exist Release\nul copy "hlp\NetPerSec.cnt" Release
echo.
goto :done

:Error
echo hlp\NetPerSec.hpj(1) : error: Problem encountered creating help file

:done
echo.
