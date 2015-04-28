/**
 * Licensed under the MIT licenses.
 *
 * @file ReqRspBase.h
 * @brief  协议管理基类
 *
 * 新增命令步骤:(参见user目录)
 *    1. 新增协议处理类Ty，继承ReqRspBase<Ty>
 *    2. 在构造函数内注册自动完成命令
 *    3. 在构造函数内绑定命令,实现回调函数
 *    4. 在构造函数内注册回包
 *    5. 在Active.cpp 中包含相应头文件，并激活刚刚实现的类
 *
 * @version 1.0
 * @author owent, admin@owent.net
 * @date 2012-10-9
 *
 * @history
 *
 */

#ifndef _SIMU_UTILITY_REQRSPBASE_H_
#define _SIMU_UTILITY_REQRSPBASE_H_

#include <iostream>
#include <sstream>
#include <algorithm>

#include <ext/Singleton.h>

#include "utility/CmdShell.h"

#include "utility/ReqMgr.h"
#include "utility/RspMgr.h"

#include "common/Active.h"


#define CHECK_REQ_PLAYER_VALID(player, param)   \
    if (NULL == player.get())                   \
    {                                           \
        DisableReqPkg("invalid player", param); \
        return;                                 \
    }

#define CHECK_REQ_PLAYER_LOGINED(player, param) \
    CHECK_REQ_PLAYER_VALID(player, param)       \
    if (false == player->IsLogined())             \
    {                                           \
        DisableReqPkg("player not logined", param); \
        return;                                 \
    }

#define CHECK_RSP_PLAYER_VALID(player)                  \
    if (NULL == player.get())                           \
    {                                                   \
        COUT_ERROR_MSG("invalid player")<< std::endl;   \
        return;                                         \
    }

#define CHECK_RSP_PLAYER_LOGINED(player)                                            \
    CHECK_RSP_PLAYER_VALID(player)                                                  \
    if (false == player->IsLogined())                                                 \
    {                                                                               \
        COUT_ERROR_MSG("player "<< player->GetID())<< " not logined")<< std::endl;\
        return;                                                                     \
    }

namespace simulator
{
    template<typename Ty, typename TSimu>
    class ReqRspBase: public Singleton<Ty>
    {
    public:
        typedef typename TSimu::player_t player_t;
        typedef typename TSimu::msg_id_t msg_id_t;
        typedef typename TSimu::msg_t msg_t;

    public:
        /**
         * 激活当前类
         */
        static void Active()
        {
            Singleton<Ty>::GetInstance();
        }

        static void OnCmdManagerError(copt::callback_param stParam)
        {
            COUT_ERROR_MSG("Cmd Error.")<< std::endl;
            const typename copt::CmdOptionList::cmd_array_type& stArray = stParam.GetCmdArray();

            DisableReqPkg(stParam);
            size_t uLen = stArray.size();
            std::stringstream ss;
            if (uLen < 2)
            {
                return;
            }

            // 第一个是根节点 @Root 最后一个是@OnError
            for (size_t i = 1; i < uLen - 2; ++ i)
            {
                ss<< CMD_NAME_STR(stArray[i].first)<< " ";
            }
            std::cout<< stArray[uLen - 2].second->GetHelpMsg(ss.str().c_str())<< std::endl;
        }

        /**
         * 获取根命令行管理器
         * @return 根命令行管理器智能指针
         */
        inline std::shared_ptr<copt::CmdOptionCI> GetCmdManager()
        {
            return ReqMgr<msg_id_t, msg_t , player_t>::Instance()->GetCmdManager();
        }

        /**
         * 获取子命令行管理器
         * @param strName 命令行管理器绑定的命令
         * @return 子命令行管理器智能指针
         */
        inline std::shared_ptr<copt::CmdOptionCI> GetCmdManager(const std::string& strName)
        {
            return GetCmdManager(strName, std::dynamic_pointer_cast<copt::CmdOptionCI>(simulator::ReqMgr<msg_id_t, msg_t , player_t>::Instance()->GetCmdManager()));
        }

