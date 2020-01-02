set PRJ_NAME=core
set AXF_DIR=.\%PRJ_NAME%_Data\MCR_V2_32MB
set AXF_NAME=%PRJ_NAME%.axf
set RAM_DIR=%AXF_DIR%
set PKG_DIR=..\..\tools\DownloadTool_mcrv2_32MB
set PKG_NAME=ALL
set TOOL_DIR=..\..\..\..\core\tool

fromELF.exe -c -bin %AXF_DIR%\%AXF_NAME% -output %RAM_DIR%\RESET
%TOOL_DIR%\FirmwarePackager .\core_LDS_mcr_v2_32MB.txt %RAM_DIR%\ %PKG_DIR%\%PKG_NAME% /q
