/**
 * Licensed under the MIT licenses.
 *
 * @file Simulator.h
 * @brief 
 *
 *
 * @version 1.0
 * @author owent, admin@owent.net
 * @date 2014年6月26日
 *
 * @history
 *
 * @note
 *   <h4>简介</h4>
 *   libsimulator 是一个用于在服务器环境中模拟客户端行为的易于拓展和集成的快速自测、压测、单元测试框架。
 *   <h4>使用方法</h4>
 *   <ol>
 *   <li>[必须] 定义Player类型</li>
 *   <li>[必须] 定义协议ID类型</li>
 *   <li>[必须] 定义协议MSG类型</li>
 *   <li>[必须] 定义协议MSG打包发包行为</li>
 *   <li>[必须] 定义添加C++调度器，定时器index为0</li>
 *   <li>[可选] 定义自定义参数</li>
 *   <li>[可选] 定义自定义命令</li>
 *   <li>[可选] 定义协议MSG初始化行为</li>
 *   <li>[可选] 定义协议MSG可视化序列化行为</li>
 *   <li>[可选] 定义定时器，并在Player构造函数中添加</li>
 *   </ol>
 *   <div style="color: Red;">调用关系图中的boost相关内容实际是std的东西。这里是doxygen误判,请自行脑补</div>
 *   @see include/Simulator.h (框架入口 - 头文件)
 *   @see simulator::Simulator (框架入口 - 接口)
 *   @see simulator::phoenix::* (phoenix 功能函数)
 *   @see include/DispatcherImpl.h (自定义调度器 - 头文件)
 *   @see simulator::DispatcherImpl (自定义调度器 - 接口)
 *   @see include/PlayerImpl.h (玩家类型 - 头文件)
 *   @see simulator::PlayerImpl (玩家类型 - 接口)
 *   @see include/ReqRspBase.h (自定义命令 - 头文件)
 *   @see simulator::ReqRspBase (自定义命令 - 接口)
 *   @see include/TimerImpl.h (定时器 - 头文件)
 *   @see simulator::TimerImpl (定时器 - 接口)
 *
 */

#ifndef __SIMULATOR_H_
#define __SIMULATOR_H_

#include "utility/AppConfig.h"

#include "CmdOption/CmdOption.h"
#include "utility/CmdShell.h"

#include "utility/ReqMgr.h"
#include "utility/RspMgr.h"
#include "utility/DispatcherMgr.h"
#include "utility/PlayerPool.h"

namespace simulator
{
    namespace detail
    {
        typedef std::function<int ()> init_fn_t;

        copt::CmdOptionCI::ptr_type BindingProgramParams();

        const std::string GenCmdHelpMsg(const char* strCmds, const char* strMessage, const int& iCmdFilterLen = 32);

        int AddInitFn(init_fn_t fn);

        int LocalInit(int argc, char *argv[]);
        int LocalActive();
        int LocalLoop();
    }

    // =========================== 通用动作 ===========================
    namespace phoenix
    {
        /**
         * @brief 通用赋值动作 - 设置一个变量值
         */
        template<typename T>
        struct AssignAction
        {
            T& tVar;
            AssignAction(T& t):tVar(t){}

            void operator()(copt::callback_param stParam)
            {
                if (stParam.GetParamsNumber() > 0)
                    tVar = stParam[0]->As<T>();
            }
        };

        /**
         * @brief 通用赋值动作 - 设置两个变量值
         */
        template<typename T1, typename T2>
        struct AssignActionEx2
        {
            T1& tVar1;
            T2& tVar2;
            AssignActionEx2(T1& t1, T2& t2): tVar1(t1), tVar2(t2){}

            void operator()(copt::callback_param stParam)
            {
                if (stParam.GetParamsNumber() > 0)
                    tVar1 = stParam[0]->As<T1>();
                if (stParam.GetParamsNumber() > 1)
                    tVar2 = stParam[1]->As<T2>();
            }
        };

        /**
         * @brief 通用赋值动作 - 容器push_back操作
         */
        template<typename T>
        struct PushBackStrAction
        {
            T& tVar;
            PushBackStrAction(T& t):tVar(t){}

            void operator()(copt::callback_param stParam)
            {
                for (copt::CmdOptionList::size_type i = 0; i < stParam.GetParamsNumber(); ++ i)
                {
                    tVar.push_back(stParam[i]->AsCppString());
                }

            }
        };

        /**
         * @brief 通用赋值动作 - 设置变量值为某个固定值
         */
        template<typename T>
        struct SetAction
        {
            T& tVar;
            T tVal;
            SetAction(T& t, const T& val):tVar(t), tVal(val){}

            void operator()(copt::callback_param stParam)
            {
                tVar = tVal;
            }
        };

        /**
         * @brief 通用赋值动作 - 设置一个变量为bool值并检查语义
         * @note no, false, disabled, disable, 0 都会被判定为false，其他为true
         */
        template<typename T>
        struct SetLogicBool
        {
            T& tVar;
            SetLogicBool(T& t):tVar(t){}

            void operator()(copt::callback_param stParam)
            {
                tVar = stParam[0]->AsLogicBool();
            }
        };
    }

    template<typename TMsgID, typename TMsg, typename TPlayer>
    struct Simulator
    {
        typedef TPlayer player_t;
        typedef TMsgID msg_id_t;
        typedef TMsg msg_t;
        typedef copt::CmdOptionCI::ptr_type option_mgr_t;

        typedef typename ReqMgr<msg_id_t, msg_t, player_t>::serialize_fn_t serialize_fn_t;
        typedef typename ReqMgr<msg_id_t, msg_t, player_t>::sender_fn_t sender_fn_t;
        typedef typename ReqMgr<msg_id_t, msg_t, player_t>::initmsg_fn_t initmsg_fn_t;
        typedef detail::init_fn_t init_fn_t;

