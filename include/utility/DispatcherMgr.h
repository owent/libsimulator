/**
 * Licensed under the MIT licenses.
 *
 * @file DispatcherMgr.h
 * @brief 
 *
 *
 * @version 1.0
 * @author owent, admin@owent.net
 * @date 2014年6月25日
 *
 * @history
 *
 */

#ifndef _SIMU_UTILITY_DISPATCHERMGR_H_
#define _SIMU_UTILITY_DISPATCHERMGR_H_

#include <map>
#include <list>
#include <bitset>

#include "ext/Singleton.h"

#include "utility/DispatcherImpl.h"

namespace simulator
{
    /**
     * @brief 调度器管理器
     */
    class DispatcherMgr : public Singleton<DispatcherMgr>
    {
    public:
        typedef std::map<int, std::shared_ptr<DispatcherImpl> > value_type;
        typedef std::function<int (std::shared_ptr<PlayerImpl>) > player_proc_fn;
        typedef std::function<int () > proc_fn;

        std::bitset<EN_DT_MAX> m_stDefaultStatus;

    public:

        int Init();

        /**
         * @brief 获取新玩家默认的调度器状态
         * @return 调度器状态
         */
        inline std::bitset<EN_DT_MAX>& GetDefaultStatus() { return m_stDefaultStatus; }

        /**
         * @brief 获取新玩家默认的调度器状态
         * @return 调度器状态
         */
        inline const std::bitset<EN_DT_MAX>& GetDefaultStatus() const { return m_stDefaultStatus; }

        /**
         * @brief 添加调度器
         * @param index 调度器索引
         * @return 0或错误码
         */
        template<typename TD>
        int AddDispatcher(int index)
        {
            if (index >= EN_DT_MAX || index < 0)
            {
                return -1;
            }

            if (m_stDispatchers.end() != m_stDispatchers.find(index))
            {
                return -2;
            }

            DispatcherImpl* pDispatcher = new TD();
            if (NULL == pDispatcher)
            {
                return -3;
            }

            pDispatcher->m_iIndex = index;
            m_stDispatchers[index] = std::shared_ptr<DispatcherImpl>(pDispatcher);
            m_stDefaultStatus.set(index, pDispatcher->IsInitEnabled());
            return 0;
        }

        /**
         * @brief 触发线程启动事件
         * @return 0或错误码
         */
        int OnPlayerThreadStart();

        /**
         * @brief 触发线程结束事件
         * @return 0或错误码
         */
        int OnPlayerThreadEnd();

        /**
         * @brief 触发玩家Proc事件
         * @note 【可重入】嵌套调用时最外层会关闭调度器对ProcOne函数的调用
         * @param player 触发事件的玩家
         * @return 0或错误码
         */
        int ProcOne(std::shared_ptr<PlayerImpl> player);

        /**
         * @brief 触发全局Proc事件
         * @return 0或错误码
         */
        int Proc();

        /**
         * @brief 添加玩家ProcOne事件监听
         * @param fn
         */
        void AddPlayerProcListener(player_proc_fn fn);

        /**
         * @brief 添加全局Proc事件监听
         * @param fn
         */
        void AddProcListener(proc_fn fn);
    private:
        value_type m_stDispatchers;
        std::list<player_proc_fn> m_stPlayerProcHandles;
        std::list<proc_fn> m_stProcHandles;
    };
}


#endif /* _SIMULATOR_DISPATCHERMGR_H_ */
