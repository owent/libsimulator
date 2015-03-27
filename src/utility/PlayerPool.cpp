
#include <ctime>

#include "std/foreach.h"

#include "utility/AppConfig.h"
#include "utility/ReqMgr.h"
#include "utility/RspMgr.h"
#include "utility/DispatcherMgr.h"

#include "utility/PlayerPool.h"

#include "utility/CmdShell.h"

namespace simulator
{

    std::map<std::string, std::shared_ptr<PlayerImpl> > PlayerPool::m_stPlayerMap;
    std::shared_ptr<PlayerImpl> PlayerPool::m_pCmdPlayer;
    util::lock::SpinLock PlayerPool::m_stPlayerOprLock;

    // 最大用户线程数
    struct ThreadData
    {
        std::list< std::shared_ptr<PlayerImpl> > players;
        thread_t thread;
        util::lock::SpinLock lock;
        ThreadData(): thread(0){}
    };

    struct ThreadDataPool
    {
        ThreadData *pPool;
        int iMaxNumber;
        ThreadDataPool(): pPool(NULL), iMaxNumber(0) {}
        ~ThreadDataPool()
        {
            if (NULL != pPool)
            {
                delete []pPool;
                pPool = NULL;
                iMaxNumber = 0;
            }
        }

        ThreadData& operator[](int index)
        {
            if (NULL == pPool)
            {
                iMaxNumber = AppConfig::m_iPlayerThreadPoolMax;
                pPool = new ThreadData[iMaxNumber];
            }

            return *(pPool + index);
        }
    };

    static ThreadDataPool g_szThreads;

    std::shared_ptr<PlayerImpl> PlayerPool::FindPlayerByID(const std::string& strID)
    {
        std::map<std::string, std::shared_ptr<PlayerImpl> >::iterator iter = m_stPlayerMap.find(strID);
        if(iter == m_stPlayerMap.end())
        {
            return std::shared_ptr<PlayerImpl>();
        }

        return iter->second;
    }

    int PlayerPool::InsertPlayerCmd(std::shared_ptr<PlayerImpl> pPlayer, const std::string& strCmd)
    {
        // 如果没有当前用户直接执行
        if (NULL == pPlayer.get())
        {
            if (ReqMgrBase::Inst)
            {
                ReqMgrBase::Inst->ProcessCmd(pPlayer, strCmd);
            }
            return 0;
        }

        pPlayer->InsertCmd(strCmd);
        return 0;
    }

    std::shared_ptr<PlayerImpl> PlayerPool::AddPlayer(const std::string& strID, std::shared_ptr<PlayerImpl> player)
    {
        std::shared_ptr<PlayerImpl> pRet = FindPlayerByID(strID);
        if (NULL != pRet.get())
        {
            return pRet;
        }

        // 新对象
        pRet = player;
        pRet->m_strID = strID;

        // 选择一个工作线程
        // 如果用户数较少，则每个用户有单独的工作线程
        // 如果用户数很多，则多个用户公用线程，防止线程切换开销过高
        int iSelectedThread = 0;
        for (int i = 1; i < AppConfig::m_iPlayerThreadPoolMax; ++ i)
        {
            if (g_szThreads[i].players.size() < g_szThreads[iSelectedThread].players.size())
            {
                iSelectedThread = i;
            }
        }

        while (false == g_szThreads[iSelectedThread].lock.TryLock());
        g_szThreads[iSelectedThread].players.push_back(pRet);

        if (0 == g_szThreads[iSelectedThread].thread)
        {
            // 工作线程启动
            THREAD_CREATE(pRet->m_pProcThread, PlayerPool::PlayerProcFunc, &g_szThreads[iSelectedThread]);
            g_szThreads[iSelectedThread].thread = pRet->m_pProcThread;
        }
        g_szThreads[iSelectedThread].lock.Unlock();


        m_stPlayerOprLock.Lock();
        m_stPlayerMap[strID] = pRet;
        m_stPlayerOprLock.Unlock();
        return pRet;
    }

    void PlayerPool::GetAllPlayer(std::vector<std::shared_ptr<PlayerImpl> >& stVec)
    {
        std::map<std::string, std::shared_ptr<PlayerImpl> >::iterator iter = m_stPlayerMap.begin();
        for(; iter != m_stPlayerMap.end(); ++ iter)
        {
            stVec.push_back(iter->second);
        }
    }

