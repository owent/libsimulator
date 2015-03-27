/**
 * Licensed under the MIT licenses.
 *
 * @file Active.cpp
 * @brief 统一协议激活接口
 *
 *
 * @version 1.0
 * @author owent, admin@owent.net
 * @date 2013-02-06
 *
 * @history
 *
 */

#include <list>

namespace proto
{
    namespace detail
    {

        static std::list<void (*)()>& _get_active_fn()
        {
            static std::list<void (*)()> ret;
            return ret;
        }

        void _add_active_fn(void (*fn)())
        {
            _get_active_fn().push_back(fn);
        }
    }
}

void active_protos()
{
    typedef std::list<void (*)()> list_type;
    list_type& fn_list = ::proto::detail::_get_active_fn();

    for(list_type::iterator iter = fn_list.begin(); iter != fn_list.end(); ++ iter)
    {
        (**iter)();
    }
}


