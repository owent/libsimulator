#include "utility/AppConfig.h"

namespace simulator
{
    bool AppConfig::m_bIsContinue = true;

    std::string AppConfig::m_strHistoryFileName = ".shell_history";
    std::string AppConfig::m_strProtoLogFile = "proto.log";

    bool AppConfig::m_bIsNoInteractive = false;
    std::vector<std::string> AppConfig::m_stOptCmds;
    std::string AppConfig::m_strScriptFile = "";

    int AppConfig::m_iPlayerThreadPoolMax = 1;
}
