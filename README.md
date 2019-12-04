# Google Test Integration With Android Studio

This project demonstrates the way to integrate the Google Test C++ unit testing library with Android Studio.

The tests are integrated through `CMake (in CMakeLists.txt)` and run as a post-build event.

This sample project is inspired by: https://stackoverflow.com/questions/46429958/android-ndk-with-google-test

How to integrate Google Test with XCode for iOS you can check here: https://github.com/mattstevens/xcode-googletest

# Run and output

Tests are post-native-build. They can be executed by pressing `Play` button, or executing gradle build by wide variety of ways, no matter in Android Studio or not. If tests are failed, next build steps (run application, etc.) will not be executed.

After execution of tests check the `Build / Build Output` tab of Android Studio to see the test **output**. 

Here is an example:
```
/mnt/disk2/dev/Pets/android-studio-googletest/app/.cxx/cmake/debug/arm64-v8a/native-tests-lib: 1 file pushed. 26.6 MB/s (6925720 bytes in 0.248s)
/mnt/disk2/dev/Pets/android-studio-googletest/app/build/intermediates/cmake/debug/obj/arm64-v8a/libnative-lib.so: 1 file pushed. 24.2 MB/s (1327032 bytes in 0.052s)
Running main() from /home/alex/Android/Sdk/ndk-bundle/sources/third_party/googletest/src/gtest_main.cc
[==========] Running 2 tests from 1 test case.
[----------] Global test environment set-up.
[----------] 2 tests from CreateWelcomingString
[ RUN      ] CreateWelcomingString.NotEmpty
[       OK ] CreateWelcomingString.NotEmpty (0 ms)
[ RUN      ] CreateWelcomingString.Validity
[       OK ] CreateWelcomingString.Validity (0 ms)
[----------] 2 tests from CreateWelcomingString (0 ms total)
[----------] Global test environment tear-down
[==========] 2 tests from 1 test case ran. (1 ms total)
[  PASSED  ] 2 tests.
```
# How to set-up Google Tests?
### Write Google Tests
`app/src/test/cpp/native-libTests.cpp`
```
#include <gtest/gtest.h>
#include <native-lib.h>

using namespace std;

TEST(CreateWelcomingString, NotEmpty)
{
    EXPECT_NE(createWelcomingString().length(), 0);
}

TEST(CreateWelcomingString, Validity)
{
    const string welcomingString = createWelcomingString();
    EXPECT_EQ(welcomingString, "Hello from C++");
    EXPECT_NE(welcomingString, "Good bye from C++");
}

```

### Create CMakeLists.txt with rules to build and run tests

`app/src/test/cpp/CMakeLists.txt`

Variables passed to the script:

`TEST_SRC_DIR` Source dir for tests. Passed from the main script (in which this one is included).

`ENABLE_CPP_TESTS` determines if tests will build & run.

`ANDROID_SDK_ROOT` required to find path to `adb` tool.

```
if (${ENABLE_CPP_TESTS} MATCHES "true")

    # Build and link tests

    # Path to Google Test source dir included into Android NDK.
    # Note: Another Google Test implementation can be used.
    set(GTEST_DIR ${ANDROID_NDK}/sources/third_party/googletest)

    add_library(gtest STATIC ${GTEST_DIR}/src/gtest_main.cc ${GTEST_DIR}/src/gtest-all.cc)
    target_include_directories(gtest PRIVATE ${GTEST_DIR})
    target_include_directories(gtest PUBLIC ${GTEST_DIR}/include)

    add_executable(native-tests-lib
            ${TEST_SRC_DIR}/native-libTests.cpp)

    target_link_libraries(native-tests-lib native-lib gtest)

    # Push and execute tests as post-build event.

    set(TARGET_TEST_DIR /data/local/tmp/native-tests-lib) # Directory on device to push tests.
    set(TARGET_TEST_LIB_DIR ${TARGET_TEST_DIR}/${ANDROID_ABI})
    message("ANDROID_SDK_ROOT: ${ANDROID_SDK_ROOT}")

    find_program(ADB NAMES adb PATHS ${ANDROID_SDK_ROOT}/platform-tools) # Verified to be working on Linux.

    add_custom_command(TARGET native-tests-lib POST_BUILD
            COMMAND ${ADB} shell mkdir -p ${TARGET_TEST_LIB_DIR}

            # Push libraries

            COMMAND ${ADB} push $<TARGET_FILE:native-tests-lib> ${TARGET_TEST_LIB_DIR}/
            COMMAND ${ADB} push $<TARGET_FILE:native-lib> ${TARGET_TEST_LIB_DIR}/

            # Execute tests

            # FIXME What's correct: Build is stopped, if tests failed.
            # What's to fix: Next build, if no files changed, will not execute and tests will not run, i.e. untested apk will be installed.
            # Note: Sometimes desired logic is working. Sometimes not.
            COMMAND ${ADB} shell \"export LD_LIBRARY_PATH=${TARGET_TEST_LIB_DIR}\; ${TARGET_TEST_LIB_DIR}/native-tests-lib\")

endif () # ENABLE_CPP_TESTS
```

### Edit Main CMakeLists.txt to include the Test script

`app/src/main/cpp/CMakeLists.txt`

Include main sources directory, so `#include` directive will work correctly in the test files. This is relative to this file location.

`include_directories(.)`

Include the test script at the end of file, after main library is linked.
```
set(TEST_SRC_DIR ../../test/cpp) # Variable will be used in test script.
include(../../test/cpp/CMakeLists.txt)
```

### Edit app-level build.gradle

`app/build.gradle`

Pass the required variables into CMake script.

I prefer to do it in this script instead of creating encironment variable(s).
```
android {
    ...
    defaultConfig {
        ...
        externalNativeBuild {
            ...
            cmake {
                ...

                // Tests are enabled by default.
                // To disable tests add line "enable_cpp_tests=false" into the file "local.properties".
                arguments "-DENABLE_CPP_TESTS=" + get_enable_cpp_tests().toString()

                // Use location of Android Sdk from "local.properties".
                // This is required to find path to "adb" tool.
                arguments "-DANDROID_SDK_ROOT=" + get_android_sdk_root().toString()
            }
        }
    }
```
At the end of file, on top level add the helper functions to read variables.
```
def get_enable_cpp_tests() {
    return get_properties().getProperty('enable_cpp_tests', 'true').toBoolean()
}

def get_android_sdk_root() {
    return get_properties().getProperty('sdk.dir', '').toString()
}

def get_properties() {
    Properties properties = new Properties()
    properties.load(new File(rootDir.absolutePath + '/local.properties').newDataInputStream())
    return properties
}
```
### Now run!
