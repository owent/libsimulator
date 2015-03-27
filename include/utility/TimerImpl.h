/**
 * Licensed under the MIT licenses.
 *
 * @file TimerImpl.h
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

#ifndef _SIMU_UTILITY_TIMERIMPL_H_
#define _SIMU_UTILITY_TIMERIMPL_H_

#include <ctime>

namespace simulator
{
    class PlayerImpl;
    class TimerImpl
    {
    public:
        TimerImpl(): m_pOwner(NULL) {}
        virtual ~TimerImpl(){}

        virtual int Init() = 0;
        virtual int Tick(time_t sec) = 0;

        template<typename TPlayer>
        TPlayer* GetOwner()
        {
            return dynamic_cast<TPlayer*>(m_pOwner);
        }

    private:
        friend class PlayerImpl;
        PlayerImpl* m_pOwner;
    };
}


#endif /* _SIMULATOR_TIMERIMPL_H_ */
