set jdk=C:\EFW_ALL\jdk1.8.0_351\bin
rem set jdk=C:\EFW_ALL\jdk-20.0.2\bin
set LIB=C:\EFW_ALL\quickjs_start
set CLASSPATH=C:\EFW_ALL\quickjs_start;%LIB%\j2qjs.jar;
echo %CLASSPATH%
%jdk%\java Test
pause