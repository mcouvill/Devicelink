/*! \file devicelink.h
\brief Main header file with the defines for the various devicelink codes
\author Mike Couvillion
\version 2.1.4
\date Copyright 2004-2007
\note This file predefines the various codes that are used to build queries
	for sending to the IL2 game engine.
\note This code is free to use for free utilities. It is not free for commercial use.  
Not responsible for any errors. Use at your own risk. If you do use the code I ask you
give proper credit.
*/
#pragma once

#ifndef _MSC_VER
#define _MSC_VER 1310 //!<for non-Win32 OS's to make my other #if's work correctly.
#endif

#include <stdio.h>
#include <winsock2.h>
#include <stdlib.h>
#include <string.h>
#include "mc_lock.h"

#define DL_GET_VERSION   "2"   //!< When this code is sent the game returns the version of DeviceLink that is running.
#define DL_ACCESS_GET  	"4"  
#define DL_ACCESS_SET   "6" 
#define DL_TOD   "20"  //!< queries for Time of Day. Returns float (hours)
#define DL_GET_PLANE   "22"  //!< returns id of aircraft
#define DL_GET_COCKPITS   "24"  //!< returns int with number of planes
#define DL_GET_CUR_COCKPIT   "26"  //!< returns int, current cockpit
#define DL_GET_ENGINES   "28"  //!< returns int
#define DL_GET_IAS   "30"  //!< returns float
#define DL_GET_VARIO   "32"  //!< returns float
#define DL_GET_SLIP   "34"  //!< returns float in degrees
#define DL_GET_TURN   "36"  //!< returns float
#define DL_GET_ANG_SPD   "38"  //!< retuns float in deg/s
#define DL_GET_ALT   "40"  //!< returns float (meters)
#define DL_GET_AZI   "42"  //!< returns float
#define DL_GET_BEACON_AZI   "44"  //!< retuns float (degrees)
#define DL_GET_ROLL   "46"  //!< returns float
#define DL_GET_PITCH   "48"  //!< returns float
#define DL_GET_FUEL   "50"  //!< returns fuel in kilograms
#define DL_GET_OVERLOAD   "52"  //!< returns fuel
#define DL_GET_SHAKE   "54"  //!< returns float
#define DL_GET_MAG   "62"  //!< pass in the engine idx and get int range 0-3
#define DL_GET_RPM   "64"  //!< pass in the engine idx and get float
#define DL_GET_MANIFOLD   "66"  //!< pass in int of engine idx. returns float
#define DL_GET_TEMP_OILIN   "68"  //!< pass in int of engine idx. returns float
#define DL_GET_TEMP_OILOUT   "70"  //!< pass in int of engine idx. returns float
#define DL_GET_TEMP_WATER   "72"  //!< pass in int of engine idx. returns float
#define DL_GET_TEMP_CYL   "74"  //!< pass in int of engine idx. returns float
#define ENGINE_ONE   0 
#define ENGINE_TWO   1 
#define ENGINE_THREE   2 
#define ENGINE_FOUR	  3 
#define DL_ALL_INST "30/32/34/36/38/40/42/44/46/48/50"

// Command codes for landing gear

#define DL_GET_LEFT_GEAR_POS   "56"  //!< returns float
#define DL_GET_RIGHT_GEAR_POS   "58"  //!< returns float
#define DL_GET_CENTER_GEAR_POS   "60"  //!< returns float
#define DL_GET_GEAR_STATUS   "164"  //!< return float
#define DL_SET_GEAR_STATUS   "165"  //!< sends command to set the gear status
#define DL_RAISE_GEAR_MAN   "167"  //!< issue ONE command to raise gear manually
#define DL_LOWER_GEAR_MAN   "169"  //!< issue ONE command to lower gear manually
#define DL_GET_POWER   "80"  //!<  return float
#define DL_SET_POWER   "81"  //!<  set(float)
#define DL_GET_FLAPS_POS   "82"  //!<  returns float
#define DL_SET_FLAPS_POS   "83"  //!<  send float ranged -1.00 to 1.00
#define DL_GET_AILERON   "84"  //!<  returns float
#define DL_SET_AILERON   "85"  //!<  set float
#define DL_GET_ELV   "86"  //!< returns float
#define DL_SET_ELV   "87"  //!< set(float)
#define DL_GET_RUDDER   "88"  //!< returns float
#define DL_SET_RUDDER   "89"  //!< set(float)
#define DL_GET_BRAKES   "90"  //!< returns float
#define DL_SET_BRAKES   "91"  //!< set(float)
#define DL_GET_PROP_PITCH   "92"  //!< returns float
#define DL_SET_PROP_PITCH   "93"  //!< set(float)
#define DL_GET_AIL_TRIM   "94"  //!< returns float
#define DL_SET_AIL_TRIM   "95"  //!< set(float)
#define DL_GET_ELV_TRIM   "96"  //!< returns float
#define DL_SET_ELV_TRIM   "97"  //!< set(float)
#define DL_GET_RUDDER_TRIM   "98"  //!< returns float
#define DL_SET_RUDDER_TRIM   "99"  //!< set(float)

