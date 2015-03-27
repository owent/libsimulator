#include <errno.h>

#include <cstring>
#include <string>
#include <cstddef>
#include <cstdlib>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <iostream>

#include <setjmp.h>
#include <signal.h>
#include <unistd.h>

#include <std/foreach.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "utility/CmdShell.h"

namespace simulator
{

    sigjmp_buf ctrlc_buf;

    static void OnSignalCatched(int sig)
    {
        rl_initialize(); // 解除readline绑定

        switch(sig)
        {
        case SIGTTIN:
            puts("turn tty input into background.");
            signal(SIGTTIN,SIG_IGN);
            CmdShell::Instance()->SetEOF();
            break;

        case SIGTTOU:
            puts("turn tty output into background.");
            signal(SIGTTOU,SIG_IGN);
            CmdShell::Instance()->SetEOF();
            break;

        case SIGQUIT:
        case SIGTSTP:
        case SIGTERM:
            printf("catch sig: %d, bye\n", sig);
            CmdShell::Instance()->SetEOF();
            CmdShell::Instance()->Close();
            break;

        case SIGINT:
            puts(""); //清除缓冲区
            break;

        default:
            printf("catch sig: %d\n", sig);
            break;
        }

        rl_cleanup_after_signal();
        siglongjmp(ctrlc_buf, 1); // 重新读数据
    }

    AutoCompleteWrapper& AutoCompleteWrapper::operator [](const std::string& strName)
    {
        std::string strKey = strName;
        std::transform(strName.begin(), strName.end(), strKey.begin(), ::toupper);

        AutoCompleteWrapper& ret = stChildren[strKey];
        ret.strName = strName;

        return ret;
    }

    void CmdShell::_read_history()
    {
        using_history();
        read_history(m_strHistoryFileName.c_str());
    }

    void CmdShell::_write_history()
    {
        // 统一写出, 移除冗余history
        write_history(m_strHistoryFileName.c_str());
    }

    CmdShell::CmdShell(): m_strShellName("CmdShell"), m_strPrompt("~>"),
        m_bIsEOF(false), m_bIsClosed(false),
        m_strHistoryFileName(".shell_history"), m_iMaxHostoryLength(32767)
    {
    }

    CmdShell::~CmdShell()
    {
        _write_history();
    }

    void CmdShell::SetEOF()
    {
        m_bIsEOF = true;
    }

    bool CmdShell::IsEOF() const
    {
        return m_bIsEOF;
    }

    void CmdShell::Close()
    {
        m_bIsClosed = true;
    }

    bool CmdShell::IsClosed() const
    {
        return m_bIsClosed;
    }

    /**
     * 设置控制台前缀
     * @param strMsg 新前缀
     */
    void CmdShell::SetPrompt(const std::string& strMsg)
    {
        m_strPrompt = strMsg;
    }

    /**
     * 设置最大历史Size，返回原始值
     * @param iNum 最大历史记录Size
     * @return 更改前的值
     */
    int CmdShell::SetMaxHistoryNumber(int iNum)
    {
        using std::swap;
        swap(iNum, m_iMaxHostoryLength);

        stifle_history(m_iMaxHostoryLength);
        return iNum;
    }

    void CmdShell::Init(const std::string& strName, const std::string strHistoryFileName)
    {
        m_strShellName.reserve(1024);
        m_strShellName = strName;
        m_strHistoryFileName = strHistoryFileName;

        SetMaxHistoryNumber(m_iMaxHostoryLength);
        // ====================================================================
        rl_readline_name = m_strShellName.c_str();
        rl_catch_signals = 0; // readline 不捕获 SIGINT, SIGQUIT, SIGTERM, SIGALRM, SIGTSTP, SIGTTIN, and SIGTTOU

        rl_attempted_completion_function = CmdShell::CompleteCmd;

        _read_history();

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
    }

    const std::string CmdShell::ReadLine()
    {
        std::string ret;
        char* strRd = NULL;

        while ( sigsetjmp( ctrlc_buf, 1 ) != 0 );

        if(IsEOF())
        {
            return ret;
        }

        while ((strRd = readline(m_strPrompt.c_str())) == NULL || *strRd == '\0')
        {
            if (strRd == NULL)
            {
                SetEOF();
                return ret;
            }
            if (*strRd == '\0')
            {
                free(strRd);
            }
        }

        add_history(strRd);
        append_history(1, m_strHistoryFileName.c_str());

        ret = strRd;

        free(strRd);
        return ret;
    }

    AutoCompleteWrapper& CmdShell::GetAutoComplete()
    {
        return m_stAutoComplete;
    }

    char** CmdShell::CompleteCmd(const char* text, int start, int end)
    {
        // text 记录的是当前单词， rl_line_buffer 记录的是完整行

        // =========================
        CmdShell& stInstance = CmdShell::GetInstance();
        std::stringstream stStream;
        stStream.str(rl_line_buffer);
        AutoCompleteWrapper* pWrapper = &stInstance.m_stAutoComplete;
        std::string strCur, strPrefix, strFullCmd;
        while(stStream >> strCur)
        {
            strFullCmd = strCur;
            std::transform(strCur.begin(), strCur.end(), strCur.begin(), ::toupper);
            AutoCompleteWrapper::value_type::iterator iter = pWrapper->stChildren.find(strCur);
            if (iter == pWrapper->stChildren.end())
                break;
            pWrapper = &iter->second;
            strPrefix += strFullCmd + " ";
            strCur = "";
        }
        AutoCompleteWrapper::value_type::iterator stIter = pWrapper->stChildren.begin();

        // 查找不完整词
        if (strCur.size() > 0)
        {
            stIter = pWrapper->stChildren.lower_bound(strCur);
        }

        while (stIter != pWrapper->stChildren.end())
        {
            // 跳过所有占位符
            if (stIter->second.strName.substr(0, 4) == "@PH_")
            {
                ++ stIter;
                continue;
            }

            if (strCur == stIter->first.substr(0, strCur.size()))
                stInstance.m_stAutoCompleteCmds.push_back(stIter->second.strName);
            else
                break;

            ++ stIter;
        }

        // 如果无响应命令，且允许列举文件列表则响应列举目录占位符
        if ( stInstance.m_stAutoCompleteCmds.empty() &&
            pWrapper->stChildren.find("@PH_CUR_FILE_LIST") != pWrapper->stChildren.end())
        {
            return rl_completion_matches(text, rl_filename_completion_function);
        }

        return rl_completion_matches(text, CmdShell::CompleteCmdGenerator);
    }

    char* CmdShell::CompleteCmdGenerator(const char* text, int state)
    {
        // 将会被多次调用，直到返回NULL为止
        // 所有的生成的指令必须以malloc分配内存并返回，每次一条,readline内部会负责释放
        CmdShell& stInstance = CmdShell::GetInstance();

        // 所有候选命令列举完毕
        if (stInstance.m_stAutoCompleteCmds.empty())
            return NULL;

        std::list<std::string>::iterator iter = stInstance.m_stAutoCompleteCmds.begin();

        char* ret = reinterpret_cast<char*>(malloc((*iter).size() + 1));
        if (ret == NULL)
        {
            std::cerr<< __FILE__<< ":"<< __LINE__<< " => "<< "Malloc memory for readline cmds failed."<< std::endl;
            return ret;
        }

        strncpy(ret, (*iter).c_str(), (*iter).size() + 1);
        stInstance.m_stAutoCompleteCmds.pop_front();

        return ret;
    }
}
