Devicelink Wrapper Version 2.1.4.1

In this zip file I included the devicelink.cpp and devicelink.h wrapper class files.  
You can link these in directly to your project.  The project file has been upgraded to VS 2003.
There is also a sample config.ini file as well.  Discussion about this library can be found at:
http://www.wingwalkers.org/Forum/index.php You have to register to post, but not to read.

Changes:
v2.1.4.1
-- Fixed an error in Get_Roll that was erroneously returning m_beacon_azimuth rather than m_roll.
Cut and paste errors are a pain!  Thanks to "sirnoname" for the bug report.

Changes:
v2.1.4
-- Added GetCurCockpit method to return what cockpit the user is currently in. 0 is always pilot
-- Added Get_Overload method
-- Added Get_ShakeLvl method
-- removed private selecteng method. Turns it was identical to toggleswitch
-- Added SelectAllEngines method
-- Added UnSelectAllEngines method
-- Added SelectLeftEngines method
-- Added SelectRightEngines method
-- Added ToggleSelAllEngines method
-- Added ToggleSelLeftEngines method
-- Added ToggleSelRightEngines method
-- Added ToggleSelEng1 method
-- Added ToggleSelEng2 method
-- Added ToggleSelEng3 method
-- Added ToggleSelEng4 method
-- Added Query, Set and Get methods for _WingFold
-- Added Query, Set and Get methods for _Canopy
-- Added Query, Set and Get methods for _TailHook
-- Added Query, Set and Get methods for _Chocks
-- Added Query, Set and Get Methods for _Gunner
-- Added JumpToCockpit for jumping up to 10 different cockpits (max game supports)
-- Corrected several comments for functions so Doxygen could produce the proper 
annotated function and private variable descriptions


v2.1.3
-- refactored SelectEng1-4 methods into a single private method of selecteng(code). Maintained the API
so it is backwards compatible.
-- Added enum WeapType {MG, CANNON, ROCKETS, BOMBS, MGCANNON} for setting up the Query, Set and Get of Weapons
-- corrected logic error in setctrl() method when compiling under Visual Studio 2005.
-- bumped SendMsg buffer to 80 bytes from 64.
-- added a private array m_weap of WeapType. It's initialized in the constructor.
-- added Query_Gunpods to query game on whether gunpods are on or off and assign to private m_gunpods
-- added ToggleGunPods to switch them on/off
-- added GetGunPodsState which returns m_gunpods to a higher application
-- changed constructor to initialize many of the private variables rather than assign them values. Little cleaner that way.
-- cleaned up many routines that were adding extraneous "R/" to command codes.
-- Added ToggleBombSight method for the auto bombsight
-- Added IncSightDist
-- Added DecSightDist
-- Added AdjSightRight
-- Added AdjSightLeft
-- Added IncSightAlt
-- Added DecSightAlt
-- Added IncSightVelocity
-- Added DecSightVelocity
-- Added ToggleHook for the arresting hook
-- Added Query, Set and Get _Airbrakes methods
-- Added Query, Set and Get _Tailwheel methods
-- Added Query, Set and Get _LvlStab methods
-- Added a Set_CowlFlaps method for incrementing the Cowl flap position.
-- Added ManualGearDown and ManualGearUp methods

v2.1.2 
-- Corrected bug in the retrieval of engine data in certain scenarios. works consistently now.
-- removed set_engine_one_data private function. Since 4.0.4 of FB you can now query for all the engine 
data for any engine in one single UDP query.
-- adding functions for dealing with Weapons.

v2.1.1 
-- Changed ReadMsg so as to add a 50ms timeout on waiting for the blocking of the recv socket call.
This means any application using this lib can not be started prior to the server without worrying
about a blocking call.
-- refactored queryint, queryfloat, to use an overloaded getval method. This eliminated the need
for a getintval and getfloatval method and used overloading to perform the same function. Also, eliminated
a buffer creation and initialization in both those routines that wasn't needed. Sped up those routines
by at least 10% each.
--added methods for querying, getting and setting flaps.
--refactored QueryMsg to eliminate a no longer needed buffer allocation.
--because of added combined commands and some bug fixes to Devicelink itself I've increased the
read buffer from 80 to 256.
-- changed the Visual Studio 2003 solution file to correctly build the Release version. Dumb typo on my part.

