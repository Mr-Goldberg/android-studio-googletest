#
# Tests
#
# Tests are executed only once during build. If no source files updated - no test will be executed.
#
# Tests are run on the targed device, and will fail, if device is not connected.
# If required to build project without installing, tests should be disabled.
#
# Tests are enabled by default.
# To disable tests add line "enable_cpp_tests=false" to the file "local.properties".
#

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