//Other Toggles

#define DL_GET_LVL_STAB   "100"  //!< return 1 or 0
#define DL_TOGGLE_STAB   "101" 
#define DL_TOGGLE_GUNSIGHT   "399" 
#define DL_TOGGLE_AUTOPILOT   "401" 
#define DL_TOGGLE_LVL_AUTOPILOT   "403" 
#define DL_TOGGLE_MAP   "417" 
#define DL_TOGGLE_CHAT   "419" 
#define DL_TOGGLE_ONLINE_RATE   "421"  //!< set(int) 1=show, 0=hide
#define DL_TOGGLE_SPEED_BAR   "423" 
#define DL_TOGGLE_ICONS   "425" 
#define DL_TOGGLE_MIRRORS   "427" 
#define DL_TOGGLE_SAVE_TRACK   "429" 
#define DL_TOGGLE_ACCEL_TIME   "435" 
#define DL_TOGGLE_NORMAL_TIME   "437" 
#define DL_TOGGLE_DEC_TIME   "439" 
#define DL_TOGGLE_PAUSE   "441" 

//Engine Controls

#define DL_GET_CHRG_NXT   "110"  //!< returns int
#define DL_SET_CHRG_NXT   "111" 
#define DL_GET_CHRG_PREV   "112"  //!< returns int
#define DL_SET_CHRG_PREV   "113" 
#define DL_SET_FIRE_EXT   "161" 
#define DL_SET_COWL_FLAPS   "171" 

///Toggles for Engines

#define DL_TOGGLE_ENGINE   "103" 
#define DL_GET_WEP   "104"  //!< return 1 or 0
#define DL_TOGGLE_WEP   "105" 
#define DL_TOGGLE_ENGINES_ALL   "139" 
#define DL_TOGGLE_ENGINES_LEFT   "141" 
#define DL_TOGGLE_ENGINES_RIGHT   "143" 
#define DL_TOGGLE_ENG1_SELECT   "145" 
#define DL_TOGGLE_ENG2_SELECT   "147" 
#define DL_TOGGLE_ENG3_SELECT   "149" 
#define DL_TOGGLE_ENG4_SELECT   "151" 
#define DL_TOGGLE_ENG5_SELECT   "153" 
#define DL_TOGGLE_ENG6_SELECT   "155" 
#define DL_TOGGLE_ENG7_SELECT   "157" 
#define DL_TOGGLE_ENG8_SELECT   "159" 
#define DL_GET_FEATHER   "162"  //!< return 1 or 0
#define DL_TOGGLE_FEATHER   "163" 

///Select Engines

#define DL_SELECT_ENG_ALL   "115" 
#define DL_UNSELECT_ENG_ALL   "117" 
#define DL_SELECT_ENG_LEFT   "119" 
#define DL_SELECT_ENG_RIGHT   "121" 
#define DL_SELECT_ENG_1   "123" 
#define DL_SELECT_ENG_2   "125" 
#define DL_SELECT_ENG_3   "127" 
#define DL_SELECT_ENG_4   "129" 
#define DL_TOGGLE_ENGINE1   "123/103" 
#define DL_TOGGLE_ENGINE2   "125/103" 
#define DL_TOGGLE_ENGINE3   "127/103" 
#define DL_TOGGLE_ENGINE4   "129/103" 
#define DL_TOGGLE_ENGINE5   "131/103" 
#define DL_TOGGLE_ENGINE6   "133/103" 
#define DL_TOGGLE_ENGINE7   "135/103" 
#define DL_TOGGLE_ENGINE8   "137/103" 

