set PRJ_NAME=bootloader
set AXF_DIR=.\%PRJ_NAME%_Data\MCR_V2_Q_128MB
set AXF_NAME=%PRJ_NAME%.axf
set RAM_DIR=%AXF_DIR%
set PKG_DIR=..\..\tools\DownloadTool_mcrv2_q_128MB
set PKG_NAME=AITBOOT
set TOOL_DIR=..\..\..\..\core\tool

fromELF.exe -c -bin %AXF_DIR%\%AXF_NAME% -output %RAM_DIR%\ALL_DRAM
%TOOL_DIR%\FirmwarePackager .\bootloader_LDS_mcr_v2.txt %RAM_DIR%\ %PKG_DIR%\%PKG_NAME% /q