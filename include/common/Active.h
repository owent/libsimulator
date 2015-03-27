/**
 * Licensed under the MIT licenses.
 *
 * @file Active.h
 * @brief 
 *
 *
 * @version 1.0
 * @author owent, admin@owent.net
 * @date 2014年6月24日
 *
 * @history
 *
 */

#ifndef _SIMU_COMMON_ACTIVE_H_
#define _SIMU_COMMON_ACTIVE_H_

namespace proto
{

    namespace detail
    {
        void _add_active_fn(void (*fn)());

        template<typename TC>
        class Activitor
        {
        public:
            Activitor()
            {
                _add_active_fn(TC::Active);
            }

        };
    }

};

#define ACTIVE_PROTO_OBJ_NAME(class_name) ProtoActivitor##class_name

#define ACTIVE_PROTO(T) \
    namespace { \
        static ::proto::detail::Activitor<T> ACTIVE_PROTO_OBJ_NAME(T); \
    }

#endif /* ACTIVE_H_ */