///Lights

#define DL_TOGGLE_COCKPIT_LTS   "409" 
#define DL_TOGGLE_NAV_LTS   "411" 
#define DL_TOGGLE_LND_LTS   "413" 
#define DL_TOGGLE_SMOKE   "415" 

///General Aircaft

#define DL_GET_AIRBRK   "172"  //!< return int
#define DL_TOGGLE_AIRBRK   "173"  
#define DL_GET_TAILWHEEL   "174"  //!< return int
#define DL_TOGGLE_TAILWHEEL   "175"  
#define DL_SET_DROP_TANKS   "177" 
#define DL_TOGGLE_ATTACH_PLANE   "179" 
#define DL_BAILOUT   "405" 
#define DL_TINT_RETICLE   "407" 
#define DL_GET_WING_FOLD   "210"  //!< int: 1 or 0
#define DL_TOGGLE_WING_FOLD   "211" 
#define DL_GET_CANOPY   "212"  //!< int 1 or 0
#define DL_TOGGLE_CANOPY   "213" 
#define DL_GET_HOOK   "214"  //!< int 1 or 0
#define DL_TOGGLE_HOOK   "215" 
#define DL_GET_CHOCKS   "216"  //!< int 1 or 0
#define DL_TOGGLE_CHOCKS   "217" 


///Bombsight

#define DL_TOGGLE_SIGHT_AUTO   "193" 
#define DL_INC_SIGHT   "195" 
#define DL_DEC_SIGHT   "197" 
#define DL_RIGHT_SIGHT   "199" 
#define DL_LEFT_SIGHT   "201" 
#define DL_INC_SIGHT_ALT   "203" 
#define DL_DEC_SIGHT_ALT   "205" 
#define DL_INC_SIGHT_VEL   "207" 
#define DL_DEC_SIGHT_VEL   "209" 

///Gunner controls

#define DL_GET_GUNNER   "220"  //!< return int (0-1)
#define DL_SET_GUNNER   "221"  //!< set(int) (0-1) 1=start, 0=stop
#define DL_SET_GUNNER_MOVE   "223"  //!< set(int,int,int)

///Camera

#define DL_GET_GUNNER_POS   "300"  //!< return int idx of gunner slot
#define DL_SET_GUNNER_POS   "301" 
#define DL_JUMP_TO_PILOT   "303" 
#define DL_JUMP_TO_POS2   "305" 
#define DL_JUMP_TO_POS3   "307" 
#define DL_JUMP_TO_POS4   "309" 
#define DL_JUMP_TO_POS5   "311" 
#define DL_JUMP_TO_POS6   "313" 
#define DL_JUMP_TO_POS7   "315" 
#define DL_JUMP_TO_POS8   "317" 
#define DL_JUMP_TO_POS9   "319" 
#define DL_JUMP_TO_POS10   "321" 
#define DL_CHASE_VIEW   "393" 
#define DL_CHASE_VIEW_FRIEND   "395" 
#define DL_CHASE_VIEW_ENEMY   "397" 

///FOV

#define DL_SET_FOV_90   "323" 
#define DL_SET_FOV_85   "325" 
#define DL_SET_FOV_80   "327" 
#define DL_SET_FOV_75   "329" 
#define DL_SET_FOV_70   "331" 
#define DL_SET_FOV_65   "333" 
#define DL_SET_FOV_60   "335" 
#define DL_SET_FOV_55   "337" 
#define DL_SET_FOV_50   "339" 
#define DL_SET_FOV_45   "341" 
#define DL_SET_FOV_40   "343" 
#define DL_SET_FOV_35   "345" 
#define DL_SET_FOV_30   "347" 
#define DL_GET_FOV   "348" //!< return float, current FOV in deg
#define DL_TOGGLE_FOV   "349" 
#define DL_INC_FOV   "351" 
#define DL_DEC_FOV   "353" 

//O/ther Views

#define DL_COCKPIT_VIEW   "355" 
#define DL_NO_COCKPIT_VIEW   "357" 
#define DL_EXT_VIEW   "359" 
#define DL_NEXT_FRIEND   "361" 
#define DL_NEXT_ENEMY   "363" 
#define DL_FLYBY   "365" 

///Padlock