        typedef typename RspMgr<msg_id_t, msg_t, player_t>::callback_func_type callback_func_type;
        typedef typename RspMgr<msg_id_t, msg_t, player_t>::pick_msgid_fn_t pick_msgid_fn_t;

        typedef DispatcherMgr::proc_fn proc_fn;
        typedef std::function<int (std::shared_ptr<player_t>) > player_proc_fn;

        /**
         * @brief 获取命令行参数绑定器，用于绑定额外命令选项
         * @return 绑定器智能指针
         */
        static option_mgr_t GetArgsMgr()
        {
            return detail::BindingProgramParams();
        }

        /**
         * @brief 添加额外的初始化函数，这些函数将会在系统初始化完成后，激活协议前执行
         * @param fn
         * @return 0或错误码
         */
        static int AddInitFunc(init_fn_t fn)
        {
            return detail::AddInitFn(fn);
        }

        /**
         * @brief 执行初始化
         * @param argc main函数传入的argc
         * @param argv main函数传入的argv
         * @param fn_e 协议可视化解包functor
         * @param fn_p 获取消息体ID的functor
         * @param fn_s 协议打包发送functor
         * @param fn_i 协议包初始化functor
         * @return 0或错误码
         */
        static int Init(int argc, char *argv[], serialize_fn_t fn_e, pick_msgid_fn_t fn_p, sender_fn_t fn_s, initmsg_fn_t fn_i)
        {
            detail::LocalInit(argc, argv);

            // =============== 初始化 ================
            RspMgr<msg_id_t, msg_t, player_t>::Instance()->Init(fn_e, fn_p);
            ReqMgr<msg_id_t, msg_t, player_t>::Instance()->Init(fn_s, fn_e, fn_i);

            DispatcherMgr::Instance()->Init();

            // 激活业务协议
            return detail::LocalActive();
        }

        /**
         * @brief 进入主循环
         * @return 0或错误码
         */
        static int MainLoop()
        {
            return detail::LocalLoop();
        }

        /**
         * 添加协议回调调度器
         * @param index 调度器ID
         * @note 添加调度器应该在初始化之前
         * @see simulator::EN_DISPATCHER_TYPE
         * @return 0或错误码
         */
        template<typename TDispatcher>
        static int AddDispatcher(int index)
        {
            return DispatcherMgr::Instance()->AddDispatcher<TDispatcher>(index);
        }

        /**
         * @brief 添加玩家Proc事件响应函数
         * @param fn 响应functor
         * @note 调用式 int (std::shared_ptr<玩家类型>); 返回值为成功处理的包数，小于0则中断后续事件响应和Dispatcher的ProcOne回调
         * @note 事件响应时可以通过调用 ProcOnePlayer 函数重入调度器ProcOne玩家的行为 <br />
         *       重入调度器时不响应Listener事件并且会导致系统不再自动响应调度器ProcOne行为
         * @note 可以在此尝试recv回包数据
         * @see ProcOnePlayer
         */
        static void AddPlayerProcListener(player_proc_fn fn)
        {
            DispatcherMgr::Instance()->AddPlayerProcListener(DispatcherProcHandler<player_t>(fn));
        }

        /**
         * @brief 执行或重入调度器ProcOne行为
         * @note 如果在 AddPlayerProcListener 的目标执行过程中调用本函数则是重入调度器。<br />
         *       重入调度器时不响应Listener事件并且会导致系统不再自动响应调度器ProcOne行为
         * @param player 对应的玩家
         * @return 成功处理的包数或错误码
         * @see AddPlayerProcListener
         */
        static int ProcOnePlayer(std::shared_ptr<PlayerImpl> player)
        {
            return DispatcherMgr::Instance()->ProcOne(player);
        }

        /**
         * @brief 添加主循环Proc事件响应函数
         * @param fn 响应functor
         * @note 调用式 int (); 返回值为成功处理的包数，小于0则中断后续事件响应和Dispatcher的Proc回调
         * @note 如果是epoll可以在此尝试获取可用的fd
         */
        static void AddProcListener(proc_fn fn)
        {
            DispatcherMgr::Instance()->AddProcListener(fn);
        }

        /**
         * @brief 回包解包完成，执行回调
         * @param pstPlayer
         * @param stPkg
         * @return 0或错误码
         */
        static int ProcessRspMsg(std::shared_ptr<player_t> pstPlayer, const msg_t& stPkg)
        {
            return RspMgr<msg_id_t, msg_t, player_t>::Instance()->ProcessOnePkg(pstPlayer, stPkg);
        }

        /**
         * @brief 创建玩家对象并添加到玩家池
         * @param strID 玩家ID或OpenID
         * @return 玩家对象智能指针
         */
        static std::shared_ptr<player_t> CreatePlayer(const std::string& strID)
        {
            return std::dynamic_pointer_cast<player_t>(PlayerPool::CreatePlayer<player_t>(strID));
        }

        /**
         * @brief 获取当前前台命令行线程操作的玩家对象
         * @return 玩家对象智能指针
         */
        static std::shared_ptr<player_t> GetCmdPlayer()
        {
            return std::dynamic_pointer_cast<player_t>(PlayerPool::GetCmdPlayer());
        }

        /**
         * @brief 设置当前前台命令行线程操作的玩家对象
         * @param player 玩家对象智能指针
         */
        static void SetCmdPlayer(std::shared_ptr<player_t> player)
        {
            PlayerPool::SetCmdPlayer(std::dynamic_pointer_cast<PlayerImpl>(player));
        }
    };
}


#endif /* SIMULATOR_H_ */