v2.1 - Incremented to 2.1 to represent the additional functionality added to the library.
- Added a SuperChgNxt and SuperChgPrv methods for increasing and decreasing the supercharger.
- Added SelectEng3 and SelectEng4 methods for being able to Select engines 3 and 4
- Added methods for turning Toggling WEP and a method for checking it's state (Get_WEP)
- corrected the assumption all float returned values would be greater than zero.
- Added the following instrument functions:
		float Get_IAS(void);
		float Get_Alt(void);
		float Get_AngSpd(void);
		float Get_Azimuth(void);
		float Get_BeaconAzimuth(void);
		float Get_Pitch(void);
		float Get_Roll(void);
		float Get_Slip(void);
		float Get_Turn(void);
		float Get_Vario(void);
		float Get_Fuel(void);
		bool Set_IAS(void);
		bool Set_Alt(void);
		void Set_AngSpd(void);
		bool Set_Azimuth(void);
		bool Set_BeaconAzimuth(void);
		bool Set_Pitch(void);
		bool Set_Roll(void);
		bool Set_Slip(void);
		bool Set_Turn(void);
		void Set_Vario(void);
		bool Set_Fuel(void);

and associated private vars.

- Added the following controls methods:
		float Get_Aileron(void);
		bool Query_Aileron(void);
		bool Set_Aileron(float pos);
		float Get_Elevator(void);
		bool Query_Elevator(void);
		bool Set_Elevator(float pos);
		float Get_Rudder(void);
		bool Query_Rudder(void);
		bool Set_Rudder(float pos);
		float Get_Power(const int eng_idx);
		bool Query_Power(const int eng_idx);
		bool Set_Power(const int eng_idx,float pos);
		float Get_PropPitch(const int eng_idx);
		bool Query_PropPitch(const int eng_idx);
		bool Set_PropPitch(const int eng_idx,float pos);
		float Get_Brakes(void);
		bool Query_Brakes(void);
		bool Set_Brakes(float pos);
		float Get_AilTrim(void);
		bool Query_AilTrim(void);
		bool Set_AilTrim(float pos);
		float Get_ElvTrim(void);
		bool Query_ElvTrim(void);
		bool Set_ElvTrim(float pos);
		float Get_RudTrim(void);
		bool Query_RudTrim(void);
		bool Set_RudTrim(float pos);
		
- Added a 'power' private var to the m_engine_type structure.
- Added a 'prop_pitch' private var to the m_engine_type structure.

There will be more changes, mostly additional functions coming soon but wanted to get this one out as I did fix a few bugs too.

v2.0.7 -
- Refactored the various Toggle type methods to better utilize the private toggleswitch() method.
- Corrected error in GetMags where I forgot to add the passed in reference engine number. Would always return the result of engine number 1.
- Refactored code to eliminate need for private variable of m_code_len. This could be determined on the fly and didn't need to be persistently stored. Reduced the number of locks needed as well.
- GetNumEngines-removed call to IsInitialized as it was not needed.
- added a setengfloats and getengfloats private methods which allowed me to refactor a lot of excess code 
out of the various engine commands. Simplified it quite a bit.
- changed getparamval again to better handle multi-engine aircraft responses.
- add m_ret_cnt as a private variable to allow getparamval the ability to parse the appropriate number of 
delimiters when looking for a return value. For example, some queries return a single value to a query,
but the engine functions return the engine referenced AND the associated value.

v2.0.6 - Had to correct the various engine related calls to account for change in the engine index values
-Updated comments to better produce documentation of the API
-Added Get_Mags function that returns the number magnetos in the aircraft


