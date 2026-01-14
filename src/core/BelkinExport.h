#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
    #ifdef BelkinCore_EXPORTS
        #define BELKINCORE_EXPORT __declspec(dllexport)
    #else
        #define BELKINCORE_EXPORT __declspec(dllimport)
    #endif
#else
    #if __GNUC__ >= 4
        #define BELKINCORE_EXPORT __attribute__ ((visibility ("default")))
    #else
        #define BELKINCORE_EXPORT
    #endif
#endif

