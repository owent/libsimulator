/**
 * Licensed under the MIT licenses.
 *
 * @file Config.h
 * @brief 
 *
 *
 * @version 1.0
 * @author owent, admin@owent.net
 * @date 2013年9月23日
 *
 * @history
 *
 */

#ifndef _SIMU_UTILITY_APPCONFIG_H_
#define _SIMU_UTILITY_APPCONFIG_H_

#include <string>
#include <vector>
#include <stdint.h>
#include <cstddef>

namespace simulator
{
    struct AppConfig
    {
        static bool m_bIsContinue;

        static std::string m_strHistoryFileName;
        static std::string m_strProtoLogFile;

        static bool m_bIsNoInteractive;
        static std::vector<std::string> m_stOptCmds;
        static std::string m_strScriptFile;

        static int m_iPlayerThreadPoolMax;
    };
}


#endif /* CONFIG_H_ */
