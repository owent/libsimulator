#ifndef _SIMU_UTILITY_PLAYERPOOL_H_
#define _SIMU_UTILITY_PLAYERPOOL_H_

#include <map>
#include <vector>
#include <stdint.h>
#include <string>
#include "std/functional.h"
#include "std/smart_ptr.h"

#include "utility/PlayerImpl.h"

namespace simulator
{
    /**
     * @brief 玩家池
     */
    class PlayerPool
    {
    public:
        /**
         * @brief 查找玩家
         * @param strID ID或OpenID
         * @return 玩家对象智能指针
         */
        static std::shared_ptr<PlayerImpl> FindPlayerByID(const std::string& strID);

        /**
         * 插入角色指令到待执行列表
         * @param [in] pPlayer 角色玩家
         * @param [in] strCmd 命令行
         * @return 0或错误码
         */
        static int InsertPlayerCmd(std::shared_ptr<PlayerImpl> pPlayer, const std::string& strCmd);

        /**
         * @brief 创建玩家并添加到玩家池
         * @param strID ID
         * @return 玩家对象智能指针
         */
        template<typename TPlayer>
        static std::shared_ptr<PlayerImpl> CreatePlayer(const std::string& strID)
        {
            std::shared_ptr<PlayerImpl> pRet = FindPlayerByID(strID);
            if (NULL != pRet.get())
            {
                return pRet;
            }

            pRet = std::shared_ptr<PlayerImpl>(new TPlayer());
            if (!pRet)
            {
                return pRet;
            }
            pRet->m_strID = strID;

            // 创建回调失败则不分配CPU资源
            if (pRet->OnCreated() < 0)
            {
                return std::shared_ptr<PlayerImpl>();
            }

            pRet = AddPlayer(strID, pRet);

            // 初始化失败则直接失效
            if (pRet->Init() < 0)
            {
                pRet->m_bIsAvailable = false;
            }
            return pRet;
        }

        /**
         * @brief 添加到玩家池
         * @note 该操作会分配玩家资源
         * @param strID 玩家ID
         * @param player 玩家对象智能指针
         * @return 玩家对象智能指针
         */
        static std::shared_ptr<PlayerImpl> AddPlayer(const std::string& strID, std::shared_ptr<PlayerImpl> player);

        /**
         * @brief 获取所有玩家
         * @param stVec 将输出包含玩家对象智能指针的vector
         */
        static void GetAllPlayer(std::vector<std::shared_ptr<PlayerImpl> >& stVec);

        /**
         * @brief 获取当前前台命令行线程操作的玩家对象
         * @return 玩家对象智能指针
         */
        static std::shared_ptr<PlayerImpl> GetCmdPlayer();

        /**
         * @brief 设置当前前台命令行线程操作的玩家对象
         * @param player 玩家对象智能指针
         */
        static void SetCmdPlayer(std::shared_ptr<PlayerImpl> player);

        /**
         * @brief 后台线程池启动函数(请不要直接调用)
         * @param 线程参数
         */
        static void* PlayerProcFunc(void*);

        /**
         * @brief 等待所有线程池退出
         */
        static void JoinAllPlayerThread();
    private:
        static std::map<std::string, std::shared_ptr<PlayerImpl> > m_stPlayerMap;
        static std::shared_ptr<PlayerImpl> m_pCmdPlayer;
        static util::lock::SpinLock m_stPlayerOprLock;
    };
}
#endif
