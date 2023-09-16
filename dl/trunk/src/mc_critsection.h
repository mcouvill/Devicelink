/*! \file mc_critsection.h
	\brief The header file for the critical section class for thread safe operation

*/
#pragma once
#include "windows.h"

/*!	\brief The critical section class for thread safe operation

*/
class MC_CritSection
{
	long m_nLockCnt; //!< keeps count of the number of locks
	long m_nThreadId; //!< stores the thread id
	bool SetLock(const long nThreadId); //!< sets a lock on the thread indicated by the ID.

public:

	MC_CritSection();
    void Enter(); //!< Function for entering a critical section
    void Leave(); //!< Function for leaving a critical section
    bool Try(); //!< try block call
	
	~MC_CritSection();
};
