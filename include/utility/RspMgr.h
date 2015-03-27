#ifndef _SIMU_UTILITY_RSP_MGR_H_
#define _SIMU_UTILITY_RSP_MGR_H_

#include <map>
#include <string>
#include <fstream>
#include <stdint.h>
#include <std/functional.h>
#include <std/smart_ptr.h>
#include <ext/Singleton.h>

#include "common/StdoutMsg.h"
#include "utility/PlayerImpl.h"


namespace simulator
{
    template<typename TMsgID, typename TMsg, typename TPlayer>
    class RspMgr: public Singleton<RspMgr<TMsgID, TMsg, TPlayer> >
    {
    public:
        typedef TMsgID msg_id_t;
        typedef std::function<void (std::shared_ptr<TPlayer>, const TMsg&)> callback_func_type;
        typedef std::function<std::string (const TMsg&)> serialize_fn_t;
        typedef std::function<msg_id_t (std::shared_ptr<TPlayer>, const TMsg&)> pick_msgid_fn_t;

    public:
        RspMgr(){}
        ~RspMgr(){}

        int Init(serialize_fn_t fn_e, pick_msgid_fn_t fn_p)
        {
            m_pMsgSerializeFn = fn_e;
            m_pMsgPickMsgIDFn = fn_p;
            return 0;
        }

        void SetProcessFunc(msg_id_t uKey, const callback_func_type& stCaller)
        {
            m_stRspCmds[uKey] = stCaller;
        }

        int ProcessOnePkg(std::shared_ptr<TPlayer> pstPlayer, const TMsg& stPkg)
        {
            //打开proto日志文件
            if (!AppConfig::m_strProtoLogFile.empty() && m_pMsgSerializeFn)
            {
                std::ofstream stProtoLog;
                stProtoLog.open(AppConfig::m_strProtoLogFile.c_str(), std::ios::app | std::ios::out);
                if (stProtoLog.is_open())
                {
                    stProtoLog<< " ================ Response Pkg Begin ================ "<<
                        std::endl << m_pMsgSerializeFn(stPkg)<<
                        " ================ Response Pkg End ================ "<<
                        std::endl;
                }
            }

            // 控制台输出
            std::cout << ShellFont::GenerateString("response ",
                      SHELL_FONT_SPEC_BOLD | SHELL_FONT_COLOR_RED)
                      << std::endl
                      << m_pMsgSerializeFn(stPkg)
                      << std::endl;
            // 回调查找和调用
            typename std::map<msg_id_t, callback_func_type >::iterator iter = m_stRspCmds.find(m_pMsgPickMsgIDFn(stPkg));

            if (iter == m_stRspCmds.end())
            {
                return -1;
            }

            iter->second(pstPlayer, stPkg);
            return 0;
        }

    private:
        std::map<msg_id_t, callback_func_type > m_stRspCmds;

        serialize_fn_t m_pMsgSerializeFn;
        pick_msgid_fn_t m_pMsgPickMsgIDFn;
    };
}

#endif

