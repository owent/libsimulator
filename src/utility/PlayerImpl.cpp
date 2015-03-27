#include <sys/socket.h>
#include <netdb.h>

#include <fstream>
#include <ctime>

#include <std/smart_ptr.h>
#include <std/foreach.h>

#include "utility/RspMgr.h"
#include "utility/DispatcherMgr.h"

#include "utility/PlayerPool.h"
#include "utility/PlayerImpl.h"

namespace simulator
{

    PlayerImpl::PlayerImpl(): m_strID("")
    {
        m_bIsLogged = false;
        m_bIsAvailable = true;
        m_bIsInited = false;
        m_iProcLevel = 0;
        m_pProcThread = 0;

        m_stDispatcherStatus = DispatcherMgr::Instance()->GetDefaultStatus();
    }

    PlayerImpl::PlayerImpl(const std::string& id): m_strID(id)
    {
        m_bIsLogged = false;
        m_bIsAvailable = true;
        m_bIsInited = false;
        m_iProcLevel = 0;
        m_pProcThread = 0;

        m_stDispatcherStatus = DispatcherMgr::Instance()->GetDefaultStatus();
    }

    PlayerImpl::~PlayerImpl()
    {
    }

    int PlayerImpl::OnCreated()
    {
        return 0;
    }

    int PlayerImpl::Init()
    {
        // 保证只初始化一次
        if (m_bIsInited)
        {
            return -1;
        }

        m_bIsInited = true;
        owent_foreach(std::shared_ptr<TimerImpl>& timer, m_stTimers)
        {
            timer->Init();
        }

        return 0;
    }

    void PlayerImpl::InsertCmd(const std::string& strCmd)
    {
        // 插入指令队列
        m_stCmdListLock.Lock();
        m_stCmdList.push_back(strCmd);
        m_stCmdListLock.Unlock();
    }

    const std::list<std::string>& PlayerImpl::GetCmdList()
    {
        return m_stCmdList;
    }

    void PlayerImpl::RemoveCmd(size_t start, size_t end)
    {
        if (end <= start)
            return;

        std::list<std::string>::iterator iter = m_stCmdList.begin();
        while (start > 0 && iter != m_stCmdList.end())
        {
            ++ iter;
            -- start;
            -- end;
        }

        m_stCmdListLock.Lock();
        while (end > 0 && iter != m_stCmdList.end())
        {
            m_stCmdList.erase(iter ++);
            -- end;
        }
        m_stCmdListLock.Unlock();
    }

    void PlayerImpl::SetLogined()
    {
        m_bIsLogged = true;
    }

    bool PlayerImpl::IsLogined() const
    {
        return m_bIsLogged;
    }

    const std::string& PlayerImpl::GetID() const
    {
        return m_strID;
    }

    void PlayerImpl::Exit()
    {
        m_bIsAvailable = false;
    }

    void PlayerImpl::EnableDispatcher(int index)
    {
        m_stDispatcherStatus.set(index, true);
    }

    void PlayerImpl::DisableDispatcher(int index)
    {
        m_stDispatcherStatus.set(index, false);
    }

    bool PlayerImpl::IsDispatcherEnable(int index)
    {
        return m_stDispatcherStatus.test(index);
    }

    int PlayerImpl::Tick(time_t sec)
    {
        owent_foreach(std::shared_ptr<TimerImpl>& timer, m_stTimers)
        {
            timer->Tick(sec);
        }

        return 0;
    }

}

