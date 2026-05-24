@echo off
REM 待办列表应用构建脚本
REM 使用方法:
REM   build.bat              - Debug构建
REM   build.bat Release      - Release构建

setlocal enabledelayedexpansion

echo ========================================
echo 待办列表应用构建脚本
echo ========================================
echo.

REM 设置默认配置
set CONFIG=Debug
if not "%~1"=="" (
    set CONFIG=%~1
)

echo 构建配置: %CONFIG%
echo.

REM 检查CMake是否可用
where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo 错误: 未找到CMake，请先安装CMake
    exit /b 1
)

REM 创建构建目录
if not exist build (
    echo 创建构建目录...
    mkdir build
)

REM 进入构建目录
cd build

REM 配置（如果需要）
if not exist CMakeCache.txt (
    echo.
    echo 配置CMake...
    cmake -G "Visual Studio 17 2022" -A x64 ..
    if !ERRORLEVEL! NEQ 0 (
        echo 错误: CMake配置失败
        exit /b 1
    )
)

REM 构建
echo.
echo 构建中...
cmake --build . --config %CONFIG%

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo 错误: 构建失败
    exit /b 1
)

echo.
echo ========================================
echo 构建成功!
echo ========================================
echo.
echo 可执行文件:
if %CONFIG% == Debug (
    echo   bin\Debug\TodoApp.exe
) else (
    echo   bin\Release\TodoApp.exe
)

echo.
echo 测试可执行文件:
if %CONFIG% == Debug (
    echo   tests\Debug\todoapp_tests.exe
) else (
    echo   tests\Release\todoapp_tests.exe
)
echo.
echo 运行应用:
echo   start "" bin\%CONFIG%\TodoApp.exe
echo.

endlocal
