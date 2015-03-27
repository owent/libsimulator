/**
 * Licensed under the MIT licenses.
 *
 * @file ShellCmd.h
 * @brief 
 *
 *
 * @version 1.0
 * @author owent, admin@owent.net
 * @date 2013年9月27日
 *
 * @history
 *
 */

#ifndef _SIMU_UTILITY_SHELLCMD_H_
#define _SIMU_UTILITY_SHELLCMD_H_

#include "CmdOption/CmdOption.h"

namespace simulator
{
    copt::CmdOptionCI::ptr_type BindingSysCmd(bool& isSysCmd);
}

#endif /* SHELLCMD_H_ */
