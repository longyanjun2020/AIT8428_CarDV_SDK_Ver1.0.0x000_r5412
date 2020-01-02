@perl -v > NUL
@@IF ERRORLEVEL 1  GOTO L_no_perl

@ECHO off

SET SRC_DIR=..\..\..\..\core\src\dvr\net
SET PRJ_CONF=amn_project.ucos.conf
SET TMP_CONF=combined.conf
SET OUTPUT_SYSCFG=auto_syscfg.h

@REM @ECHO "Check the %PRJ_CONF% exists"
IF EXIST %PRJ_CONF% (
    rem copy %SRC_DIR%\common\conf\amn_system.ucos.conf + %PRJ_CONF% %TMP_CONF%
) else (
    echo.
	echo %PRJ_CONF% Does not exists. Please be sure that customized conf file is not needed.
    rem copy %SRC_DIR%\common\conf\amn_system.ucos.conf %TMP_CONF%
)

@ECHO.
copy %SRC_DIR%\common\conf\amn_system.ucos.conf + %PRJ_CONF% %TMP_CONF%

rem copy /A %SRC_DIR%\common\conf\amn_system.ucos.conf + /A %PRJ_CONF% %TMP_CONF%
REM using /A option appends 0x1A which might cause error for %OUTPUT_SYSCFG%
perl %SRC_DIR%\bin\cfg-compile.pl -o %OUTPUT_SYSCFG% %TMP_CONF%
del %TMP_CONF%


@REM ECHO "Check the %OUTPUT_SYSCFG% is generated automatically"
@ECHO.
IF EXIST %OUTPUT_SYSCFG% (
    echo %OUTPUT_SYSCFG% is generated.
) else (
	echo ERROR on generating %OUTPUT_SYSCFG%
)
@ECHO.

@REM Generats CHP. Now it's obsoleted.
@REM @mkdir http\chp_C
@REM @perl bin\chp-compile.pl -o http/chp_C/ http/chp/
@REM @ECHO "Check the CHP files are generated automatically"
@REM @DIR http\chp_C

@GOTO End


:L_no_perl
@ECHO "No PERL executable exists in your PATH, please install it first"
@ECHO "You may download from here: http://www.activestate.com/activeperl/downloads"

GOTO End


:End
@PAUSE
