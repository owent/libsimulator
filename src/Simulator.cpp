#include <string>
#include <unistd.h>
#include <vector>
#include <list>
#include <cstdlib>
#include <map>
#include <cctype>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <setjmp.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include "std/foreach.h"

#include "utility/AppConfig.h"

#include "CmdOption/CmdOption.h"
#include "utility/CmdShell.h"

#include "utility/RspMgr.h"
#include "utility/ReqMgr.h"
#include "utility/PlayerPool.h"
#include "utility/thread.h"

#include "utility/ShellCmd.h"

#include "Simulator.h"

// 激活协议接口
extern void active_protos();


namespace simulator
{
    // 无输入时信号捕获接口
    extern sigjmp_buf ctrlc_buf;

    namespace detail
    {
        static void OnSignalCatched(int sig)
        {
            switch(sig)
            {
            case SIGTTIN:
                puts("turn tty input into background.");
                signal(SIGTTIN,SIG_IGN);
                break;

            case SIGTTOU:
                puts("turn tty output into background.");
                signal(SIGTTOU,SIG_IGN);
                break;

            case SIGQUIT:
            case SIGTSTP:
            case SIGTERM:
                printf("catch signal: %d, bye\n", sig);
                AppConfig::m_bIsContinue = false;
                break;

            default:
                printf("catch signal: %d\n", sig);
                break;
            }

            siglongjmp(ctrlc_buf, 1);
        }

        // ===========================================================
        // =================        绑定的函数        ================
        // ===========================================================

        // 绑定的输出函数
        static void help_func(copt::callback_param stParams, bool& bIsContinue, copt::CmdOptionCI& stCmdOpt)
        {
            std::cout<< "Usage:"<< std::endl<< stCmdOpt.GetHelpMsg()<< std::endl;
            // 立即退出
            bIsContinue = false;
        }

        // 生成帮助信息
        const std::string GenCmdHelpMsg(const char* strCmds, const char* strMessage, const int& iCmdFilterLen)
        {
            std::stringstream stStream;
            stStream<< "\033[1;33m"<< std::setw(iCmdFilterLen)<< std::setiosflags(std::ios::left)<<
                strCmds<< "\033[0m\t"<< strMessage;
            return stStream.str();
        }

        static std::list<init_fn_t>& GetInitFnList()
        {
            static std::list<init_fn_t> ret;
            return ret;
        }

        int AddInitFn(init_fn_t fn)
        {
            GetInitFnList().push_back(fn);
            return 0;
        }

        void ReadOptionCmds(copt::CmdOptionCI::ptr_type pSysCmds, bool& bIsSysCmd)
        {
            // 文件命令
            owent_foreach(const std::string& strCmd, AppConfig::m_stOptCmds)
            {
                if (!AppConfig::m_bIsContinue) break;

                // 如果是系统指令则不把命令推送给角色
                bIsSysCmd = false;
                pSysCmds->Start(strCmd, true);
                if(bIsSysCmd) continue;

                PlayerPool::InsertPlayerCmd(PlayerPool::GetCmdPlayer(), strCmd);
            }
        }

        void ReadFileCmds(copt::CmdOptionCI::ptr_type pSysCmds, bool& bIsSysCmd)
        {
            if (AppConfig::m_strScriptFile.size() == 0)
            {
                return;
            }

            std::fstream stFile;
            stFile.open(AppConfig::m_strScriptFile.c_str(), std::ios_base::in);
            if (false == stFile.is_open())
            {
                printf("open cmd source file %s failed.\n", AppConfig::m_strScriptFile.c_str());
                return;
            }

            // 文件命令
            while(false == stFile.eof())
            {
                if (!AppConfig::m_bIsContinue) break;
                std::string strCmd;
                std::getline(stFile, strCmd);

                // trim string
                strCmd = strCmd.erase(0, strCmd.find_first_not_of(" \t\r\n")).erase(strCmd.find_last_not_of(" \t\r\n") + 1);
                if (strCmd.size() == 0) continue;

                // 如果是系统指令则不把命令推送给角色
                bIsSysCmd = false;
                pSysCmds->Start(strCmd, true);
                if(bIsSysCmd) continue;

                PlayerPool::InsertPlayerCmd(PlayerPool::GetCmdPlayer(), strCmd);
            }
        }

