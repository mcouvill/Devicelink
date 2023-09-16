/*! \file mc_lock.h
	\brief The header for the thread locking class for thread safe operation. Uses mc_critsection

*/
#pragma once
#include "MC_CritSection.h"

class MC_Lock
{
    MC_CritSection* m_pCS; //!< var for creating MC_CritSection object

public:
/*! \brief This sets up the lock for the accessing thread.

*/
    MC_Lock(MC_CritSection* pCS);
    ~MC_Lock();
};
