/**
 * Licensed under the MIT licenses.
 *
 * @file ShellCmd.cpp
 * @brief 
 *
 *
 * @version 1.0
 * @author owent, admin@owent.net
 * @date 2013年9月27日
 *
 * @history
 *
 */

#include "utility/ReqMgr.h"

#include "utility/CmdShell.h"
#include "utility/ShellCmd.h"

namespace simulator
{

    static void OnSysCmdExit(copt::callback_param stParams, bool& bIsSysCmd)
    {
        // 不执行指令推送
        bIsSysCmd = true;

        AppConfig::m_bIsContinue = false;
    }

    static void OnSysExecShell(copt::callback_param stParam, bool& bIsSysCmd)
    {
        // 不执行指令推送
        bIsSysCmd = true;

        std::string strCmd;
        size_t uParamNum = stParam.GetParamsNumber();
        for (size_t i = 0 ; i < uParamNum; ++ i) {
            if (strCmd.size() > 0)
                strCmd += " ";
            strCmd += stParam[i]->AsCppString();
        }

        int res = system(strCmd.c_str());

        if (res != 0)
        {
            printf("$? = %d\n", res);
        }

    }

    static void OnSysCmdHelp(copt::callback_param stParams, bool& bIsSysCmd, copt::CmdOptionCI& handler)
    {
        // 不执行指令推送
        bIsSysCmd = true;

        // 角色指令
        puts("Usage:");
        puts("Player Command:");
        puts(ReqMgrBase::Inst->GetCmdManager()->GetHelpMsg().c_str());

        // 系统指令
        puts("");
        puts("System Command:");
        puts(handler.GetHelpMsg().c_str());
    }

    static void OnSysCmdSetCmdPlayer(copt::callback_param stParam, bool& bIsSysCmd)
    {
        // 不执行指令推送
        bIsSysCmd = true;

        if (stParam.GetParamsNumber() < 1)
        {
            COUT_ERROR_MSG("SetCmdPlayer require at least 1 parameter.")<< std::endl;
            return;
        }

        ReqMgrBase::Inst->SetCmdPlayer(stParam[0]->AsCppString());
    }

    static void BindingSysCmdAutoComplete()
    {
        // 注册自动完成
        AutoCompleteWrapper& stAuto = CmdShell::Instance()->GetAutoComplete();


        stAuto["!"]["@PH_CUR_FILE_LIST"];
        stAuto["sh"]["@PH_CUR_FILE_LIST"];
        stAuto["exit"];
        stAuto["quit"];
        stAuto["?"];
        stAuto["help"];
        stAuto["SetCmdPlayer"];
    }

    copt::CmdOptionCI::ptr_type BindingSysCmd(bool& isSysCmd)
    {
        static copt::CmdOptionCI::ptr_type handler = copt::CmdOptionCI::Create();

        // 帮助命令
        handler->BindCmd("?, help", OnSysCmdHelp, std::ref(isSysCmd), std::ref(*handler))
            ->SetHelpMsg(ReqMgrBase::GenCmdHelpMsg("?, help", "help.").c_str());

        // 退出指令
        handler->BindCmd("exit, quit", OnSysCmdExit, std::ref(isSysCmd))
            ->SetHelpMsg(ReqMgrBase::GenCmdHelpMsg("exit, quit", "exit.").c_str());

        // 运行系统命令
        handler->BindCmd("!, sh", OnSysExecShell, std::ref(isSysCmd))
            ->SetHelpMsg(ReqMgrBase::GenCmdHelpMsg("!, sh", "<...> exec system cmd.").c_str());

        // 设置当前命令行的角色
        handler->BindCmd("SetCmdPlayer", OnSysCmdSetCmdPlayer, std::ref(isSysCmd))
            ->SetHelpMsg(ReqMgrBase::GenCmdHelpMsg("SetCmdPlayer", "<openid> set cmd player.").c_str());

        BindingSysCmdAutoComplete();
        return handler;
    }

}