    std::shared_ptr<PlayerImpl> PlayerPool::GetCmdPlayer()
    {
        return m_pCmdPlayer;
    }

    void PlayerPool::SetCmdPlayer(std::shared_ptr<PlayerImpl> player)
    {
        m_pCmdPlayer = player;

        // 设置命令行提示
        if (player)
        {
            CmdShell::Instance()->SetPrompt(player->GetID() + ">");
        }
    }

    void* PlayerPool::PlayerProcFunc(void* pThreadData)
    {
        // ==================== 线程资源初始化 ====================
        DispatcherMgr::Instance()->OnPlayerThreadStart();
        // -------------------- 线程资源初始化 --------------------

        ThreadData* pThreadInfo = (ThreadData*)pThreadData;
        int iRspNum = 0;
        while (
            AppConfig::m_bIsContinue &&
            (!AppConfig::m_bIsNoInteractive || !pThreadInfo->players.empty()) // 无交互模式下，玩家池为空时线程直接退出
        )
        {
            if (iRspNum <= 0)
            {
                // 上次无回包，这次就sleep一下，防止空跑CPU
                THREAD_SLEEP_MS(10);
            }

            iRspNum = DispatcherMgr::Instance()->Proc();

            std::list< std::shared_ptr<PlayerImpl> >::iterator iter = pThreadInfo->players.begin();
            while(iter != pThreadInfo->players.end())
            {
                std::list< std::shared_ptr<PlayerImpl> >::iterator cur = iter ++;
                std::shared_ptr<PlayerImpl> ptrPlayer = *cur;
                if(ptrPlayer->m_bIsAvailable)
                {
                    time_t tTimeNow = time(NULL);

                    int iPlayerRspNum = 0;

                    // 处理回包
                    while(iPlayerRspNum < 10)
                    {
                        int iRes = DispatcherMgr::Instance()->ProcOne(ptrPlayer);
                        if (iRes <= 0)
                        {
                            break;
                        }

                        iPlayerRspNum += iRes; // 防止其他Player进入回包处理饥饿状态
                    }
                    iRspNum += iPlayerRspNum;

                    // 处理请求命令
                    while(false == ptrPlayer->m_stCmdListLock.TryLock())
                    {
                        THREAD_SLEEP_MS(1);
                    }

                    // 执行时不加锁，防止内部添加指令死锁
                    std::list<std::string> strCmdList;
                    strCmdList.swap(ptrPlayer->m_stCmdList);

                    ptrPlayer->m_stCmdList.clear();
                    ptrPlayer->m_stCmdListLock.Unlock();

                    if (ReqMgrBase::Inst)
                    {
                        owent_foreach(std::string& strCmd, strCmdList)
                        {
                            ReqMgrBase::Inst->ProcessCmd(ptrPlayer, strCmd);
                        }
                    }

                    // 处理 lua timeout 回调
                    ptrPlayer->Tick(tTimeNow);
                }
                else
                {
                    // 角色失效则移除
                    while(false == m_stPlayerOprLock.TryLock());
                    while(false == pThreadInfo->lock.TryLock());

                    m_stPlayerMap.erase(ptrPlayer->GetID());
                    pThreadInfo->players.erase(cur);

                    pThreadInfo->lock.Unlock();
                    m_stPlayerOprLock.Unlock();
                }
            }
        }

        // 释放全部数据
        while(false == m_stPlayerOprLock.TryLock());
        while(false == pThreadInfo->lock.TryLock());

        std::list< std::shared_ptr<PlayerImpl> >::iterator iter = pThreadInfo->players.begin();
        for (; iter != pThreadInfo->players.end(); ++ iter)
        {
            m_stPlayerMap.erase((*iter)->GetID());
        }
        pThreadInfo->players.clear();

        pThreadInfo->lock.Unlock();
        m_stPlayerOprLock.Unlock();

        // ==================== 线程资源释放 ====================
        DispatcherMgr::Instance()->OnPlayerThreadEnd();
        // -------------------- 线程资源释放 --------------------

        pThreadInfo->thread = 0;

        THREAD_RETURN;
    }

    void PlayerPool::JoinAllPlayerThread()
    {
        for (int i = 0; i < AppConfig::m_iPlayerThreadPoolMax; ++ i)
        {
            if (g_szThreads[i].thread)
            {
                THREAD_JOIN(g_szThreads[i].thread);
            }
        }
    }
}
