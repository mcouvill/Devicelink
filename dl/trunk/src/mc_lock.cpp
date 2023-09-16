#include "mc_lock.h"
/*! \brief MC_Lock class for thread safe operation.

Handles locking on entry and auomatically unlocks when going out of scope by 
using the deconstructor to do so.  Prevents threads hanging in case of some
sort of untrapped exception being thrown.

*/

MC_Lock::MC_Lock(MC_CritSection* pCS)
    {
        m_pCS = pCS;
        if(m_pCS)
            m_pCS->Enter();
    }

/*! \brief Deconstructor which automatically closes out the lock when it goes out of scope.

*/

MC_Lock::~MC_Lock()
    {
        if(m_pCS)
            m_pCS->Leave();
    }