#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "std/foreach.h"


#include "utility/PlayerImpl.h"
#include "utility/PlayerPool.h"

#include "utility/ReqMgr.h"


namespace simulator
{
    ReqMgrBase* ReqMgrBase::Inst = NULL;

    void ReqMgrBase::SetCmdPlayer(const std::string& strID)
    {
        std::shared_ptr<PlayerImpl> pPlayer = PlayerPool::FindPlayerByID(strID);
        if (NULL != pPlayer.get())
        {
            PlayerPool::SetCmdPlayer(pPlayer);
        }
        else
        {
            COUT_ERROR_MSG("SetCmdPlayer "<< strID<< " Not found.")<< std::endl;
        }
    }

    const std::string ReqMgrBase::GenCmdHelpMsg(const char* strCmds, const char* strMessage, const int& iCmdFilterLen)
    {
        std::stringstream stStream;
        stStream<< ShellFont::GetStyleCode(SHELL_FONT_SPEC_BOLD | SHELL_FONT_COLOR_YELLOW)<< std::setw(iCmdFilterLen)<< std::setiosflags(std::ios::left)<<
            strCmds<< ShellFont::GetStyleCloseCode()<< strMessage;
        return stStream.str();
    }
}