        /**
         * 获取指定父命令行管理器的子管理器
         * @param strName 命令行管理器绑定的命令
         * @param pFather 父节点
         * @return 子命令行管理器智能指针
         */
        inline std::shared_ptr<copt::CmdOptionCI> GetCmdManager(const std::string& strName, std::shared_ptr<copt::CmdOptionCI> pFather)
        {
            std::shared_ptr<copt::CmdOptionCI> ret = std::dynamic_pointer_cast<copt::CmdOptionCI>(pFather->GetBindedCmd(strName.c_str()));
            if (ret == std::shared_ptr<copt::CmdOptionCI>())
            {
                ret = std::dynamic_pointer_cast<copt::CmdOptionCI>(
                    pFather->BindChildCmd(strName, copt::CmdOptionCI::Create())
                );

                ret->BindCmd("@OnError", ReqRspBase<Ty, TSimu>::OnCmdManagerError);
                ret->SetHelpMsg(CMD_NAME_STR(strName + " ").c_str());
            }
            return ret;
        }

        /**
         * 关闭请求报文处理后的发送动作
         */
        inline static void DisableReqPkg(copt::callback_param stParam)
        {
            ReqInfo<msg_t , player_t >& stReqInfo = GetReqInfo(stParam);
            stReqInfo.bSendReq = false;
        }

        /**
         * 关闭请求报文处理后的发送动作
         */
        void static DisableReqPkg(const std::string& strMsg, copt::callback_param stParam)
        {
            DisableReqPkg(stParam);
            COUT_ERROR_MSG(strMsg)<< std::endl;
            const typename copt::CmdOptionList::cmd_array_type& stArray = stParam.GetCmdArray();

            size_t uLen = stArray.size();
            std::stringstream ss;
            if (uLen <= 0)
                return;
            for (size_t i = 1; i < uLen - 1; ++ i)
            {
                ss<< CMD_NAME_STR(stArray[i].first)<< " ";
            }
            std::cout<< stArray.back().second->GetHelpMsg(ss.str().c_str())<< std::endl;
        }


        /**
         * 获取待发送的请求信息
         * @return 待发送的请求报文
         */
        inline static ReqInfo<msg_t , player_t >& GetReqInfo(copt::callback_param stParam)
        {
            ReqInfo<msg_t , player_t >* pReqInfo = (ReqInfo<msg_t , player_t >*)(stParam.GetExtParam());
            return *pReqInfo;
        }

        /**
         * 获取待发送的请求报文
         * @return 待发送的请求报文
         */
        inline static msg_t& GetReqPkg(copt::callback_param stParam)
        {
            ReqInfo<msg_t , player_t >& stReqInfo = GetReqInfo(stParam);
            return *stReqInfo.pSendMsg;
        }

        /**
         * 获取待发送请求的玩家
         * @return 待发送请求的玩家
         */
        inline static std::shared_ptr<player_t> GetReqPlayer(copt::callback_param stParam)
        {
            ReqInfo<msg_t , player_t >& stReqInfo = GetReqInfo(stParam);
            return stReqInfo.pPlayer;
        }

        /**
         * 获取自动完成框架，用于注册自动完成命令
         * @return 自动完成框架
         */
        inline AutoCompleteWrapper& RegisterAutoComplete()
        {
            return CmdShell::Instance()->GetAutoComplete();
        }

        /**
         * 设置回包的回调函数
         * @param id    回包ID
         * @param func  可调用对象，参数为Protocol::Pkg&
         */
        inline void SetRspProcessFunc(msg_id_t id, const typename RspMgr<msg_id_t, msg_t , player_t >::callback_func_type& func)
        {
            RspMgr<msg_id_t, msg_t , player_t >::Instance()->SetProcessFunc(id, func);
        }
    };
}

#endif /* REQRSPBASE_H_ */
