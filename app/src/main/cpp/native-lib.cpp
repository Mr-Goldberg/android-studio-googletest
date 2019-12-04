#include <jni.h>
#include <string>
#include "native-lib.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_goldberg_googletest_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */)
{
    std::string hello = createWelcomingString();
    return env->NewStringUTF(hello.c_str());
}

std::string createWelcomingString()
{
    return "Hello from C++";
}