#define DL_PAD_ENEMY   "367"  //!< padlock enemy
#define DL_PAD_FRIEND   "369"  //!< padlock friend
#define DL_PAD_ENEMY_GRND   "371"  //!< padlock enemy ground
#define DL_PAD_FRIEND_GRND   "373"  //!< padlock friendly ground
#define DL_PAD_NEXT   "375"  //!< padlock next
#define DL_PAD_PREV   "377"  //!< padlock previous
#define DL_PAD_VIEW_FRONT   "479"  //!< padlock snap view to front

///External Padlocks

#define DL_PAD_EXT_ENEMY   "381" 
#define DL_PAD_EXT_FRIEND   "383" 
#define DL_PAD_EXT_CLOSE_ENEMY   "385" 
#define DL_PAD_EXT_ENEMY_GRND   "387" 			
#define DL_PAD_EXT_FRIEND_GRND   "389" 
#define DL_PAD_EXT_CLOSE_ENEMY_GRND   "391" 

///Weapons

#define DL_GET_WEAP1   "180"  //!< return int (0-1)
#define DL_SET_WEAP1   "181"  //!< set(int) (0-1) 1=start, 0=stop
#define DL_GET_WEAP2   "182"  //!< return int (0-1)
#define DL_SET_WEAP2   "183"  //!< set(int) (0-1)1=start, 0=stop
#define DL_GET_WEAP3   "184"  //!< return int (0-1)
#define DL_SET_WEAP3   "185"  //!< set(int) (0-1) 1=start, 0=stop
#define DL_GET_WEAP4   "186"  //!< return int (0-1)
#define DL_SET_WEAP4   "187"  //!< set(int) (0-1) 1=start, 0=stop
#define DL_GET_WEAP1_2   "188"  //!< return int (0-1)
#define DL_SET_WEAP1_2   "189"  //!< set(int) (0-1) 1=start, 0=stop
#define DL_GET_GUNPOD   "190"  //!< return int (0-1)
#define DL_SET_GUNPOD   "191"  //!< set(int) (0-1) 1=start, 0=stop
#define DL_START_GUNNER   "221/1"  //!< requires a paramater. 1 = start, 0 = stop
#define DL_STOP_GUNNER   "221/0"  //!< requires a paramater. 1 = start, 0 = stop

#define DELIM_1 '/'
#define DELIM_2 '\\'
#define REQUEST 'R'
#define ANSWER	'A'
enum Speed {KMH, KTS, MPH};
enum WeapType {MG, CANNON, ROCKETS, BOMBS, MGCANNON};


/// Static vars for socket code

static fd_set    ActiveReadFds;

