/**
 * Licensed under the MIT licenses.
 *
 * @file StdoutMsg.h
 * @brief 
 *
 *
 * @version 1.0
 * @author owent, admin@owent.net
 * @date 2013年9月24日
 *
 * @history
 *
 */

#ifndef _SIMU_COMMON_STDOUTMSG_H_
#define _SIMU_COMMON_STDOUTMSG_H_

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <stdint.h>
#include <inttypes.h>

#include "ShellFont.h"

#ifndef COUT_MSG
    #define COUT_MSG(fmt) std::cout<< fmt
#endif
#ifndef CERR_MSG
    #define CERR_MSG(fmt) std::cerr<< fmt
#endif
#ifndef COUT_ERROR_MSG
    #define COUT_ERROR_MSG(fmt) CERR_MSG(ShellFont::GetStyleCode(SHELL_FONT_COLOR_RED)<< fmt<< ShellFont::GetStyleCloseCode())
#endif
#ifndef COUT_WARNING_MSG
    #define COUT_WARNING_MSG(fmt) COUT_MSG(ShellFont::GetStyleCode(SHELL_FONT_COLOR_YELLOW)<< fmt<< ShellFont::GetStyleCloseCode())
#endif
#ifndef COUT_NOTICE_MSG
    #define COUT_NOTICE_MSG(fmt) COUT_MSG(ShellFont::GetStyleCode(SHELL_FONT_COLOR_GREEN)<< fmt<< ShellFont::GetStyleCloseCode())
#endif

#ifndef PR_MSG
    #if defined(_MSC_VER)
        #define PR_MSG(...) printf(__VA_ARGS__)
    #elif defined(__GNUC__) || defined(__clang__)
        #define PR_MSG(fmt, arg...) printf(fmt, ##arg)
    #endif
#endif
#ifndef FPR_ERR_MSG
    #if defined(_MSC_VER)
        #define FPR_ERR_MSG(...) fprintf(stderr, __VA_ARGS__)
    #elif defined(__GNUC__) || defined(__clang__)
        #define FPR_ERR_MSG(fmt, arg...) fprintf(stderr, fmt, ##arg)
    #endif
#endif

#ifndef PR_ERROR_MSG
    #if defined(_MSC_VER)
        #define PR_ERROR_MSG(...) FPR_ERR_MSG(__VA_ARGS__)
    #elif defined(__GNUC__) || defined(__clang__)
        #define PR_ERROR_MSG(fmt, arg...) FPR_ERR_MSG("%s" fmt "%s\n", ShellFont::GetStyleCode(SHELL_FONT_COLOR_RED).c_str(), ##arg, ShellFont::GetStyleCloseCode().c_str())
    #endif
#endif
#ifndef PR_WARNING_MSG
    #if defined(_MSC_VER)
        #define PR_WARNING_MSG(...) PR_MSG(__VA_ARGS__)
    #elif defined(__GNUC__) || defined(__clang__)
        #define PR_WARNING_MSG(fmt, arg...) PR_MSG("%s" fmt "%s\n", ShellFont::GetStyleCode(SHELL_FONT_COLOR_YELLOW).c_str(), ##arg, ShellFont::GetStyleCloseCode().c_str())
    #endif
#endif
#ifndef PR_NOTICE_MSG
    #if defined(_MSC_VER)
        #define PR_NOTICE_MSG(...) PR_MSG(__VA_ARGS__)
    #elif defined(__GNUC__) || defined(__clang__)
        #define PR_NOTICE_MSG(fmt, arg...) PR_MSG("%s" fmt "%s\n", ShellFont::GetStyleCode(SHELL_FONT_COLOR_GREEN).c_str(), ##arg, ShellFont::GetStyleCloseCode().c_str())
    #endif
#endif

#endif /* STDOUTMSG_H_ */
