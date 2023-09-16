#include "mc_critsection.h"

/*! \brief Constructor for MC_CritSection. Initializes the ID and cnt.

*/

MC_CritSection::MC_CritSection()
    {
        m_nThreadId = 0;
        m_nLockCnt = 0;
    }

    void MC_CritSection::Enter()
	{
	} //!< Function for entering a critical section
    void MC_CritSection::Leave()
	{
	} //!< Function for leaving a critical section
    bool MC_CritSection::Try()
	{
		return true;
	} //!< try block call
	
MC_CritSection::~MC_CritSection()
{
}