        void ReadConsoleCmds(copt::CmdOptionCI::ptr_type pSysCmds, bool& bIsSysCmd)
        {
            // 创建命令行工具
            CmdShell& shell = CmdShell::GetInstance();

            // 跳过交互式命令?
            if (AppConfig::m_bIsNoInteractive)
            {
                // 系统信号处理
                if(signal(SIGINT, OnSignalCatched) == SIG_ERR)
                    printf("failed to register cancel interrupts with kernel\n");

                if(signal(SIGQUIT, OnSignalCatched) == SIG_ERR)
                    printf("failed to register quit interrupts with kernel\n");

                if(signal(SIGTERM, OnSignalCatched) == SIG_ERR)
                    printf("failed to register quit interrupts with kernel\n");

                if(signal(SIGALRM, OnSignalCatched) == SIG_ERR)
                    printf("failed to register quit interrupts with kernel\n");

                if(signal(SIGTSTP, OnSignalCatched) == SIG_ERR)
                    printf("failed to register tty stop interrupts with kernel\n");

                if(signal(SIGTTIN, OnSignalCatched) == SIG_ERR)
                    printf("failed to register tty tin interrupts with kernel\n");

                if(signal(SIGTTOU, OnSignalCatched) == SIG_ERR)
                    printf("failed to register tty tou interrupts with kernel\n");

                return;
            }

            // 初始化命令行工具
            shell.Init("Pluto.Simulator", AppConfig::m_strHistoryFileName);
            shell.SetPrompt("~>");  // 设置输出前缀

            // 用户命令
            while(AppConfig::m_bIsContinue)
            {
                errno = 0;
                const std::string strCmd = shell.ReadLine();

                if (shell.IsEOF())
                {
                    if (shell.IsClosed())
                    {
                        AppConfig::m_bIsContinue = false;
                    }
                    break;
                }

                // 如果是系统指令则不把命令推送给角色
                bIsSysCmd = false;
                pSysCmds->Start(strCmd, true);
                if(bIsSysCmd) continue;

                PlayerPool::InsertPlayerCmd(PlayerPool::GetCmdPlayer(), strCmd);
            }
        }

        copt::CmdOptionCI::ptr_type BindingProgramParams()
        {
            static copt::CmdOptionCI::ptr_type handler;
            if(handler)
            {
                return  handler;
            }

            handler = copt::CmdOptionCI::Create();

            // 绑定帮助信息
            handler->BindCmd("?, -h, --help, help", help_func, std::ref(AppConfig::m_bIsContinue), std::ref(*handler))
                ->SetHelpMsg(GenCmdHelpMsg("?, -h, --help, help", "Print help information.").c_str());

            // 绑定设置历史操作文件路径
            handler->BindCmd("-his, --history, --history-file", phoenix::AssignAction<std::string>(AppConfig::m_strHistoryFileName))
                ->SetHelpMsg(GenCmdHelpMsg("-his, --history, --history-file", "<file path> Set Cmd Log File(default: .shell_history)").c_str());

            // 绑定设置协议反序列化日志文件路径
            handler->BindCmd("-pro, --proto, --proto-log", phoenix::AssignAction<std::string>(AppConfig::m_strProtoLogFile))
                ->SetHelpMsg(GenCmdHelpMsg("-pro, --proto, --proto-log", "<file path> Set Proto Log File(default: proto.log)").c_str());

            // 绑定设置玩家线程池最大线程数量
            handler->BindCmd("-ptm, --player-threadpool-max", phoenix::AssignAction<int>(AppConfig::m_iPlayerThreadPoolMax))
                ->SetHelpMsg(GenCmdHelpMsg("-ptm, --player-threadpool-max", "<max thread number> Set max thread number of player thread pool.(default: 1)").c_str());

            // 绑定脚本模式
            handler->BindCmd("-ni, --no-interactive", phoenix::SetAction<bool>(AppConfig::m_bIsNoInteractive, true))
                ->SetHelpMsg(GenCmdHelpMsg("-ni, --no-interactive", "disable interactive cmd source.").c_str());

            handler->BindCmd("-f, --rf, --read-file", phoenix::AssignAction<std::string>(AppConfig::m_strScriptFile))
                ->SetHelpMsg(GenCmdHelpMsg("-f, --rf, --read-file", "<file path> enable file cmd source").c_str());

            handler->BindCmd("-c, --cmd", phoenix::PushBackStrAction<std::vector<std::string> >(AppConfig::m_stOptCmds))
                ->SetHelpMsg(GenCmdHelpMsg("-c, --cmd", "[cmd ...] add cmd to run").c_str());

            return handler;
        }

        int LocalInit(int argc, char *argv[])
        {
            // 创建忽视大小写的命令绑定器
            copt::CmdOptionCI::ptr_type pPragramParams = BindingProgramParams();

            // 输出帮助信息
            pPragramParams->Start(argc, (const char**)argv, false);

            // 如果不需要后续功能则直接退出
            if (false == AppConfig::m_bIsContinue)
            {
                return 0;
            }

            std::list<init_fn_t>& stInitFns = GetInitFnList();
            owent_foreach(init_fn_t& fn, stInitFns)
            {
                fn();
            }

            return 0;
        }

        int LocalActive()
        {
            active_protos();
            return 0;
        }

        int LocalLoop()
        {
            // 如果不需要后续功能则直接退出
            if (false == AppConfig::m_bIsContinue)
            {
                return 0;
            }


            // 创建系统指令绑定器
            bool bIsSysCmd = false;
            copt::CmdOptionCI::ptr_type pSysCmds = BindingSysCmd(bIsSysCmd);

            // 处理参数预定命令
            ReadOptionCmds(pSysCmds, bIsSysCmd);

            // 处理文件读入命令
            ReadFileCmds(pSysCmds, bIsSysCmd);

            // 处理终端输入命令
            ReadConsoleCmds(pSysCmds, bIsSysCmd);

            // 等待所有用户进程退出
            while ( sigsetjmp( ctrlc_buf, 1 ) != 0 );
            PlayerPool::JoinAllPlayerThread();

            return 0;
        }
    }
}







