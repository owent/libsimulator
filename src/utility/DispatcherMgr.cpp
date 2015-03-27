/**
 * Licensed under the MIT licenses.
 *
 * @file DispatcherMgr.cpp
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

#include "std/foreach.h"

#include "utility/DispatcherMgr.h"
#include "utility/DispatcherImpl.h"

namespace simulator
{
    int DispatcherMgr::Init()
    {
        for (value_type::iterator iter = m_stDispatchers.begin(); iter != m_stDispatchers.end(); ++ iter)
        {
            iter->second->Init();
        }

        return 0;
    }

    int DispatcherMgr::OnPlayerThreadStart()
    {
        for (value_type::iterator iter = m_stDispatchers.begin(); iter != m_stDispatchers.end(); ++ iter)
        {
            iter->second->OnPlayerThreadStart();
        }

        return 0;
    }

    int DispatcherMgr::OnPlayerThreadEnd()
    {
        for (value_type::iterator iter = m_stDispatchers.begin(); iter != m_stDispatchers.end(); ++ iter)
        {
            iter->second->OnPlayerThreadEnd();
        }

        return 0;
    }

    int DispatcherMgr::ProcOne(std::shared_ptr<PlayerImpl> player)
    {
        int ret = 0;
        bool bFirstLevel = (player->m_iProcLevel <= 0);
        ++ player->m_iProcLevel;

        if (bFirstLevel)
        {
            owent_foreach(player_proc_fn& fn, m_stPlayerProcHandles)
            {
                int res = fn(player);
                if(res < 0)
                {
                    return res;
                }
                ret += res;
            }
        }

        // 如果只有第一层或是调度器重入则执行调度
        if (!bFirstLevel || 1 == player->m_iProcLevel)
        {
            for (value_type::iterator iter = m_stDispatchers.begin(); iter != m_stDispatchers.end(); ++ iter)
            {
                if (player->IsDispatcherEnable(iter->first))
                {
                    int res = iter->second->ProcOne(player);
                    ret += res > 0? res: 0;
                }
            }

        }

        // 最外层负责清0
        if (bFirstLevel)
        {
            player->m_iProcLevel = 0;
        }

        return ret;
    }

    int DispatcherMgr::Proc()
    {
        int ret = 0;
        owent_foreach(proc_fn& fn, m_stProcHandles)
        {
            int res = fn();
            if(res < 0)
            {
                return res;
            }
            ret += res;
        }

        for (value_type::iterator iter = m_stDispatchers.begin(); iter != m_stDispatchers.end(); ++ iter)
        {
            int res = iter->second->Proc();
            ret += res > 0? res: 0;
        }

        return ret;
    }

    void DispatcherMgr::AddPlayerProcListener(player_proc_fn fn)
    {
        m_stPlayerProcHandles.push_back(fn);
    }

    void DispatcherMgr::AddProcListener(proc_fn fn)
    {
        m_stProcHandles.push_back(fn);
    }
}

