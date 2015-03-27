/**
 * Licensed under the MIT licenses.
 *
 * @file DispatcherImpl.h
 * @brief 
 *
 *
 * @version 1.0
 * @author owent, admin@owent.net
 * @date 2014年6月26日
 *
 * @history
 *
 */

#ifndef _SIMU_UTILITY_DISPATCHERIMPL_H_
#define _SIMU_UTILITY_DISPATCHERIMPL_H_

#include "utility/PlayerImpl.h"
#include "std/smart_ptr.h"
#include "std/functional.h"

namespace simulator
{
    class DispatcherMgr;
    class DispatcherImpl
    {
    private:
        int m_iIndex;
        friend class DispatcherMgr;

    public:
        virtual ~DispatcherImpl();

        virtual int Init() = 0;

        inline int GetIndex() { return m_iIndex; };

        /**
         * @brief 执行主循环逻辑，每次主循环都会调用一次
         * @note 返回值和主循环休眠有关
         * @return 返回有效处理的协议包数量
         */
        virtual int Proc() { return 0; };

        /**
         * @brief 执行主循环逻辑，每次主循环对每个玩家都会调用一次
         * @note 返回值和主循环休眠有关
         * @return 返回有效处理的协议包数量
         */
        virtual int ProcOne(std::shared_ptr<PlayerImpl> player) { return 0; };

        /**
         * @brief 是否初始可用,建议CPP调度器初始即可用
         * @return 返回true则初始就处于激活状态
         */
        virtual bool IsInitEnabled() { return false; };

        /**
         * @brief 线程启动时调用
         * @return 0或错误码
         */
        virtual int OnPlayerThreadStart();

        /**
         * @brief 线程即将退出时调用
         * @return 0或错误码
         */
        virtual int OnPlayerThreadEnd();
    };

    /**
     * @brief 协议调度器
     */
    template<typename TPlayer>
    class DispatcherBase: public DispatcherImpl
    {
    public:
        virtual ~DispatcherBase(){}

        /**
         * @note 返回值和主循环休眠有关
         * @return 返回有效处理的协议包数量
         */
        virtual int ProcOne(std::shared_ptr<TPlayer> player) { return 0; };

        virtual int OnPlayerThreadStart() { return 0; }
        virtual int OnPlayerThreadEnd() { return 0; }

        virtual int ProcOne(std::shared_ptr<PlayerImpl> player)
        {
            return ProcOne(std::dynamic_pointer_cast<TPlayer>(player));
        }
    };

    template<typename TPlayer>
    class DispatcherProcHandler
    {
    public:
        typedef std::function<int (std::shared_ptr<TPlayer>)> func_t;
        func_t func;

        DispatcherProcHandler(func_t fn): func(fn){}
        int operator()(std::shared_ptr<PlayerImpl> player)
        {
            if (func)
            {
                return func(std::dynamic_pointer_cast<TPlayer>(player));
            }
            return 0;
        }
    };

}

#endif /* _SIMU_DISPATCHERIMPL_H_ */