/*!	\brief The C++ wrapper class for devicelink

	This class attempts to wrap the obscure devicelink codes into a usable set of APIs
	for writing applications communicating with IL2 Sturmovik via its UDP protocol.  The design
	decision in this class was to encapsulate the communications as a single object for use by higher
	classes.  Query_ methods will send requests to the game for requested values and set a private variable.
	Set_ methods will send commands	to the game, and Get_ methods will return the private variable.
*/
class C_DeviceLink
{
	public:
//Initialization functions
		C_DeviceLink();
		~C_DeviceLink();
		bool Init();
		bool Init(FILE *dl_output);
		bool ReadConfig(void);
//Messaging methods
		bool SendMsg(void);
		bool ReadMsg(void);
		bool QueryMsg(const char* code);
//Lights and Smoke Toggles
		bool ToggleSmoke(void);
		bool ToggleLandLights(void);
		bool ToggleCockpitLights(void);
		bool ToggleNavLights(void);
//Misc aircraft
		bool GetAircraftID(char* ac, unsigned int buff_size = 64);
		float GetTOD(void);
		bool ToggleHook(void);
		bool Query_LvlStab(void);
		int Get_LvlStab(void);
		bool Set_LvlStab(void);
		float Get_Overload(void);
		float Get_ShakeLvl(void);

//Cockpit/Gunner station functions
		bool Query_Canopy(void);
		bool Set_Canopy(void);
		int Get_Canopy(void);
		int GetNumOfCockpits(void);
		int GetCurCockpit(void);
		bool Query_Gunner(void);
		bool Set_Gunner(const char* code);
		int Get_Gunner(void);
		bool JumpToCockpit(const int pitnum);
//Landing Gear methods
		bool Gear_Is_Up(void);
		float GetGearPos(char* gearcode);
		bool ToggleGear(void);		
		bool SetGearDown(void);
		bool SetGearUp(void);
		bool ManualGearUp(void);
		bool ManualGearDown(void);
		bool Query_Chocks(void);
		int Get_Chocks(void);
		bool Set_Chocks(void);
//Misc utility functions
		bool HasData(void);
		bool IsInitialized(void);
		bool GetDLVersion(char* verstr, unsigned int buff_size = 64);
		bool ValidGet(const char* code);
//Engine Methods
		bool Set_Engine_Data(const int eng_num);
		bool Set_RPM(const int eng_num);
		bool Set_Temp_Cyl(const int eng_num);
		bool Set_Temp_Oilin(const int eng_num);
		bool Set_Temp_Oilout(const int eng_num);
		bool Set_Temp_Water(const int eng_num);
		bool Set_Manifold(const int eng_num);
		float Get_RPM(const int eng_num);
		float Get_Temp_Cyl(const int eng_num);
		float Get_Temp_Oilin(const int eng_num);
		float Get_Temp_Oilout(const int eng_num);
		float Get_Temp_Water(const int eng_num);
		float Get_Manifold(const int eng_num);
		int Get_WEP(void);
		bool SuperChgNxt(const int eng_num);
		bool SuperChgPrv(const int eng_num);
		bool ToggleWEP(void);
		bool SelectAllEngines(void);
		bool UnSelectAllEngines(void);
		bool SelectLeftEngines(void);
		bool SelectRightEngines(void);
		bool ToggleSelAllEngines(void);
		bool ToggleSelLeftEngines(void);
		bool ToggleSelRightEngines(void);
		bool ToggleSelEng1(void);
		bool ToggleSelEng2(void);
		bool ToggleSelEng3(void);
		bool ToggleSelEng4(void);
		bool SelectEng1(void);
		bool SelectEng2(void);
		bool SelectEng3(void);
		bool SelectEng4(void);
		bool ToggleEng1Select(void);
		bool ToggleEng2Select(void);
		bool FeatherEngine(void);
		bool StartEng1(void);
		bool StartEng2(void);
		bool StartEng3(void);
		bool StartEng4(void);
		int GetMags(const int eng_num);
		int GetNumEngines(void);		
// Instrument methods	
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
		bool SetAllInstruments(void);
// Controls Methods
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
		float Get_Flaps(void);
		bool Query_Flaps(void);
		bool Set_Flaps(float pos);
		bool Query_Airbrakes(void);
		int Get_Airbrakes(void);
		bool Set_Airbrakes(void);
		bool Query_Tailwheel(void);
		int Get_Tailwheel(void);
		bool Set_Tailwheel(void);
		bool Set_CowlFlaps(void);
		bool Query_WingFold(void);
		int Get_WingFold(void);
		bool Set_WingFold(void);
		bool Query_TailHook(void);
		int Get_TailHook(void);
		bool Set_TailHook(void);

// Weapons Methods
		int Get_Weapon(WeapType weap);
		bool Query_Weapon(WeapType weap);
		bool Set_Weapon(WeapType weap, int ival);
		bool Query_GunPods(void);
		int GetGunPodsState(void);
		bool ToggleGunPods(void);
		bool ToggleBombSight(void);
		bool IncSightDist(void);
		bool DecSightDist(void);
		bool AdjSightRight(void);
		bool AdjSightLeft(void);
		bool IncSightAlt(void);
		bool DecSightAlt(void);
		bool IncSightVelocity(void);
		bool DecSightVelocity(void);


