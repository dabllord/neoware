@ECHO OFF

SET "NDK=C:/Users/neoware/neoware-vonyaesh/android-ndk-r26d"

SET BUILD_PATH=cmake_build

cmake -S. -B%BUILD_PATH% -G "Unix Makefiles" ^
-DCMAKE_EXPORT_COMPILE_COMMANDS=TRUE ^
-DCMAKE_BUILD_TYPE=Release ^
-DCMAKE_TOOLCHAIN_FILE=%NDK%/build/cmake/android.toolchain.cmake ^
-DCMAKE_C_COMPILER=%NDK%\toolchains\llvm\prebuilt\windows-x86_64\bin\clang.exe ^
-DCMAKE_CXX_COMPILER=%NDK%\toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe ^
-DANDROID_NDK=%NDK% ^
-DANDROID_ABI=arm64-v8a ^
-DANDROID_NATIVE_API_LEVEL=21

EXIT