v2.05 - 
-Fixed devicelink.h include in first line. 
-Reduced buffers allocating 1024 bytes to only allocate 80 bytes.
-Increased command buffer in SendMsg() from 32 bytes to 64 bytes to better match the message length expected by Devicelink.
- in Set_Engine_Data added call to special function set_engine_one_data to handle the specific instance of Engine index = 0 (meaning the first or primary engine). The set_engine_one_data takes advantage of the devicelink ability to retrieve all the engine data for engine index 0 in a single query to the server.
- replaced hard coded values for engine indexes with proper #define macros.  This fixes a bug in the engine data routines.
-changed m_engine[x].rpm from an integer to a float.
- changed the get and set functions for rpm to handle floats instead of ints. The server returns a float for the value and not an int.
-in Set_Manifold a cut and paste error had a failure condition resetting the temp_water variable to zero instead of the manifold variable.
-Restructed code layout to match my source code control system and added VS2003 and VS2005 
solution files for building the devicelink_lib.lib.
-Added #define for the Request and Answer indicators (R and A) and made macro #defines for the delimiters
-replaced hard coded delimiter strings with new macro delimiters.
-completely rewrote getparamval. Now more efficient and robust and eliminates troublesome strtok calls. Also, less compiler specific #ifdefs.
-ReadMSg now no longers rebuilds the socket interface.  This was done due to a previous bug in devicelink that has been fixed in the latest builds.  this also makes ReadMsg function about 20% faster.
-changed some minimum byte sizes to 64 vs 32.
-In GetAircraftID changed unneeded byte array copy to a simpler and safer strncpy.
-in get_read_buff changed one comparison for NULL to correct byte array and replaced improper sizeof call
with strlen call.




v2.0.4 - Fixed a bug in GetDLVersion. Added #if to support compiling with safer
string libraries in Visual Studio 2005.
v2.0.3 - Changed stncpy in the get_x_buff functions to also use passed in size param.
v2.0.2: Made changes to the following:
set_read_buff, get read_buff, get_cmd_buff to add in a second parameter for the passed in buffer size.
This was done to deal with proper checking of buffer constraints for strings where the previous
methods didn't work across compiler and OS differences. In the declaration of these functions I made
the parameter default to 32 to accomodate anyone having used these functions and maintaining backward
compatability.

v2.0.1: Made a couple of minor changes to fix a couple of dumbass errors I made.

V2.0:

Most of the functions are commented.  I didn't include all the possible devicelink commands 
in the headers as I tend to create them as I need but there is enough there to use as an 
example if you add more.  I did include functions to retrieve engine data and cleaned up the 
code some more.  Mainly added a lot more error trapping and debugging messages as well as
eliminating several potential buffer overflow vulnerabilities.  The natural question is why
didn't I use STL for strings if I wanted to stay away from buffer overflows? Performance was 
the main reason and that Microsoft's STL (based on Dinkumware) is out of date and I didn't 
want to hassle with updating it and/or relying on other programmers to update thei copies.

Most of the issues could be addressed by simply checking buffers and limiting writes to them.

Other big change was I added a CritSec class and a thread lock class to make the library
thread-safe.  I launch no threads of my own (and hope to keep it that way) but this should
make the primitives and private vars thread safe. I made the Lock destructor take care of
executing the thread unlock. Should keep any exceptions you throw from blocking and causing
a race condition.  I find that even if you are careful to match Enter and Leave on 
thread locking you can still get hung on the odd exception.  If that bubbles out to 
the OS you might even get a hard lock. A Bad Thing--"Be sure". ;-)

I cleaned up the Debug Output as well.  By default it provides no output to either the stderr
or a debug file. However, if you define DEBUG_OUTPUT then it will default to writing
debug messages to std_err.  If you call Init() with a filename instead of nothing then it will
output the debug to the specified file.

As time goes on I will be updating this from time to time. Mostly adding new functions 
and will strive to make it backwards compatible.

This new version should be compatible with previous versions. I added to the API but did not
eliminate any old methods (except ones that weren't used.)


Now, for the legal stuff.

Use at your own risk. You break something don't blame me.  I've provided the raw code so you 
can examine it yourself.  You are allowed to re-use this code in any product private or 
commercial I only ask that I receive credit if you do.  I would also appreciate any 
feedback on bugs you find.

WWSensei

wwsensei@wingwalkers.org