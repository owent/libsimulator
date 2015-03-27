#ifndef _SIMU_UTILITY_REQ_MGR_H_
#define _SIMU_UTILITY_REQ_MGR_H_

#include <string>
#include <set>
#include <fstream>
#include <stdint.h>

#include <std/functional.h>
#include "ext/Singleton.h"

#include "utility/PlayerImpl.h"

#include "CmdOption/CmdOption.h"

#include "common/StdoutMsg.h"

#define CMD_NAME_STR(str) ShellFont::GenerateString(str, SHELL_FONT_SPEC_BOLD | SHELL_FONT_COLOR_YELLOW)

namespace simulator
{
    template<typename TMsg, typename TPlayer>
    struct ReqInfo
    {
        bool bSendReq;
        TMsg* pSendMsg;
        std::shared_ptr<TPlayer> pPlayer;
    };


    class ReqMgrBase
    {
    public:
        static ReqMgrBase* Inst; /** 用于类型收敛，不建议外部直接使用 **/

    public:
        ReqMgrBase():m_pCmdManager(copt::CmdOptionCI::Create()) {}
        virtual ~ReqMgrBase(){}

        void SetCmdPlayer(const std::string& strID);

        /**
         * 统一生成指令帮助信息
         * @param strCmds 指令
         * @param strMessage 帮助消息
         * @param iCmdFilterLen 对其长度(默认为32)
         * @return 生成的Shell字符串
         */
        static const std::string GenCmdHelpMsg(const char* strCmds, const char* strMessage, const int& iCmdFilterLen = 32);

        virtual int ProcessCmd(std::shared_ptr<PlayerImpl> pPlayer, const std::string& szCmd) = 0;

    public:
        /**
         * 获取用户指令管理器
         * @note 该管理器的指令执行依赖激活的Uin
         * @return 用户指令管理器
         */
        inline std::shared_ptr<copt::CmdOptionCI> GetCmdManager() { return m_pCmdManager; }

    private:
        std::shared_ptr<copt::CmdOptionCI> m_pCmdManager;
    };

    template<typename TMsgID, typename TMsg, typename TPlayer>
    class ReqMgr : public ReqMgrBase, public Singleton< ReqMgr<TMsgID, TMsg, TPlayer> >
    {
    public:
        typedef std::function<int (std::shared_ptr<TPlayer>, TMsg&)> sender_fn_t;
        typedef std::function<std::string (const TMsg&)> serialize_fn_t;
        typedef std::function<void (TMsg&)> initmsg_fn_t;

    public:
        ReqMgr()
        {
            ReqMgrBase::Inst = this;
        }

        virtual ~ReqMgr(){}

        void Init(sender_fn_t fn_s, serialize_fn_t fn_e, initmsg_fn_t fn_i)
        {
            // 不支持的命令错误提示
            GetCmdManager()->BindCmd("@OnDefault, @OnError", &ReqMgr::OnCmdDefault, this);
            m_pSenderFn = fn_s;
            m_pSerializeFn = fn_e;
            m_pInitMsgFn = fn_i;
        }

        /**
         * 执行角色指令
         * @param [in] pPlayer 角色玩家
         * @param [in] szCmd 命令行
         * @return 错误码或0
         */
        virtual int ProcessCmd(std::shared_ptr<PlayerImpl> pPlayer, const std::string& szCmd)
        {
            ReqInfo<TMsg, TPlayer> stReqInfo;
            TMsg stCSMsg;
            stReqInfo.bSendReq = true;
            stReqInfo.pPlayer = pPlayer? std::dynamic_pointer_cast<TPlayer>(pPlayer): std::shared_ptr<TPlayer>();
            stReqInfo.pSendMsg = &stCSMsg;

            if (m_pInitMsgFn)
            {
                m_pInitMsgFn(stCSMsg);
            }

            GetCmdManager()->Start(szCmd, true, &stReqInfo);

            if (false == stReqInfo.bSendReq)
            {
                return 0;
            }

            // 如果执行完后仍没有角色数据则取消报文发送
            if (NULL == stReqInfo.pPlayer.get())
            {
                COUT_ERROR_MSG(szCmd)<< " cannot be exec without cmd player."<< std::endl;
                return 0;
            }

            return _send_req(stCSMsg, stReqInfo.pPlayer);
        }

    private:

        /* 发送请求 */
        int _send_req(TMsg& stPkg, std::shared_ptr<TPlayer> ptrPlayer)
        {
            //临时skey值
            int iRet = m_pSenderFn(ptrPlayer, stPkg);
            if (iRet < 0)
            {
                COUT_ERROR_MSG("Failed to send pkg.")<< " ret: "<< iRet<< " !"<< std::endl;
                return iRet;
            }

            // 如果 CPP 回调 被关闭，则不需要打印协议内容到控制台
            if (!ptrPlayer || !ptrPlayer->IsDispatcherEnable(EN_DT_CPP) || NULL == m_pSerializeFn)
            {
                return iRet;
            }

            // 输出协议文本到日志文件
            if (AppConfig::m_strProtoLogFile.size() > 0)
            {
                std::ofstream stProtoLog;
                stProtoLog.open(AppConfig::m_strProtoLogFile.c_str(), std::ios::app | std::ios::out);
                if (stProtoLog.is_open())
                {
                    stProtoLog<< " ================ Request Pkg Begin ================ "<< std::endl <<
                        m_pSerializeFn(stPkg)<< std::endl <<
                        " ================ Request Pkg End ================ "<< std::endl;
                }
            }

            // 输出协议文本到控制台
            std::cout << ShellFont::GenerateString("request ",
                      SHELL_FONT_SPEC_BOLD | SHELL_FONT_COLOR_RED)
                      << std::endl
                      << m_pSerializeFn(stPkg)
                      << std::endl;
            return 0;
        }

    private:
        sender_fn_t m_pSenderFn;
        serialize_fn_t m_pSerializeFn;
        initmsg_fn_t m_pInitMsgFn;

    public:
        void OnCmdDefault(copt::callback_param stParam)
        {
            if (stParam.GetParamsNumber() > 0)
            {
                printf("Unknown Command ");
                COUT_ERROR_MSG(stParam[0]->AsString())<< std::endl;

                // 禁止发送报文
                bool* pIsSendReq = (bool*)stParam.GetExtParam();
                (*pIsSendReq) = false;
            }
        }

    };
}
#endif