	private:
		/// struct for containing the various engine data.
		struct m_engine_type
		{
			float rpm; //!< stores an engine's RPM value
			float manifold; //!< stores an engine's manifold pressure
			float temp_oilin; //!< stores an engine's Oil In temperature
			float temp_oilout; //!< stores an engine's Oil Out tempature
			float temp_water; //!< stores an engine's water tempature
			float temp_cylinders; //!< stores an engine's cylinders temperature
			float power; //!< stores the engine's power setting
			float prop_pitch; //!< stores the prop pitch of the engine
		};
		int m_weap[5]; //!< array for storing the state of the weapons
		float m_ias; //!< priv var for indicated airspeed 0--infinite
		float m_vario; //!< stores the current value of the variometer
		float m_azimuth; //!< stores the heading 0.00-359.99
		float m_beacon_azimuth; //!< stores the beacon heading 0.00-359.99 range
		float m_pitch; //!< stores the current pitch -90.00 - 90.00 range
		float m_roll; //!< stores the roll -180.00 - 180.00 range
		float m_slip; //!< stores the slip value -45.00 - 45.00 range
		float m_turn; //!< stores the current turn value -1.00 - 1.00 range
		float m_alt; //!< stores the current altitude 0.00 to infinity range
		float m_ang_spd; //!< stores the angular speed. -inf to +inf range
		float m_fuel; //!< stores the current fuel in kg. 0 to infinity range
		float m_aileron; //!< stores the aileron value. -1.00 to 1.00 range
		float m_elevator; //!< stores the elevator value. -1.00 to 1.00 range
		float m_rudder; //!< stores the rudder setting. -1.00 to 1.00 range
		float m_brakes; //!< stores the brake setting. -1.00 to 1.00 range
		float m_ail_trim; //!< stores the aileron trim setting. -1.00 to 1.00 range
		float m_elv_trim; //!< stores the elevator trim setting. -1.00 to 1.00 range
		float m_rudder_trim; //!< stores the rudder trim setting. -1.00 to 1.00 range
		float m_flaps; //!< stores the flaps setting. -1.00 to 1.00 range
		int m_gunpod; //!< stores the state of the gunpods firings. 0 or 1
		int m_airbrakes; //!< stores the state of the air brake deployment. 0 or 1
		int m_tailwheel; //!< stores the state of the tail wheel lock. 0 or 1
		int m_lvlstab; //!< stores whether the level stabilizer is engaged. 0 or 1.
		int m_wingfold; //!< stores whether the wings are folded or not. 0 or 1
		int m_canopy; //!< stores whether the canopy is open or closed. 0 or 1.
		int m_tailhook; //!< stores whether the tail hook is deployed or not. 0 or 1.
		int m_chocks; //!< stores whether the chocks are in or out. 0 or 1
		int m_gunner; //!< stores whether the gunner is firing or not. 0 or 1.
		char m_dl_ver[32]; //!< private variable for holding the devicelink version number
		bool m_readdata; //!< flag to indicate whether any data was actually read from the buffer
		FILE *dl_output; //!< filename for the debug file output.
		bool m_initialized; //!< flag to indicate whether the devicelink object has been initialized
		char m_game_ip[16]; //!< buffer for storing the IP of the game server
		u_short m_port; //!< UDP port number where the game is listening
		struct hostent *hostinfo;
		struct sockaddr_in m_this_end; //!< struct for socket ops
		struct sockaddr_in m_other_end; //!< struct for socket ops
		SOCKET m_sock; //!< stores the socket number
		char m_cmd[64]; //!< buffer for sotring a command string to be sent to the game
		char m_buff[256]; //!< buffer for storing what we read from the UDP socket.
		unsigned int m_ret_cnt; //!< a count of the expected number of returned values from a function call.
		
		bool setengfloats(const int eng_num, const char* code, float *engine_part);
		float getengfloats(const int eng_num, float *engine_part);
		bool getparamval(const char* code, char* strval, unsigned int buff_size = 64);
		bool querystring(const char* code, char* qstr, unsigned int buff_size = 64);
		float queryfloat(const char* code);
		void getval(const char* code, float* val);
		void getval(const char* code, int* val);
		int queryint(const char* code);
		bool set_command_buff(const char* code);
		bool get_cmd_buff(char* temp_buff, unsigned int buff_size = 64);
		bool toggleswitch(const char* code);
		struct m_engine_type m_engine[4];
		void init_err(void);
		void errmsg(const char* str);
		bool set_has_read_data(bool flag);
		bool set_read_buff(const char* temp_buff, unsigned int buff_size = 64);
		bool get_read_buff(char* temp_buff, unsigned int buff_size = 64);
		bool starteng(const char* seleng, const char* togeng);
		bool setctrl(const char* code, float pos);
				
		void dl_strncpy(char * dest_str, char * src_str, unsigned int dest_size);//!< modified copy command to distinguish between VS2003 and VS2005 buffer handling.
};

