#ifndef _SIMU_UTILITY_PLAYER_H_
#define _SIMU_UTILITY_PLAYER_H_

#include <cstddef>
#include <list>
#include <string>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <map>
#include <ctime>
#include <bitset>



#include "ext/Lock/SpinLock.h"
#include "utility/thread.h"
#include "std/smart_ptr.h"

#include "utility/AppConfig.h"

#include "utility/TimerImpl.h"

namespace simulator
{
    /**
     * @brief 调度器类型
     */
    enum EN_DISPATCHER_TYPE
    {
        EN_DT_CPP = 0,     //!< EN_DT_CPP C++调度器必须手动添加，其他为可选

        EN_DT_SYS_MAX = 32,//!< EN_DT_SYS_MAX

        EN_DT_MAX = 128    //!< EN_DT_MAX
    };

    class DispatcherMgr;
    class PlayerImpl
    {
    public:
        typedef std::shared_ptr<PlayerImpl> ptr_t;

    public:
        PlayerImpl();
        PlayerImpl(const std::string& id);
        virtual ~PlayerImpl();

        /**
         * @brief 创建角色后触发
         * @return 返回值小于0表示创建失败，如果玩家创建失败则不会进入玩家池。稍后会直接析构
         */
        virtual int OnCreated();

        /**
         * @brief 创建角色并加入到玩家池后初始化时触发
         * @return 返回值小于0表示初始化失败，如果玩家初始化失败则会标记为不可用，单不会被删除。稍后在玩家线程循环中被移出
         */
        virtual int Init();

        /**
         * @brief 插入命令行指令
         * @param strCmd 命令行指令
         */
        void InsertCmd(const std::string& strCmd);

        /**
         * @brief 获取命令列表
         * @return 命令列表,不可更改
         */
        const std::list<std::string>& GetCmdList();

        /**
         * @brief 移除命令
         * @param start 开始位置下标
         * @param end 结束位置下标
         * @note 移除范围是[start, end)
         */
        void RemoveCmd(size_t start, size_t end);

        /**
         * @brief 设置为已登入状态
         */
        void SetLogined();

        /**
         * @brief 判断是否已登入
         * @return 是否已登入
         */
        bool IsLogined() const;

        /**
         * @brief 获取ID
         * @return 玩家唯一ID
         */
        const std::string& GetID() const;

        /**
         * @brief 标记为不可用，稍后在玩家线程循环中被移出
         */
        void Exit();

        /**
         * @brief 启用调度器
         * @param index 调度器id
         */
        void EnableDispatcher(int index);

        /**
         * @brief 关闭调度器
         * @param index 调度器id
         */
        void DisableDispatcher(int index);

        /**
         * @brief 查看调度器是否已启用
         * @param index 调度器id
         * @return 已启用返回true
         */
        bool IsDispatcherEnable(int index);

        /**
         * @brief 添加定时器
         * @return 成功返回0
         */
        template<typename TT>
        TT* AddTimer()
        {
            TT* timer = new TT();

            // mmap number extended or memory limit
            if (NULL == timer)
            {
                return NULL;
            }

            timer->m_pOwner = this;

            m_stTimers.push_back(std::shared_ptr<TimerImpl>(timer));
            return timer;
        }

        /**
         * 触发定时器Tick事件
         * @param sec 当前秒数
         * @return 0或错误码
         */
        int Tick(time_t sec);

    private:
        util::lock::SpinLock m_stCmdListLock;
        std::list<std::string> m_stCmdList;

        std::string m_strID;

        bool m_bIsLogged;
        bool m_bIsAvailable;
        bool m_bIsInited;
        int m_iProcLevel;

        thread_t m_pProcThread; /** 角色工作线程 **/

        std::bitset<EN_DT_MAX> m_stDispatcherStatus;
        std::list<std::shared_ptr<TimerImpl> > m_stTimers;

        friend class PlayerPool;
        friend class DispatcherMgr;
    };
}
#endif
