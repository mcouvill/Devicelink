/*! \file devicelink.cpp
	\brief The main source file for the devicelink functions. v2.1.4.1
	\author Mike Couvillion
	\date 2004-2007 Copyright.
*/

#include "devicelink.h"

/*!
/note	This variable will be used for providing a thread locking mechanism and is
used throughout the class, hence it being global to the class.
*/
MC_CritSection my_critsec;

/*! \brief Constructor. Mainly initializes the private member variables.

*/
C_DeviceLink::C_DeviceLink(void)
: m_gunpod(0)
,m_port(0)
,m_initialized(FALSE)
,m_readdata(FALSE)
,m_sock(0)
,m_ret_cnt(1)
,dl_output(NULL)
,m_ias(0.00)
,m_vario(0.00)
,m_azimuth(0.00)
,m_beacon_azimuth(0.00)
,m_pitch(0.00)
,m_roll(0.00)
,m_slip(0.00)
,m_turn(0.00)
,m_alt(0.00)
,m_ang_spd(0.00)
,m_aileron(0.00)
,m_flaps(0.00)
,m_airbrakes(0)
,m_lvlstab(0)
,m_wingfold(0)
,m_canopy(0)
,m_tailhook(0)
,m_chocks(0)
,m_gunner(0)
{
	memset(m_cmd, NULL, sizeof(m_cmd));
	memset(m_buff, NULL, sizeof(m_buff));
	dl_strncpy(m_game_ip,"0.0.0.0",sizeof(m_game_ip));
	memset(m_dl_ver, NULL, sizeof(m_dl_ver));
	for (int i = 0; i < 4; ++i)
	{
		m_engine[i].rpm = 0.00;
		m_engine[i].manifold = 0.00;
		m_engine[i].temp_oilin = 0.00;
		m_engine[i].temp_oilout = 0.00;
		m_engine[i].temp_water = 0.00;
		m_engine[i].temp_cylinders = 0.00;
		m_engine[i].power = 0.00;
		m_engine[i].prop_pitch = 0.00;
	}
	for (int j = MG; j < MGCANNON; ++j)
	{
		m_weap[j] = 0;
	}
}

/*! \brief Deconstructor
*/
C_DeviceLink::~C_DeviceLink()
{

}
/**************************/
/* Private Method Section */
/**************************/

/*! \brief This function allows a developer to set the command buffer
\param code : a const char defined devicelink code for the devicelink function you will use
\return \b boolean

\note This method takes in a string which is checked for NULL or if it is 
larger than the private member variable it will bail.
Ultimately, it will assign the private m_cmd variable with a 
string to be sent to the game. It doesn't actually send the code
just preps the buffer to be sent.

*/
bool C_DeviceLink::set_command_buff(const char * code)
{
	if (code == NULL)
	{
		errmsg("Invalid code passed to set_command_buff.\n");
		return FALSE;
	}
	if (strlen(code) > sizeof(m_cmd))
	{
		errmsg("temp buffer too large for m_cmd in set_command_buff.\n");
		return FALSE;
	}
	MC_Lock m_Lock(&my_critsec);
#if _MSC_VER >= 1400
	_snprintf_s(m_cmd,sizeof(m_cmd),_TRUNCATE,"%c%c%s",REQUEST,DELIM_1,code);
#else
	_snprintf(m_cmd,sizeof(m_cmd),"%c%c%s",REQUEST,DELIM_1,code);
#endif
	return TRUE;
}

/*! \brief This private routine does an actual query to the devicelink server for a float value.
\param code : a const char defined devicelink code for the float value you will query for
\return \b float
\note It takes the devicelink code as a param, queries the game and converts the resulting code
into a true float value.

*/
float C_DeviceLink::queryfloat(const char *code)
{
	float fval = 0.00;
	if (QueryMsg(code) == TRUE)
	{
		getval(code, &fval);
	} else
	{
		errmsg("QueryMsg returned FALSE in queryfloat\n");
		if (HasData() == FALSE)
		{
			errmsg("queryfloat: Query failed to read data from socket.\n");
		}
		fval = 0.00;
	}
	return fval;
}

/*! \brief Gets a string representation of the return value from the queried code and places it in the passed qstr.
\param code : a const char defined devicelink code you are querying the server for
\param qstr : the buffer you will store the server return value
\param buff_size : the size of qstr you allocated
\return \b boolean
\sa QueryMsg()
\sa getparamval()
\sa HasData()
\warning Calling function is responsible for allocating memory for qstr.

*/
bool C_DeviceLink::querystring(const char* code, char* qstr, unsigned int buff_size)
{
	if (QueryMsg(code) == TRUE)
	{
		if (getparamval(code, qstr, buff_size) == TRUE)
		{
			return TRUE;
		}
	} else
	{
		errmsg("QueryMsg returned FALSE in querystring\n");
		if (HasData() == FALSE)
		{
			errmsg("Query failed to read data from socket.\n");
		}
		return FALSE;
	}
	
	return FALSE;
}

/*! \brief This private routine that returns the value of the paramter returned in an A coded response from the server.
\param code : The devicelink code you will parse the command buffer for
\param strval : The buffer you will store the resulting paramter value in
\param buff_size : the size of the strval buffer
\return : boolean
\sa get-read_buff()
\note The result is stored in the buffer pointed to by strval.
\warning strval must be allocated by the calling function!
*/
bool C_DeviceLink::getparamval(const char *code, char* strval, unsigned int buff_size)
{
	char temp_buff[256];
	memset(temp_buff, NULL, sizeof(temp_buff));
	unsigned int tbuff_size = sizeof(temp_buff);
	if (get_read_buff(temp_buff, tbuff_size) == FALSE)
	{
		errmsg("get_read_buff returned FALSE in getparamval.\n");
		return FALSE;
	}
	tbuff_size = strlen(temp_buff); //get length of temp_buff to compare to strval size.
	if (tbuff_size > buff_size)
	{
		errmsg("strval has not been allocated enought space in getparamval.\n");
		return FALSE;
	}
	
	//null out token and set pdest to point at the part of the temp_buff
	//that contains the result from a query. i.e if code 80 was passed
	//in we look for code 80 in "A/80\0.35" and then grab the 0.35 
	char* token = NULL;
	token = strstr(temp_buff, code);
	if (token == NULL)
	{
		errmsg("No matching response in buffer to query in getparamval.\n");
		return FALSE;
	}
	//Parse to the number of m_ret_cnt delimiters for the code. We don't care about it.
	//setting up a max amount to step through so we don't walk off the end of the buffer.
	unsigned int sz = strlen(token); 
	unsigned int cnt = 0;
	unsigned int tokcnt = 0;
	while ((token[0] != NULL) && (cnt <= sz) && (tokcnt < m_ret_cnt))
	{
		if (token[0] == DELIM_2)
		{
			++tokcnt;
		}
		++token;
		++cnt;
	}
	//Now get the param value.
	unsigned int j = 0;
	sz = strlen(token);
	char * ptr = NULL;
	ptr = strval;
	//copying the data to strval for conversion. Why didn't I use strcpy or some variant?
	//well, because in 1C's infinite wisdom they chose to use / and \ as delimiters. Since
	//some data can render an unescaped series of tokens with \0 that renders most of the
	//string functions useless.
	while ((token[0] != DELIM_2) & (token[0] != NULL) & (token[0] != DELIM_1) & (j <= sz))
	{
		*ptr = *token;
		++ptr;
		++token;
		++j;
	}
	
	if (strval != NULL)
	{
		if (strlen(strval) > 0)
		{
			return TRUE;
		}
	}

	return FALSE;
}

/*! \brief Executes a query expecting an int return.
\param code : a const char defined devicelink code
\return \b integer : The integer result associated with the code. -1 indicates failure.
\sa QueryMsg()
\sa getintval()
\sa HasData()
\note Error conditions are reached when either getintval fails or the socket has no data in the buffer
from the query.

*/
int C_DeviceLink::queryint(const char *code)
{
	int ivar = 0;
	if (QueryMsg(code) == TRUE)
	{
		getval(code, &ivar);
	} else
	{
		errmsg("QueryMsg returned FALSE in queryint\n");
		if (HasData() == FALSE)
		{
			errmsg("queryint: Query failed to read data from socket.\n");
		}
		ivar = 0;
	}
	return ivar;
}

/*! \brief This routine is used as a generic routine for flipping a toggle switch.
\param code : defined code for the switch you wish to toggle.
\return \b boolean

\note It assumes the calling routine passes the correct code. Also useful for the single
Set style commands that require no parameters like IncSightDist

*/
bool C_DeviceLink::toggleswitch(const char* code)
{
	if (set_command_buff(code) == FALSE)
	{
		errmsg("set_command_buff returned FALSE in toggleswitch.\n");
		return FALSE;
	}
	if (SendMsg()== TRUE)
	{
		return TRUE;
	} else
	{
		return FALSE;
	}
}

/*! \brief private function to set the passed in engine part to the queried float val
\param \b integer eng_num : index of the engine to set
\param const char* code : the defined devicelink code for the query
\param float engine_part : the engine_part to be set to the returning float value
\sa queryfloat()
*/
bool C_DeviceLink::setengfloats(const int eng_num, const char* code, float *engine_part)
{
	if ((eng_num < ENGINE_ONE) | (eng_num > ENGINE_FOUR))
	{
		errmsg("setengfloats called with invalid engine number.\n");
		return FALSE;
	}
	char temp_cmd[32];
	memset(temp_cmd,NULL,sizeof(temp_cmd));
#if _MSC_VER >= 1400
	_snprintf_s(temp_cmd,sizeof(temp_cmd),_TRUNCATE,"%s%c%d",code,DELIM_2,eng_num);
#else
	_snprintf(temp_cmd,sizeof(temp_cmd),"%s%c%d",code,DELIM_2,eng_num);
#endif
	{
	MC_Lock m_Lock(&my_critsec);
	m_ret_cnt = 2; //set expected return value delimiter count to 2
	}
	float fval = 0.00;
	fval = queryfloat(temp_cmd);
	{
	MC_Lock m_Lock(&my_critsec);
	*engine_part = fval;
	m_ret_cnt = 1; //set expected return value delimiter count to 2
	}
	return TRUE;
}

/*! \brief modified copy command to distinguish between VS2003 and VS2005 buffer handling.
\param dest_str : pre-allocated string to be have strings copied into
\param src_str : source string to be copied from
\param dest_size : size of dest_str
*/
void C_DeviceLink::dl_strncpy(char * dest_str, char * src_str, unsigned int dest_size)
{
#if _MSC_VER >= 1400
	strncpy_s(dest_str, dest_size, src_str,_TRUNCATE);
#else
	strncpy(dest_str, src_str,dest_size);	
#endif
}

/*! \brief private function for starting the passed in engine
\param seleng : the engine you are selecting
\param togeng : the engine you are toggling
*	- Example:
*		- starteng(DL_SELECT_ENG_1,DL_TOGGLE_ENGINE1);\n
*		.
\return \b boolean
\sa SendMsg()
*/
bool C_DeviceLink::starteng(const char* seleng, const char* togeng)
{
	char temp_cmd[64];
	memset(temp_cmd, NULL,sizeof(temp_cmd));
#if _MSC_VER >= 1400
	_snprintf_s(temp_cmd,sizeof(temp_cmd),_TRUNCATE,"%s/%s/%s",seleng,togeng,DL_SELECT_ENG_ALL);
#else
	_snprintf(temp_cmd,sizeof(temp_cmd),"%s/%s/%s",seleng,togeng,DL_SELECT_ENG_ALL);
#endif
	if (set_command_buff(temp_cmd) == FALSE)
	{
		errmsg("set_command_buff returned FALSE in starteng.\n");
		return FALSE;
	}
	if (SendMsg() == TRUE)
	{
		return TRUE;
	} else
	{
		return FALSE;
	}
}

/*! \brief This will copy m_buff into the temp_buff to promote thread safety.
\param temp_buff : storage for the private buffer m_buff to be copied to
\param buff_size : size of temp_buff
\return \b boolean
\sa dl_strncpy
\note tries to make sure the receiving buffer is large enough to receive the data
*/

bool C_DeviceLink::get_read_buff(char* temp_buff, unsigned int buff_size)
{
	if (temp_buff[0] != NULL)
	{
		errmsg("temp_buff not initialized to NULL in get_read_buff.\n");
		return FALSE;
	}
	if(m_buff[0] != ANSWER)
	{
		errmsg("Not a valid response code in buffer!\n");
		return FALSE;
	}
	if (buff_size < strlen(m_buff))
	{
		errmsg("Insufficient temp buffer size in get_read_buff.\n");
		return FALSE;
	}
	MC_Lock m_Lock(&my_critsec);
	dl_strncpy(temp_buff, m_buff,buff_size);	
	return TRUE;
}

/*! \brief This copies the string temp_buff into m_buff.
\param temp_buff : the string to be stored in m_buff
\param buff_size: the size of temp_buff
\return \b boolean
\sa dl_strncpy()
*/
bool C_DeviceLink::set_read_buff(const char* temp_buff, unsigned int buff_size)
{
	if (temp_buff == NULL)
	{
		MC_Lock m_Lock(&my_critsec);
		memset(m_buff, NULL,sizeof(m_buff));
		return TRUE;
	}
	if (buff_size > sizeof(m_buff))
	{
		errmsg("temp buffer size too large for m_buff in set_read_buff.\n");
		return FALSE;
	}
	MC_Lock m_Lock(&my_critsec); 
	dl_strncpy(m_buff,const_cast<char *>(temp_buff), sizeof(m_buff));	
	return TRUE;
}

/*! \brief Thread-safe method of setting the m_readdata flag. 
\param flag : boolean value to set the m_readdata flag to
\return \b boolean
\note m_readdata is used to verify that data has, in fact, been read from 
the game before trying to process it.

*/
bool C_DeviceLink::set_has_read_data(bool flag)
{
	MC_Lock m_Lock(&my_critsec);
	m_readdata = flag;
	return TRUE;
}

/*! \brief Returns the contents of m_cmd buffer and places it into temp_buff.
\param temp_buff : buffer created to store the contents of m_cmd
\param buff_size : size of temp_buff
\return \b boolean
\sa dl_strncpy()
*/
bool C_DeviceLink::get_cmd_buff(char* temp_buff, unsigned int buff_size)
{
	if (temp_buff[0] != NULL)
	{
		errmsg("temp_buff not initialized to NULL in get_cmd_buff.\n");
		return FALSE;
	}
	if (buff_size < strlen(m_cmd))
	{
		errmsg("Insufficient temp buffer size in get_cmd_buff.\n");
		return FALSE;
	}
	MC_Lock m_Lock(&my_critsec); 
	dl_strncpy(temp_buff, m_cmd, buff_size);
	return TRUE;
}

/*! \brief Output debug message if the debug has been turned on
\param str : string you want to print to the file.
*/
void C_DeviceLink::errmsg(const char* str)
{
#ifdef DEBUG_OUTPUT
	fprintf(dl_output, str);
#endif
}

/*! \brief Returns the appropriate stored engine part variable
\param \b const \b int eng_num : Index of the requested engine
\param \b float* engine_part : the part to be retrieved
\sa IsInitialized()
*/
float C_DeviceLink::getengfloats(const int eng_num, float *engine_part)
{
	if (IsInitialized() == FALSE)
	{
		init_err();
		return -1.00;
	}
	if ((eng_num < ENGINE_ONE) | (eng_num > ENGINE_FOUR))
	{
		errmsg("getengfloats called with invalid engine number.\n");
		return -1.00;
	}
	MC_Lock m_Lock(&my_critsec);
	return *engine_part;
}

/*! \brief Display init error

*/
void C_DeviceLink::init_err(void)
{
		errmsg("DeviceLink object not Initialized.\nYou must call Init() first!");
}

/*! \brief private function for setting the various controls
\param const char* code : the devicelink code for the control to be set
\param float pos : the value you to set the control too

*/
bool C_DeviceLink::setctrl(const char* code, float pos)
{
	char tmp_cmd[80];
	memset(tmp_cmd,NULL,sizeof(tmp_cmd));
#if _MSC_VER >= 1400
	_snprintf_s(tmp_cmd,sizeof(tmp_cmd),_TRUNCATE,"%s%c%f",code,DELIM_2,pos);
#else
	_snprintf(tmp_cmd,sizeof(tmp_cmd),"%s%c%f",code,DELIM_2,pos);
#endif
	if (set_command_buff(tmp_cmd) == FALSE)
	{
		errmsg("set_command_buff returned FALSE in setctrl.\n");
		return FALSE;
	}
	if (SendMsg()== TRUE)
	{
		return TRUE;
	} else
	{
		return FALSE;
	}
}

/*!  \brief takes code and int and asignes a float value to the passed in param
\param const char* code : devicelink code to convert
\param \b float : val value to set on converted string from read buffer

*/
void C_DeviceLink::getval(const char* code, int *val)
{
	//null out temp and call getparamval
	//to set temp equal to the string representation
	//of returned val. We then convert it to a float
	char temp[64];
	memset(temp, NULL, sizeof(temp));
	if (getparamval(code, temp, sizeof(temp)) == TRUE)
	{
		*val = atoi(temp);
	} else
	{
		errmsg("getparamval returned FALSE in getval.\n");
		*val = 0;
	}
}

/*!  \brief takes code and float and asignes a float value to the passed in param
\param const char* code : devicelink code to convert
\param \b float : val value to set on converted string from read buffer

*/
void C_DeviceLink::getval(const char* code, float *val)
{
	//null out temp and call getparamval
	//to set temp equal to the string representation
	//of returned val. We then convert it to a float
	char temp[256];
	memset(temp, NULL, sizeof(temp));
	if (getparamval(code, temp, sizeof(temp)) == TRUE)
	{
		*val = static_cast<float>(atof(temp));
	} else
	{
		errmsg("getparamval returned FALSE in getval.\n");
		*val = 0;
	}
}

/************************/
/* Public Method Section */
/************************/

/*! \brief Overload for Init(FILE *output) with no file pointer passed in. Just calls Init with a NULL.

*/
bool C_DeviceLink::Init()
{
	return Init(NULL);
}

/*! \brief Initialize the DeviceLink object. 
\param output : a file type parameter. If NULL then defaults to stdout
\return \b boolean
\note IP and port are read from a config.ini file incoming parameter is the 
debug output file. defaults to stderr. DEBUG_OUTPUT must be defined for 
messages to be written.  This is the first method to call when using this library.
It abstracts out the network code for setting up a UDP client and only needs the
config file with the IP address and port number of the server.
\note If Init is called with a FILE parameter and DEBUG_OUTPUT has been defined then
it will output the debug and error messages 

*/
bool C_DeviceLink::Init(FILE *output)
{
	//if the calling program passed in file name then log
	//to that file. Otherwise log to the console.
	if (output == NULL)
	{
		dl_output = stderr;
	} else
	{
		dl_output = output;
	}

	if (ReadConfig() == FALSE)
	{
		errmsg("Failed to read config file\n");
		m_initialized = FALSE;
		return FALSE;
	}
	
	//setup the UDP client 
	m_this_end.sin_port=htons(0);
	m_this_end.sin_addr.s_addr=htons(INADDR_ANY);
	m_this_end.sin_family=AF_INET;
	memset(&m_this_end.sin_zero, 0, sizeof(m_this_end.sin_zero));
	m_other_end.sin_port = htons(m_port);
	m_other_end.sin_addr.s_addr = inet_addr(m_game_ip);
	m_other_end.sin_family = AF_INET;
	memset(&m_other_end.sin_zero, 0, sizeof(m_other_end.sin_zero));
	WSADATA wsaData;
	
	//yeah, I know, MFC code but this was easier than rolling my own.
	//would have preferred to make this multi-platform. maybe a later
	//version.
	int retval = 0;
	retval = WSAStartup(0x202,&wsaData);

	if (retval != 0) 
	{
#ifdef DEBUG_OUTPUT
		fprintf(output,"WSAStartup failed with error %d\n",retval);
#endif
        WSACleanup();
        return FALSE;
    }

	//create the UDP socket.
	m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_sock == INVALID_SOCKET)
	{
#ifdef DEBUG_OUTPUT
		fprintf(dl_output,"Error in socket call in Init(). Error :%d\n", WSAGetLastError());
#endif
		WSACleanup();
		return FALSE;
	}
#pragma warning( push, 3 ) //Microsoft's FD_SET macro causes a Lvl 4 warning. Choosing to ignore it.
	FD_SET(m_sock, &ActiveReadFds);
#pragma warning( pop ) // pop back to lvl 4 warning.
	//bail if socket creation fails
	if (m_sock < 0)
	{
#ifdef DEBUG_OUTPUT
		fprintf(dl_output,"Client: Error Opening socket: Error %d\n",WSAGetLastError());
#endif
        WSACleanup();
		return FALSE;
	} 
	
	//bind the socket. bail if fails	
	if (bind(m_sock, reinterpret_cast<struct sockaddr*>(&m_this_end), sizeof(m_this_end)) == SOCKET_ERROR)
    {
#ifdef DEBUG_OUTPUT
		fprintf(dl_output,"Client: Error binding socket: Error %d\n",WSAGetLastError());
#endif
        WSACleanup();
		return FALSE;
	} 

	if ( connect( m_sock, reinterpret_cast<struct sockaddr*>(&m_other_end), sizeof(m_other_end) ) == SOCKET_ERROR)
	{
#ifdef DEBUG_OUTPUT
		fprintf(dl_output,"Failed to connect. Error %d\n",WSAGetLastError());
#endif
		WSACleanup();
		return FALSE;
	}

	m_initialized = TRUE;
	return TRUE;
}

/*! \brief Send the string stored in m_buff to the game server. 
\return \b boolean
\sa IsIntialized()
\sa get_cmd_buffer()

\note Started to make this method private but opted to keep it public so that any command/query string could be 
built and use this method to transmit.  It will fail on IsInitialized() fail or socket error. 

*/
bool C_DeviceLink::SendMsg()
{	
	if (IsInitialized() == FALSE)
	{
		init_err();
		return FALSE;
	}

	int err = 0;
	char temp_cmd[80];
	memset(temp_cmd, NULL,sizeof(temp_cmd));
	if (get_cmd_buff(temp_cmd, sizeof(temp_cmd)) == FALSE)
	{
		errmsg("get_cmd_buff returned FALSE in SendMsg.\n");
		return FALSE;
     }
	int len = static_cast<int>(strlen(temp_cmd));
	err = send(m_sock, temp_cmd, len, 0);
	if (err == SOCKET_ERROR)
	{
#ifdef DEBUG_OUTPUT
		fprintf(dl_output, "error in SendMSg. Error %d\n",WSAGetLastError());
#endif
        WSACleanup();
		return FALSE;
	}
	return TRUE;
}

/*! \brief Read response to the query and store it in the private m_buff. 
\return \b boolean

\note This fails if IsInitialized() fails and on socket error.  If it succeeds
it calls set_has_read_data(bool flag) to set the HasData() properly. It also
loads the in the internal m_buff with the data read from the socket via the 
set_read_buff(char* temp_buff) function.
\warning Calling routine must parse out the buffer for actual values.  

*/

bool C_DeviceLink::ReadMsg()
{
	if (IsInitialized() == FALSE)
	{
		init_err();
		return FALSE;
	}
	size_t len = 0;
	size_t buff_len = 0;
	char temp_buff[256];
	memset(temp_buff, NULL, sizeof(temp_buff));
	buff_len = sizeof(temp_buff);
	memset(&ActiveReadFds, 0x0, sizeof(fd_set));
#pragma warning( disable : 4127) //supressing compiler warning about this macro. Not my macro so can't fix it.
	FD_SET(m_sock, &ActiveReadFds);
	struct timeval tv;
	memset(&tv, 0x0, sizeof(struct timeval));
	tv.tv_usec = 250000; //wait no more than .25 of a second.
	int chk = select(0, &ActiveReadFds, NULL, NULL, &tv);
	if (chk <= 0)
	{
#ifdef DEBUG_OUTPUT
		fprintf(dl_output,"no data to read in ReadMSg()\n");
#endif
		set_has_read_data(FALSE);
		return FALSE;
	} 
	len = recv(m_sock, temp_buff, buff_len, 0);
	if (len == SOCKET_ERROR)
    {
#ifdef DEBUG_OUTPUT
		fprintf(dl_output,"Client: Error receiving from socket: Error %d\n",WSAGetLastError());
#endif
		set_has_read_data(FALSE);
        WSACleanup();
		return FALSE;
	} 
	temp_buff[len] = '\0';
	buff_len = strlen(temp_buff);
	if ((set_read_buff(temp_buff, buff_len) == FALSE) | (buff_len == 0) | (len == SOCKET_ERROR))
	{
		errmsg("set_read_buff returned FALSE in ReadMsg,\nor no data was read from the socket");
		set_has_read_data(FALSE);
		return FALSE;
	}
	set_has_read_data(TRUE);
	return TRUE;
}

/*! \brief Returns true if the gear is already up and false if it isn't.
\return \b boolean

\note Fails if queryfloat() fails or the gear is actually up.

*/
bool C_DeviceLink::Gear_Is_Up()
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_GEAR_STATUS);
	if (fval > 0.00)
	{
		return FALSE;
	} else if (fval < 0.00)
	{
		errmsg("queryfloat returned an error. -1 was returned.\n");
		return FALSE; 
	} else
	{
		errmsg("Gear already up in Gear_Is_Up\n");
		return TRUE;		
	}
}

/*! \brief Toggles the state of the landing gear
\return \b boolean

\note Toggles the gear setting and utilizes the toggleswitch() function.

*/
bool C_DeviceLink::ToggleGear()
{	
	return toggleswitch(DL_SET_GEAR_STATUS);
}

/*! \brief Sets the landing gear to be up.
\return \b boolean
\note Only fails when IsIntialized() fails. Executes ToggleGear()
if Gear_Is_Up() returns a false.  

*/
bool C_DeviceLink::SetGearUp()
{
	if (Gear_Is_Up() == TRUE)
	{
		errmsg("Gear already up in SetGearUp\n");
	} else
	{
		ToggleGear();
	}
	return TRUE;
}

/*! \brief Sets the gear to be down
\return \b boolean
*/
bool C_DeviceLink::SetGearDown()
{
	if (Gear_Is_Up() == TRUE)
	{
		ToggleGear();
	} else
	{
		errmsg("Gear already down in SetGearDown\n");
	}
	return TRUE;
}

/*! \brief Takes the passed in code for querying a gear position and returns a float.
\param gearcode : the specific gear code to query the server about
\return \b float : the float position ranging from 0.0 to 1.0 of the current position of the gear
\note This is for checking each individual wheel versus just an overall status. 
Return of -1 means an error occurred in queryfloat().

*/
float C_DeviceLink::GetGearPos(char* gearcode)
{
	float gearpos = 0.00;
	gearpos = queryfloat(gearcode);
	if (gearpos >= 0.00)
	{
		return gearpos;
	}
	errmsg("GetGearPos:queryfloat returned a -1 to GetGearPos\n");
	return -1.00;
}

/*! \brief send manual lower gear command
\return \b boolean
*/
bool C_DeviceLink::ManualGearDown(void)
{
	return toggleswitch(DL_LOWER_GEAR_MAN);
}

/*! \brief send manual raise gear command
\return \b boolean
*/
bool C_DeviceLink::ManualGearUp(void)
{
	return toggleswitch(DL_RAISE_GEAR_MAN);
}
/*! \brief This reads in the config.ini file.
\return \b boolean
\todo I'm likely to rewrite this in the future. It's been written piecemeal 
so it's sloppy. 
\note It reads the config.ini file for the settings.

*/
bool C_DeviceLink::ReadConfig()
{
#if _MSC_VER >= 1400
	FILE *fp = NULL;
	errno_t err = 0;
	err = fopen_s(&fp, "config.ini", "r");
	if (err != 0)
	{
		errmsg("Cannot open config file.\n");
		exit(1);
	}
#else
	FILE *fp = NULL;
	fp = fopen("config.ini", "r");
	if (fp == NULL)
	{
		errmsg("Cannot open config file.\n");
		exit(1);
	}
#endif
	char temp[80];
	memset(temp, NULL, sizeof(temp));
	fgets(temp, 19,fp);
	if (temp[1] != 'D')
	{
		errmsg("Invalid config file\n");
		fclose(fp);
		return FALSE;
	}

	fgets(temp, 19, fp);
	if (temp[0] == 'P')
	{
		char* pdest = strstr(temp,"=") + 1;
		if (pdest == NULL)
		{
			errmsg("Invalid IP address in config file.\n");
			fclose(fp);
			return FALSE;
		}
		m_port = static_cast<u_short>(atoi(pdest));
		if ((m_port == 0) )
		{
			errmsg("Invalid port number in config file.\n");
			fclose(fp);
			return FALSE;
		}
	} else
	{
		errmsg("Invalid config file\n");
		fclose(fp);
		return FALSE;
	}

	fgets(temp,19,fp);
	if (temp[0] == 'I')
	{
		char* pdest = strstr(temp,"=") + 1;
		if (pdest == NULL)
		{
			errmsg("Invalid IP address in config file.\n");
			fclose(fp);
			return FALSE;
		}
		dl_strncpy(m_game_ip,pdest,sizeof(m_game_ip));
		if (m_game_ip[0] == NULL)
		{
			errmsg("Invalid IP address in config file.\n");
			fclose(fp);
			return FALSE;
		}
	} else
	{
		errmsg("Invalid config file\n");
		fclose(fp);
		return FALSE;
	}	

	fclose(fp);
	return TRUE;
}

/*! \brief Returns the Initialized state in a bool. Utilizes MC_Lock for thread safety.
\return \b boolean

*/
bool C_DeviceLink::IsInitialized()
{
	MC_Lock m_Lock(&my_critsec);
	return m_initialized;
}

/*! \brief This is the public method for querying the devicelink server and reading a response.
\param code : devicelink defined code to send the server and expect a response
\return \b boolean
\note It takes the string passed in by code and calls SendMsg() to send the 'R' query and then a ReadMsg() to get the response. 
*/
bool C_DeviceLink::QueryMsg(const char* code)
{
	//send the query string and read the subsequent response.
	if (set_command_buff(code) == FALSE)
	{
		errmsg("set_command_buff returned FALSE in QueryMsg.\n");
		return FALSE;
	}
	if (SendMsg() == TRUE)
	{
		if(ReadMsg() == TRUE)
		{
			if (HasData() == TRUE)
			{
				return TRUE;
			} else
			{
				errmsg("No data read from socket. Server may not be up\n");
				return FALSE;
			}
		} else
		{
			errmsg("Read failed. No data read from socket. Server may not be up\n");
			return FALSE;
		}
	} else
	{	
		errmsg("Catastrophic socket failure. SendMsg failed.\n");
		set_has_read_data(FALSE);
		return FALSE;
	}

}

/*! \brief Return a flag status set when data has actually been received from the game.
\return \b boolean
*/
bool C_DeviceLink::HasData()
{
	MC_Lock m_Lock(&my_critsec);
	return m_readdata;
}

/*! \brief Get the version of DeviceLink you are running from the game.
\param verstr : is where you are going to store the result
\param buff_size : is the size of verstr to be passed in
\return \b boolean
\warning verstr must be allocated by calling routine and must be at least 64 bytes!
\warning buff_size must be passed in and be sizeof(verstr) in calling routine!

*/
bool C_DeviceLink::GetDLVersion(char* verstr, unsigned int buff_size)
{
	if (buff_size < sizeof(m_dl_ver))
	{
		errmsg("Calling routine allocated insuffcient buffer. Must be at least 64 bytes!\n");
		return FALSE;
	}
	if (querystring(DL_GET_VERSION, m_dl_ver, sizeof(m_dl_ver)) == TRUE)
	{
		dl_strncpy(verstr, m_dl_ver, buff_size);
		return TRUE;
	}
	errmsg("querystring returned FALSE to GetDLVersion.\n");
	return FALSE;
}

/*! \brief Verifies that a Get command exists for the associated code.
\param code : the command code you are checking to see if it has a Get equivalent
\return \b boolean

*/
bool C_DeviceLink::ValidGet(const char* code)
{
	char newcode[64];
	char temp[64];
	memset(temp, NULL,sizeof(temp));
	memset(newcode, NULL,sizeof(newcode));
#if _MSC_VER >= 1400
	_snprintf_s(newcode,sizeof(newcode),_TRUNCATE,"%s%c%s",DL_ACCESS_GET,DELIM_2,code);
#else
	_snprintf(newcode,sizeof(newcode),"%s%c%s",DL_ACCESS_GET,DELIM_2,code);
#endif

	if (querystring(newcode, temp, sizeof(temp)) == TRUE)
	{
		return TRUE;
	}
	return FALSE;
}

/*! \brief Get the Time of Day
\return Returns either the TOD in a float value or a -1 as an error.
*/
float C_DeviceLink::GetTOD()
{
	float tod = 0.00;
	tod = queryfloat(DL_TOD);
	if (tod >= 0.00)
	{
		return tod;
	}
		errmsg("queryfloat returned a -1 to GetTOD\n");
		return 0.00;
}

/*! \brief Get the ID of the aircraft. 
\param ac : store the result of the aircraft name query
\param buff_size : is the size of verstr to be passed in
\return \b boolean
\warning Calling routine must allocate memory for ac. Minimum of 64 bytes.

*/
bool C_DeviceLink::GetAircraftID(char *ac, unsigned int buff_size)
{	
	char temp[64];
	memset(temp, NULL, sizeof(temp));
	if (buff_size < sizeof(temp))
	{	
		errmsg("Insuffcient buffer allocated for aircraft type name.  Must be at least 64 bytes.\n");
		return FALSE;
	}
	if (querystring(DL_GET_PLANE, temp, buff_size) == TRUE)
	{
		//copy temp into the buffer pointed to by ac but let's be safe and
		//do a sanity check on ac while we are at it.
		dl_strncpy(ac, temp, buff_size);
		return TRUE;
	}
	return FALSE;
}
/*! \brief returns the index of the cockpit. 0 = pilot's command cockpit
\return \b integer : 0 == pilot, -1 == external
*/
int C_DeviceLink::GetCurCockpit(void)
{
	return queryint(DL_GET_CUR_COCKPIT);
}

/*! \brief returns the current overload value
/return \b float : range is MININT to MAXINT

\note no clue what this value actually means. G load?
*/
float C_DeviceLink::Get_Overload(void)
{
	return queryfloat(DL_GET_OVERLOAD);
}

/*! \brief returns the current shake float value
/return \b float : range is 0 to 1

\note no clue what this value actually means. G load?
*/
float C_DeviceLink::Get_ShakeLvl(void)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_SHAKE);
	if ((0.00 > fval) || (1.00 < fval))
	{
		errmsg("queryfloat returned an error in Get_ShakeLvl.\n");
		return 0.00;
	}
	return fval;
}
/*! \brief Returns the number of cockpits in the aircraft
\return \b integer : number of cockpits in the aircraft
*/
int C_DeviceLink::GetNumOfCockpits()
{
	return queryint(DL_GET_COCKPITS);
}

/*! \brief Toggle the state of the Nav lights.
\return \b boolean
*/
bool C_DeviceLink::ToggleNavLights()
{
	return toggleswitch(DL_TOGGLE_NAV_LTS);
}

/*! \brief Toggle the state of the Panel lights
\return \b boolean
*/
bool C_DeviceLink::ToggleCockpitLights()
{
	return toggleswitch(DL_TOGGLE_COCKPIT_LTS);
}
/*! \brief Toggle the state of the landing lights
\return \b boolean
*/
bool C_DeviceLink::ToggleLandLights()
{
	return toggleswitch(DL_TOGGLE_LND_LTS);
}

/*! \brief Toggle the wingtip smoke
\return \b boolean
*/
bool C_DeviceLink::ToggleSmoke(void)
{	
	return toggleswitch(DL_TOGGLE_SMOKE);
}
/*! \brief Toggle the Auto Bombsight
\return \b boolean
*/
bool C_DeviceLink::ToggleBombSight(void)
{
	return toggleswitch(DL_TOGGLE_SIGHT_AUTO);
}
/*! \brief Toggles the arresting hook
\return \b boolean
*/
bool C_DeviceLink::ToggleHook(void)
{
	return toggleswitch(DL_TOGGLE_HOOK);
}
/*! \brief increases the angle of the bombsight
\return \b boolean
*/
bool C_DeviceLink::IncSightDist(void)
{
	return toggleswitch(DL_INC_SIGHT);
}
/*! \brief decreases the angle of the bombsight
\return \b boolean
*/
bool C_DeviceLink::DecSightDist(void)
{
	return toggleswitch(DL_DEC_SIGHT);
}
/*! \brief adjusts bombsight to the right
\return \b boolean
*/
bool C_DeviceLink::AdjSightRight(void)
{
	return toggleswitch(DL_RIGHT_SIGHT);
}

/*! \brief adjusts bombsight to the left
\return \b boolean
*/
bool C_DeviceLink::AdjSightLeft(void)
{
	return toggleswitch(DL_LEFT_SIGHT);
}

/*! \brief increases bombsight altitude
\return \b boolean
*/
bool C_DeviceLink::IncSightAlt(void)
{
	return toggleswitch(DL_INC_SIGHT_ALT);
}

/*! \brief decreases bombsight altitude
\return \b boolean
*/
bool C_DeviceLink::DecSightAlt(void)
{
	return toggleswitch(DL_DEC_SIGHT_ALT);
}

/*! \brief increases bombsight velocity
\return \b boolean
*/
bool C_DeviceLink::IncSightVelocity(void)
{
	return toggleswitch(DL_INC_SIGHT_VEL);
}

/*! \brief decreases bombsight velocity
\return \b boolean
*/
bool C_DeviceLink::DecSightVelocity(void)
{
	return toggleswitch(DL_DEC_SIGHT_VEL);
}
/*! \brief Query for gunpod state
\return \b boolean
*/
bool C_DeviceLink::Query_GunPods(void)
{
	int ival = 0;
	ival = queryint(DL_GET_GUNPOD);
	if ((0 > ival) || (1 < ival))
	{
		errmsg("queryfloat returned an error in Query_GunPods.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_gunpod = ival;
	return TRUE;
}
/*! \brief Toggle the Gunpods
\return \b boolean
*/
bool C_DeviceLink::ToggleGunPods(void)
{
	return toggleswitch(DL_SET_GUNPOD);
}

/*! \brief returns the Gunpod state private var
\return \b integer : 0 is off and 1 is on

*/
int C_DeviceLink::GetGunPodsState(void)
{
	return m_gunpod;
}
/*! \brief Start Engine one. Actually, it selects the engine, toggles it, then selects all.
\return \b boolean
*/
bool C_DeviceLink::StartEng1(void)
{
	return starteng(DL_SELECT_ENG_1,DL_TOGGLE_ENGINE1);
}

/*! \brief Start Engine two. Actually, it selects the engine, toggles it, then selects all.
\return \b boolean
*/
bool C_DeviceLink::StartEng2(void)
{
	return starteng(DL_SELECT_ENG_2,DL_TOGGLE_ENGINE2);
}

/*! \brief Start Engine three. Actually, it selects the engine, toggles it, then selects all.
\return \b boolean
*/
bool C_DeviceLink::StartEng3()
{
	return starteng(DL_SELECT_ENG_3,DL_TOGGLE_ENGINE3);
}

/*! \brief Start Engine four. Actually, it selects the engine, toggles it, then selects all.
\return \b boolean
*/
bool C_DeviceLink::StartEng4()
{
	return starteng(DL_SELECT_ENG_4,DL_TOGGLE_ENGINE4);
}

/*! \brief Get the number of Magnetos in the aircraft
\param eng_num : defined const int of the engine index to query 
\return Returns the number of magnetos or -1 if there is an error.
*/
int C_DeviceLink::GetMags(const int eng_num)
{
	if ((eng_num < ENGINE_ONE) | (eng_num > ENGINE_FOUR))
	{
		errmsg("GetMags called with invalid engine number.\n");
		return -1;
	}
	char tmp_cmd[64];
	memset(tmp_cmd,NULL,sizeof(tmp_cmd));
#if _MSC_VER >= 1400
	_snprintf_s(tmp_cmd,sizeof(tmp_cmd),_TRUNCATE,"%s%c%d",DL_GET_MAG,DELIM_2,eng_num);
#else
	_snprintf(tmp_cmd,sizeof(tmp_cmd),"%s%c%d",DL_GET_MAG,DELIM_2,eng_num);
#endif
	int ival = 0;
	ival = queryint(tmp_cmd);
	if (ival <= 0)
	{
		errmsg("Invalid number of Mags returned from queryint().\n");
		return -1;
	}
	return ival;
}

/*! \brief Feather the selected engine
\return \b boolean
\warning You must call the appropriate select engine prior to calling this. Otherwise
it defaults to Engine 1.

*/
bool C_DeviceLink::FeatherEngine(void)
{
	int ival = 0;
	ival = queryint(DL_GET_FEATHER);
	if (ival == 1)
	{
		return toggleswitch(DL_TOGGLE_FEATHER);
	} else
	{
		return FALSE;
	}
}

/*! \brief return the number of engines
\return \b int : returns the numbers of engines
*/
int C_DeviceLink::GetNumEngines(void)
{
	int ival = 0;
	ival = queryint(DL_GET_ENGINES);
	if ((ival <= 0) || (ival > 8))
	{
		errmsg("Invalid number of Mags returned from queryint(DL_GET_MAG).\n");
		return -1;
	}
	return ival;
}

/*! \brief Select/unselect Engine 1
\return \b boolean
*/
bool C_DeviceLink::ToggleEng1Select(void)
{	
	return toggleswitch(DL_TOGGLE_ENG1_SELECT);
}

/*! \brief Select/unselect Engine 2
\return \b boolean
\sa toggleswitch()
*/
bool C_DeviceLink::ToggleEng2Select(void)
{	
	return toggleswitch(DL_TOGGLE_ENG2_SELECT);
}

/*! \brief selects all the engines
\return \b boolean 
*/
bool C_DeviceLink::SelectAllEngines(void)
{
	return toggleswitch(DL_SELECT_ENG_ALL);
}

/*! \brief unselects all the engines
\return \b boolean 
*/
bool C_DeviceLink::UnSelectAllEngines(void)
{
	return toggleswitch(DL_UNSELECT_ENG_ALL);
}

/*! \brief selects the left engines
\return \b boolean 
*/
bool C_DeviceLink::SelectLeftEngines(void)
{
	return toggleswitch(DL_SELECT_ENG_LEFT);
}

/*! \brief selects the right engines
\return \b boolean 
*/
bool C_DeviceLink::SelectRightEngines(void)
{
	return toggleswitch(DL_SELECT_ENG_RIGHT);
}

/*! \brief toggles selecting all the engines 
\return \b boolean 
*/
bool C_DeviceLink::ToggleSelAllEngines(void)
{
	return toggleswitch(DL_TOGGLE_ENGINES_ALL);
}

/*! \brief toggles selecting the left engines
\return \b boolean 
*/
bool C_DeviceLink::ToggleSelLeftEngines(void)
{
	return toggleswitch(DL_TOGGLE_ENGINES_LEFT);
}

/*! \brief toggles selecting the right engines
\return \b boolean 
*/
bool C_DeviceLink::ToggleSelRightEngines(void)
{
	return toggleswitch(DL_TOGGLE_ENGINES_RIGHT);
}
/*! \brief Simply executes command to select engine 1.
\return \b boolean
*/
bool C_DeviceLink::SelectEng1(void)
{
	return toggleswitch(DL_SELECT_ENG_1);
}

/*! \brief Simply executes command to select engine 2.
\return \b boolean
*/
bool C_DeviceLink::SelectEng2(void)
{
	return toggleswitch(DL_SELECT_ENG_2);
}


/*! \brief Simply executes command to select engine 3.
\return \b boolean
*/
bool C_DeviceLink::SelectEng3(void)
{
	return toggleswitch(DL_SELECT_ENG_3);
}

/*! \brief Simply executes command to select engine 4.
\return \b boolean
*/
bool C_DeviceLink::SelectEng4(void)
{
	return toggleswitch(DL_SELECT_ENG_4);
}

/*! \brief Simply executes command to toggle the selection of engine 1.
\return \b boolean
*/
bool C_DeviceLink::ToggleSelEng1(void)
{
	return toggleswitch(DL_TOGGLE_ENG1_SELECT);
}

/*! \brief Simply executes command to toggle the selection of engine 2.
\return \b boolean
*/
bool C_DeviceLink::ToggleSelEng2(void)
{
	return toggleswitch(DL_TOGGLE_ENG2_SELECT);
}

/*! \brief Simply executes command to toggle the selection of engine 3.
\return \b boolean
*/
bool C_DeviceLink::ToggleSelEng3(void)
{
	return toggleswitch(DL_TOGGLE_ENG3_SELECT);
}

/*! \brief Simply executes command to toggle the selection of engine 4.
\return \b boolean
*/
bool C_DeviceLink::ToggleSelEng4(void)
{
	return toggleswitch(DL_TOGGLE_ENG4_SELECT);
}
/*! \brief This function will populate the private engine data structure of the passed engine number. 
\param eng_num : the engine index (0-3) you want to get data for
\return \b boolean
*/
bool C_DeviceLink::Set_Engine_Data(const int eng_num)
{
	if ((eng_num < ENGINE_ONE) | (eng_num > ENGINE_FOUR))
	{
		errmsg("Set_Engine_Data called with invalid engine number.\n");
		return FALSE;
	}
	//since the response from the server will include engine num we need to set
	//the parameter parser to 2.
	m_ret_cnt = 2;
	/********************************************************
	* We are going to make a special call to devicelink to  *
	* take advantage of the multiple query on a line aspect *
	* of the  engine data.                             *
	********************************************************/
	//build the query string
	char temp_cmd[80];
	memset(temp_cmd, NULL,sizeof(temp_cmd));
#if _MSC_VER >= 1400
	_snprintf_s(temp_cmd,sizeof(temp_cmd),_TRUNCATE,"64\\%d/66\\%d/68\\%d/70\\%d/72\\%d/74\\%d\0",eng_num,eng_num,eng_num,eng_num,eng_num,eng_num);
#else
	_snprintf(temp_cmd,sizeof(temp_cmd),"64\\%d/66\\%d/68\\%d/70\\%d/72\\%d/74\\%d\0",eng_num,eng_num,eng_num,eng_num,eng_num,eng_num);
#endif
	if ( QueryMsg(temp_cmd) == FALSE )
	{
		errmsg("QueryMsg returned false in Set_Engine_Data.\n");
		return FALSE;
	}

	MC_Lock m_Lock(&my_critsec);
	getval(DL_GET_MANIFOLD,&m_engine[eng_num].manifold);
	getval(DL_GET_TEMP_WATER,&m_engine[eng_num].temp_water);
	getval(DL_GET_TEMP_OILOUT,&m_engine[eng_num].temp_oilout);
	getval(DL_GET_TEMP_OILIN,&m_engine[eng_num].temp_oilin);
	getval(DL_GET_TEMP_CYL,&m_engine[eng_num].temp_cylinders);
	getval(DL_GET_RPM,&m_engine[eng_num].rpm);
	m_ret_cnt = 1;
	return TRUE;
}

/*! \brief Sets the stored rpm value from the indexed engine
\param eng_num : the index of the engine you want to query the server for
\return \b boolean
*/
bool C_DeviceLink::Set_RPM(const int eng_num)
{
	return setengfloats(eng_num, DL_GET_RPM, &m_engine[eng_num].rpm);
}
/*! \brief Returns the stored RPMs in float from indexed engine
\param eng_num : the index of the engine you want to query the server for
\return \b float : the stored RPM value in the private engine structure
*/
float C_DeviceLink::Get_RPM(const int eng_num)
{
	return getengfloats(eng_num, &m_engine[eng_num].rpm);
}
/*! \brief Sets the internal structure of the cylinder temp
\param eng_num : the index of the engine you want to query the server for
\return \b boolean
*/
bool C_DeviceLink::Set_Temp_Cyl(const int eng_num)
{
	return setengfloats(eng_num, DL_GET_TEMP_CYL, &m_engine[eng_num].temp_cylinders);
}

/*! \brief Returns the stored Cylinder temperature (Celsius) in float from indexed engine
\param eng_num : the index of the engine you want to query the server for
\return \b float : the stored temperature cylinder value in the private engine structure
*/
float C_DeviceLink::Get_Temp_Cyl(const int eng_num)
{
	return getengfloats(eng_num,&m_engine[eng_num].temp_cylinders);
}

/*! \brief Sets the Oil In temp in (Celsius) from the indexed engine.
\param eng_num : the index of the engine you want to query the server for
\return \b boolean
*/
bool C_DeviceLink::Set_Temp_Oilin(const int eng_num)
{
	return setengfloats(eng_num, DL_GET_TEMP_OILIN, &m_engine[eng_num].temp_oilin);
}

/*! \brief Returns the stored Oil in temperature (Celsius) in float from indexed engine
\param eng_num : the index of the engine you want to query the server for
\return \b float : the stored oil out temperature value in the private engine structure
\note Fails if invalid eng_num is passed in.

*/
float C_DeviceLink::Get_Temp_Oilin(const int eng_num)
{
	return getengfloats(eng_num,&m_engine[eng_num].temp_oilin);
}
/*! \brief Sets the stored Oil Out temp (Celsius) from the indexed engine.
\param eng_num : the index of the engine you want to query the server for
\return \b boolean
\sa setengfloats()
*/
bool C_DeviceLink::Set_Temp_Oilout(const int eng_num)
{
	return setengfloats(eng_num, DL_GET_TEMP_OILOUT, &m_engine[eng_num].temp_oilout);
}

/*! \brief Returns the stored Oil Out temperature (Celsius) in float from indexed engine
\param eng_num : the index of the engine you want to query the server for
\return \b float : the stored oil out temperature value in the private engine structure
\sa getengfloats()
\note Fails if invalid eng_num is passed in.

*/
float C_DeviceLink::Get_Temp_Oilout(const int eng_num)
{
	return getengfloats(eng_num,&m_engine[eng_num].temp_oilout);
}
/*! \brief Sets water temp in (Celsius) from the indexed engine.
\param eng_num : the index of the engine you want to query the server for
\return \b boolean

*/
bool C_DeviceLink::Set_Temp_Water(const int eng_num)
{
	return setengfloats(eng_num, DL_GET_TEMP_WATER, &m_engine[eng_num].temp_water);
}

/*! \brief Returns the stored Water temperature (Celsius) in float from indexed engine
\param eng_num : the index of the engine you want to query the server for
\return \b float : the stored water temperature value in the private engine structure

*/
float C_DeviceLink::Get_Temp_Water(const int eng_num)
{
	return getengfloats(eng_num,&m_engine[eng_num].temp_water);
}

/*! \brief Sets the stored manifold pressure from the indexed engine by querying the game.
\param eng_num : the index of the engine you want to query the server for
\return \b boolean

*/
bool C_DeviceLink::Set_Manifold(const int eng_num)
{
	return setengfloats(eng_num, DL_GET_MANIFOLD, &m_engine[eng_num].manifold);
}

/*! \brief Returns the stored manifold pressure in float from indexed engine
\param eng_num : the index of the engine you want to query the server for
\return \b float : the stored manifold pressure value in the private engine structure

\note This value was set in Set_Manifold()

*/
float C_DeviceLink::Get_Manifold(const int eng_num)
{
	return getengfloats(eng_num,&m_engine[eng_num].manifold);
}

/*! \brief increase super charger to next stage.
\param \b int : the engine number
\return \b boolean

*/
bool C_DeviceLink::SuperChgNxt(const int eng_num)
{
	if ((eng_num < ENGINE_ONE) | (eng_num > ENGINE_FOUR))
	{
		errmsg("SuperChgNxt called with invalid engine number.\n");
		return FALSE;
	}

	if (set_command_buff(DL_SET_CHRG_NXT) == FALSE)
	{
		errmsg("set_command_buff returned FALSE in SuperChgNxt.\n");
		return FALSE;
	}	
	return SendMsg();
}

/*! \brief increase super charger to prev stage.
\param \b int : the engine number
\return \b boolean

*/
bool C_DeviceLink::SuperChgPrv(const int eng_num)
{
	if ((eng_num < ENGINE_ONE) | (eng_num > ENGINE_FOUR))
	{
		errmsg("SuperChgNxt called with invalid engine number.\n");
		return FALSE;
	}

	if (set_command_buff(DL_SET_CHRG_PREV) == FALSE)
	{
		errmsg("set_command_buff returned FALSE in SuperChgPrev.\n");
		return FALSE;
	}	
	return SendMsg();
}

/*! \brief Gets the current state of the WEP of selected engine
\return \b int : 0 is off, 1 is on
\note must select engine first!
*/
int C_DeviceLink::Get_WEP(void)
{
	int ival = 0;
	ival = queryint(DL_GET_WEP);
	if (ival <= 0)
	{
		errmsg("Invalid WEP state returned from queryint(DL_GET_MAG).\n");
		return -1;
	}
	return ival;
}

/*! \brief Toggle the WEP state

\note must select engine first!
*/
bool C_DeviceLink::ToggleWEP(void)
{
	return toggleswitch(DL_TOGGLE_WEP);
}

/*! \brief Sets the altitude
\return \b boolean

*/
bool C_DeviceLink::Set_Alt(void)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_ALT);
	if (0.00 > fval)
	{
		errmsg("queryfloat returned an error in Set_Alt.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_alt = fval;
	return TRUE;
}

/*! \brief returns the altitude stored in private variable
\return \b float : Altitude in meters
*/
float C_DeviceLink::Get_Alt(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_alt;
}

/*! \brief Sets the angular speed
\return \b boolean

*/
void C_DeviceLink::Set_AngSpd(void)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_ANG_SPD);
	MC_Lock m_Lock(&my_critsec);
	m_ang_spd = fval;
}

/*! \brief returns the angular speed
\return \b float : Angular Speed in degrees per second
*/
float C_DeviceLink::Get_AngSpd(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_ang_spd;
}

/*! \brief Sets the Azimuth
\return \b boolean

*/
bool C_DeviceLink::Set_Azimuth(void)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_AZI);
	if ((0.0 > fval) || (359.99 < fval))
	{
		errmsg("queryfloat returned a bad value in Set_Azimuth.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_azimuth = fval;
	return TRUE;
}

/*! \brief returns the azimuth stored in private variable
\return \b float : Azimuth in degrees 0-359.9
*/
float C_DeviceLink::Get_Azimuth(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_azimuth;
}

/*! \brief Sets the beacon azimuth
\return \b boolean

*/
bool C_DeviceLink::Set_BeaconAzimuth(void)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_BEACON_AZI);
	if ((0.0 > fval) || (359.99 < fval))
	{
		errmsg("queryfloat returned a bad value in Set_BeaconAzimuth.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_beacon_azimuth = fval;
	return TRUE;
}

/*! \brief returns the beacon azimuth stored in private variable
\return \b float : Beacon Azimuth in degrees 0-359.9
*/
float C_DeviceLink::Get_BeaconAzimuth(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_beacon_azimuth;
}

/*! \brief Sets the IAS
\return \b boolean

*/
bool C_DeviceLink::Set_IAS(void)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_IAS);
	if (0.00 > fval)
	{
		errmsg("queryfloat returned an error in Set_IAS.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_ias = fval;
	return TRUE;
}

/*! \brief returns the IAS stored in private variable
\return \b float : IAS in degrees km/h
*/
float C_DeviceLink::Get_IAS(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_ias;
}

/*! \brief Sets the Pitch
\return \b boolean

*/
bool C_DeviceLink::Set_Pitch(void)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_PITCH);
	if ((-90.0 > fval) || (90.0 < fval))
	{
		errmsg("queryfloat returned a bad value in Set_Pitch.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_pitch = fval;
	return TRUE;
}

/*! \brief returns the pitch stored in private variable
\return \b float : Pitch in degrees -90 -- +90
*/
float C_DeviceLink::Get_Pitch(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_pitch;
}

/*! \brief Sets the roll
\return \b boolean

*/
bool C_DeviceLink::Set_Roll(void)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_ROLL);
	if ((-180.0 > fval) || (180.0 < fval))
	{
		errmsg("Error in Set_Roll. Bad value in roll query\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_roll = fval;
	return TRUE;
}

/*! \brief returns the beacon azimuth stored in private variable
\return \b float : roll in degrees -180 -- +180
*/
float C_DeviceLink::Get_Roll(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_roll;
}

/*! \brief Sets the slip
\return \b boolean


*/
bool C_DeviceLink::Set_Slip(void)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_SLIP);
	if ((-45.00 > fval) || (45.00 < fval))
	{
		errmsg("queryfloat returned a bad value in Set_Slip.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_slip = fval;
	return TRUE;
}

/*! \brief returns the slip stored in private variable
\return \b float : roll in degrees -45 -- +45
*/
float C_DeviceLink::Get_Slip(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_slip;
}

/*! \brief Sets the variometer
\return \b boolean

*/
void C_DeviceLink::Set_Vario(void)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_VARIO);
	MC_Lock m_Lock(&my_critsec);
	m_vario = fval;
}

/*! \brief returns the variometer stored in private variable
\return \b float : variometer in meters per second
*/
float C_DeviceLink::Get_Vario(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_vario;
}

/*! \brief Sets the variometer
\return \b boolean

*/
bool C_DeviceLink::Set_Fuel(void)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_FUEL);
	if (0.00 > fval)
	{
		errmsg("queryfloat returned a bad value in Set_Fuel.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_fuel = fval;
	return TRUE;
}

/*! \brief returns the fuel stored in private variable
\return \b float : roll in kilograms
*/
float C_DeviceLink::Get_Fuel(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_fuel;
}

/*! \brief Sets the turn bank angle
\return \b boolean
\sa queryfloat()

*/
bool C_DeviceLink::Set_Turn(void)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_TURN);
	if ((-1.00 > fval) || (1.00 < fval))
	{
		errmsg("queryfloat returned an error in Set_Alt.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_turn = fval;
	return TRUE;
}

/*! \brief returns the altitude stored in private variable
\return \b float : Altitude in meters
*/
float C_DeviceLink::Get_Turn(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_turn;
}

/*! \brief returns the aileron postion stored in private variable
\return \b float : Altitude in meters
*/
float C_DeviceLink::Get_Aileron(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_aileron;
}

/*! \brief queries the game for the current aileron position 
and assigns that value to private var
\return \b float : aileron position
*/
bool C_DeviceLink::Query_Aileron(void)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_AILERON);
	if ((-1.00 > fval) || (1.00 < fval))
	{
		errmsg("queryfloat returned an error in Set_Alt.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_aileron = fval;
	return TRUE;
}

/*! \brief set the game airelons to the passed in value.
\param float : pos -- the pposition to set it too. -1.00 -- +1.00
\return \b boolean
*/
bool C_DeviceLink::Set_Aileron(float pos)
{
	return setctrl(DL_SET_AILERON, pos);
}

/*! \brief returns the elevator postion stored in private variable
\return \b float : Elevator position
*/
float C_DeviceLink::Get_Elevator(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_elevator;
}

/*! \brief queries the game for the current elevator position 
and assigns that value to private var
\return \b float : Elevator position
*/
bool C_DeviceLink::Query_Elevator(void)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_ELV);
	if ((-1.00 > fval) || (1.00 < fval))
	{
		errmsg("queryfloat returned an error in Query_Elevator.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_elevator = fval;
	return TRUE;
}

/*! \brief set the game Elevator to the passed in value.
\param float : pos -- the position to set it too. -1.00 -- +1.00
\return \b boolean
*/
bool C_DeviceLink::Set_Elevator(float pos)
{
	return setctrl(DL_SET_ELV, pos);
}

/*! \brief returns the aileron postion stored in private variable
\return \b float : Elevator position
*/
float C_DeviceLink::Get_Rudder(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_rudder;
}

/*! \brief queries the game for the current Rudder position 
and assigns that value to private var
\return \b float : Rudder position
*/
bool C_DeviceLink::Query_Rudder(void)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_ELV);
	if ((-1.00 > fval) || (1.00 < fval))
	{
		errmsg("queryfloat returned an error in Query_Rudder.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_rudder = fval;
	return TRUE;
}

/*! \brief set the game Rudder to the passed in value.
\param float : pos -- the position to set it too. -1.00 -- +1.00
\return \b boolean
*/
bool C_DeviceLink::Set_Rudder(float pos)
{
	return setctrl(DL_SET_RUDDER, pos);
}

/*! \brief returns the power postion stored in private variable
\param const int : eng_idx -- the index of the engine 
\return \b float : Power position of selected engine

*/
float C_DeviceLink::Get_Power(const int eng_idx)
{
	MC_Lock m_Lock(&my_critsec);
	return m_engine[eng_idx].power;
}

/*! \brief queries the game for the current power position 
and assigns that value to private var
\param const int : eng_idx -- the index of the engine 
\return \b float : power position
*/
bool C_DeviceLink::Query_Power(const int eng_idx)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_POWER);
	if ((-1.00 > fval) || (1.00 < fval))
	{
		errmsg("queryfloat returned an error in Query_Power.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_engine[eng_idx].power = fval;
	return TRUE;
}

/*! \brief set the game power to the passed in value.
\param const int : eng_idx -- the index of the engine to set
\param float : pos -- the position to set it too. -1.00 -- +1.00
\return \b boolean
*/
bool C_DeviceLink::Set_Power(const int eng_idx, float pos)
{
	char code[64];
	memset(code,NULL,sizeof(code));
#if _MSC_VER >= 1400
	_snprintf_s(code,sizeof(code),_TRUNCATE,"%s%c%d",DL_SET_POWER,DELIM_2,eng_idx);
#else
	_snprintf(code,sizeof(code),"%s%c%d",DL_SET_POWER,DELIM_2,eng_idx);
#endif	
	bool flag = false;
	flag = setctrl(code, pos);
	MC_Lock m_Lock(&my_critsec);
	m_engine[eng_idx].power = pos; //go ahead and store the new value
	return flag;
}

/*! \brief returns the prop pitch postion stored in private variable
\param const int : eng_idx -- the index of the engine 
\return \b float : prop pitch position of selected engine

*/
float C_DeviceLink::Get_PropPitch(const int eng_idx)
{
	MC_Lock m_Lock(&my_critsec);
	return m_engine[eng_idx].prop_pitch;
}

/*! \brief queries the game for the current prop pitch position 
and assigns that value to private var
\param const int : eng_idx -- the index of the engine 
\return \b float : prop pitch position
*/
bool C_DeviceLink::Query_PropPitch(const int eng_idx)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_PROP_PITCH);
	if ((-1.00 > fval) || (1.00 < fval))
	{
		errmsg("queryfloat returned an error in Query_PropPitch.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_engine[eng_idx].prop_pitch = fval;
	return TRUE;
}

/*! \brief set the game prop pitch to the passed in value.
\param const int : eng_idx -- the index of the engine to set
\param float : pos -- the position to set it too. -1.00 -- +1.00
\return \b boolean
*/
bool C_DeviceLink::Set_PropPitch(const int eng_idx, float pos)
{
	char code[64];
	memset(code,NULL,sizeof(code));
#if _MSC_VER >= 1400
	_snprintf_s(code,sizeof(code),_TRUNCATE,"%s%c%d",DL_SET_PROP_PITCH,DELIM_2,eng_idx);
#else
	_snprintf(code,sizeof(code),"%s%c%d",DL_SET_PROP_PITCH,DELIM_2,eng_idx);
#endif	
	bool flag = false;
	flag = setctrl(code, pos);
	MC_Lock m_Lock(&my_critsec);
	m_engine[eng_idx].prop_pitch = pos; //go ahead and store the new value
	return flag;
}

/*! \brief returns the brakes postion stored in private variable
\return \b float : brakes position
*/
float C_DeviceLink::Get_Brakes(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_brakes;
}

/*! \brief queries the game for the current brakes position 
and assigns that value to private var
\return \b float : brakes position
*/
bool C_DeviceLink::Query_Brakes(void)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_BRAKES);
	if ((-1.00 > fval) || (1.00 < fval))
	{
		errmsg("queryfloat returned an error in Query_Brakes.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_brakes = fval;
	return TRUE;
}

/*! \brief set the game brakes to the passed in value.
\param float : pos -- the position to set it too. -1.00 -- +1.00
\return \b boolean
*/
bool C_DeviceLink::Set_Brakes(float pos)
{
	return setctrl(DL_SET_BRAKES, pos);
}

/*! \brief returns the aileron trim postion stored in private variable
\return \b float : aileron trim position
*/
float C_DeviceLink::Get_AilTrim(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_ail_trim;
}

/*! \brief queries the game for the current aileron trim position 
and assigns that value to private var
\return \b float : aileron trim position
*/
bool C_DeviceLink::Query_AilTrim(void)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_AIL_TRIM);
	if ((-1.00 > fval) || (1.00 < fval))
	{
		errmsg("queryfloat returned an error in Query_AilTrim.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_ail_trim = fval;
	return TRUE;
}

/*! \brief set the game aileron trim to the passed in value.
\param float : pos -- the position to set it too. -1.00 -- +1.00
\return \b boolean
*/
bool C_DeviceLink::Set_AilTrim(float pos)
{
	return setctrl(DL_SET_AIL_TRIM, pos);
}

/*! \brief returns the elevator trim postion stored in private variable
\return \b float : elevator trim position
*/
float C_DeviceLink::Get_ElvTrim(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_elv_trim;
}

/*! \brief queries the game for the current elevator trim position 
and assigns that value to private var
\return \b float : elevator trim position
*/
bool C_DeviceLink::Query_ElvTrim(void)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_ELV_TRIM);
	if ((-1.00 > fval) || (1.00 < fval))
	{
		errmsg("queryfloat returned an error in Query_ElvTrim.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_elv_trim = fval;
	return TRUE;
}

/*! \brief set the game elevator trim to the passed in value.
\param float : pos -- the position to set it too. -1.00 -- +1.00
\return \b boolean
*/
bool C_DeviceLink::Set_ElvTrim(float pos)
{
	return setctrl(DL_SET_ELV_TRIM, pos);
}

/*! \brief returns the rudder trim postion stored in private variable
\return \b float : rudder trim position
*/
float C_DeviceLink::Get_RudTrim(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_rudder_trim;
}

/*! \brief queries the game for the current rudder trim position 
and assigns that value to private var
\return \b float : rudder trim position
*/
bool C_DeviceLink::Query_RudTrim(void)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_RUDDER_TRIM);
	if ((-1.00 > fval) || (1.00 < fval))
	{
		errmsg("queryfloat returned an error in Query_RudTrim.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_rudder_trim = fval;
	return TRUE;
}

/*! \brief set the game rudder trim to the passed in value.
\param float : pos -- the position to set it too. -1.00 -- +1.00
\return \b boolean
*/
bool C_DeviceLink::Set_RudTrim(float pos)
{
	return setctrl(DL_SET_RUDDER_TRIM, pos);
}

/*! \brief set the game flaps to the passed in value.
\param float : pos -- the position to set it too. -1.00 -- +1.00
\return \b boolean
*/
bool C_DeviceLink::Set_Flaps(float pos)
{
	return setctrl(DL_SET_FLAPS_POS, pos);
}

/*! \brief returns the flaps postion stored in private variable
\return \b float : flaps position
*/
float C_DeviceLink::Get_Flaps(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_flaps;
}

/*! \brief queries the game for the current rudder trim position 
and assigns that value to private var
\return \b float : rudder trim position
*/
bool C_DeviceLink::Query_Flaps(void)
{
	float fval = 0.00;
	fval = queryfloat(DL_GET_FLAPS_POS);
	if ((-1.00 > fval) || (1.00 < fval))
	{
		errmsg("queryfloat returned an error in Query_Flaps.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_flaps = fval;
	return TRUE;
}

/*!  \brief set all the instrument private variables in a single query
\return \b boolean : successful setting.
*/
bool C_DeviceLink::SetAllInstruments(void)
{
	char cmd[80];
	memset(cmd, NULL, sizeof(cmd));
#if _MSC_VER >= 1400
	_snprintf_s(cmd,sizeof(cmd),_TRUNCATE,"%s",DL_ALL_INST);
#else
	_snprintf(cmd,sizeof(cmd),"%s",DL_ALL_INST);
#endif
	if (FALSE == QueryMsg(cmd))
	{
		errmsg("error in C_DeviceLink::SetAllInstruments.\n");
		return FALSE;
	} 	
	// set IAS
	float fval = 0.00;
	getval(DL_GET_IAS, &fval);

	{
	MC_Lock m_Lock(&my_critsec);
	m_ias = fval;
	}
	// set Vario
	fval = 0.00;
	getval(DL_GET_VARIO, &fval);
	{
	MC_Lock m_Lock(&my_critsec);
	m_vario = fval;
	}
	// set Slip
	fval = 0.00;
	getval(DL_GET_SLIP, &fval);
	{
	MC_Lock m_Lock(&my_critsec);
	m_slip = fval;
	}
	// set turn
	fval = 0.00;
	getval(DL_GET_TURN, &fval);
	{
	MC_Lock m_Lock(&my_critsec);
	m_turn = fval;
	}
	// set Angular Speed
	fval = 0.00;
	getval(DL_GET_ANG_SPD, &fval);
	{
	MC_Lock m_Lock(&my_critsec);
	m_ang_spd = fval;
	}
	// set Altimeter
	fval = 0.00;
	getval(DL_GET_ALT, &fval);
	{
	MC_Lock m_Lock(&my_critsec);
	m_alt = fval;
	}
	// set Azimuth
	fval = 0.00;
	getval(DL_GET_AZI, &fval);
	{
	MC_Lock m_Lock(&my_critsec);
	m_azimuth = fval;
	}
	// set Beacon Azimuth
	fval = 0.00;
	getval(DL_GET_BEACON_AZI, &fval);
	{
	MC_Lock m_Lock(&my_critsec);
	m_beacon_azimuth = fval;
	}
	// set Roll
	fval = 0.00;
	getval(DL_GET_ROLL, &fval);
	{
	MC_Lock m_Lock(&my_critsec);
	m_roll = fval;
	}
	// set Pitch
	fval = 0.00;
	getval(DL_GET_PITCH, &fval);
	{
	MC_Lock m_Lock(&my_critsec);
	m_pitch = fval;
	}
	// set Fuel
	fval = 0.00;
	getval(DL_GET_FUEL, &fval);
	{
	MC_Lock m_Lock(&my_critsec);
	m_fuel = fval;
	} 
	return TRUE;
}

/*! \brief Queries the game for the Weapon status
\param \b WeapType : {MG, CANNON, ROCKETS, BOMBS, MGCANNON}
\return \b boolean

*/
bool C_DeviceLink::Query_Weapon(WeapType weap)
{
	char* code = NULL;
	switch (weap)
	{
		case MG:
			code = DL_GET_WEAP1;
			break;
		case CANNON:
			code = DL_GET_WEAP2;
			break;
		case ROCKETS:
			code = DL_GET_WEAP3;
			break;
		case BOMBS:
			code = DL_GET_WEAP4;
			break;
		case MGCANNON:
			code = DL_GET_WEAP1_2;
			break;
		default:
			code = DL_GET_WEAP1_2;
			break;
	}
	int ival = 0;
	ival = queryint(code);
	if ((0.00 > ival) || (1.00 < ival))
	{
		errmsg("queryfloat returned an error in Query_Weapon.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_weap[weap] = ival;
	return TRUE;
}

/*! \brief query the game for the state of the airbrakes and set private var
\return \b boolean 
*/
bool C_DeviceLink::Query_Airbrakes(void)
{
	int ival = 0;
	ival = queryint(DL_GET_AIRBRK);
	if ((0.00 > ival) || (1.00 < ival))
	{
		errmsg("queryfloat returned an error in Query_Airbrakes.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_airbrakes = ival;
	return TRUE;
}
/*! \brief returns the private variable state of the airbrakes.
\return \b integer : 1 is deployed. 0 is not deployed
*/
int C_DeviceLink::Get_Airbrakes(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_airbrakes;
}
/*! \brief sets the airbrake on or off
\return \b boolean
*/
bool C_DeviceLink::Set_Airbrakes(void)
{
	return toggleswitch(DL_TOGGLE_AIRBRK);
}

/*! \brief query the game for the state of the wingfold and set private var
\return \b boolean 
*/
bool C_DeviceLink::Query_WingFold(void)
{
	int ival = 0;
	ival = queryint(DL_GET_WING_FOLD);
	if ((0.00 > ival) || (1.00 < ival))
	{
		errmsg("queryfloat returned an error in Query_WingFold.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_wingfold = ival;
	return TRUE;
}
/*! \brief returns the private variable state of the wingfold.
\return \b integer : 1 is deployed. 0 is not deployed
*/
int C_DeviceLink::Get_WingFold(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_wingfold;
}
/* \brief sets the wingfold on or off
\return \b boolean
*/
bool C_DeviceLink::Set_WingFold(void)
{
	return toggleswitch(DL_TOGGLE_WING_FOLD);
}

/*! \brief query the game for the state of the tail hook and set private var
\return \b boolean 
*/
bool C_DeviceLink::Query_TailHook(void)
{
	int ival = 0;
	ival = queryint(DL_GET_HOOK);
	if ((0.00 > ival) || (1.00 < ival))
	{
		errmsg("queryfloat returned an error in Query_TailHook.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_tailhook = ival;
	return TRUE;
}
/*! \brief returns the private variable state of the tail hook.
\return \b integer : 1 is deployed. 0 is not deployed
*/
int C_DeviceLink::Get_TailHook(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_tailhook;
}
/*! \brief sets the tail hook on or off
\return \b boolean
*/
bool C_DeviceLink::Set_TailHook(void)
{
	return toggleswitch(DL_TOGGLE_HOOK);
}

/*! \brief query the game for the state of the chocks and set private var
\return \b boolean 
*/
bool C_DeviceLink::Query_Chocks(void)
{
	int ival = 0;
	ival = queryint(DL_GET_CHOCKS);
	if ((0.00 > ival) || (1.00 < ival))
	{
		errmsg("queryfloat returned an error in Query_Chocks.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_chocks = ival;
	return TRUE;
}
/*! \brief returns the private variable state of the chocks.
\return \b integer : 1 is deployed. 0 is not deployed
*/
int C_DeviceLink::Get_Chocks(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_chocks;
}
/* \brief sets the chocks on or off
\return \b boolean
*/
bool C_DeviceLink::Set_Chocks(void)
{
	return toggleswitch(DL_TOGGLE_CHOCKS);
}
/*! \brief query the game for the state of the canopy and set private var
\return \b boolean 
*/
bool C_DeviceLink::Query_Canopy(void)
{
	int ival = 0;
	ival = queryint(DL_GET_CANOPY);
	if ((0.00 > ival) || (1.00 < ival))
	{
		errmsg("queryfloat returned an error in Query_Canopy.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_canopy = ival;
	return TRUE;
}
/*! \brief returns the private variable state of the canopy.
\return \b integer : 1 is deployed. 0 is not deployed
*/
int C_DeviceLink::Get_Canopy(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_canopy;
}
/* \brief sets the canopy open or close
\return \b boolean
*/
bool C_DeviceLink::Set_Canopy(void)
{
	return toggleswitch(DL_TOGGLE_CANOPY);
}


/*! \brief query the game server for the status of the gunner firing
\return \b boolean

\note 0 means not firing, 1 means firing
*/
bool C_DeviceLink::Query_Gunner(void)
{
	int ival = 0;
	ival = queryint(DL_GET_GUNNER);
	if ((0 > ival) || (1 < ival))
	{
		errmsg("queryfloat returned an error in Query_Gunner.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_gunner = ival;
	return TRUE;
}
/*! \brief return the private variable status of the Gunner firing
\return \b integer : 0 mean not firing and 1 means firing

*/
int C_DeviceLink::Get_Gunner(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_gunner;
}
/*! \brief set the gunner to fire or not fire
\param const int code : code must be either DL_START_GUNNER or DL_STOP_GUNNER
\return \b boolean

*/
bool C_DeviceLink::Set_Gunner(const char* code)
{
	return toggleswitch(code);
}

/*! \brief Jump to the passed in cockpit pit
\param \b int : the number of the cockpit
\return \b boollean
*/
bool C_DeviceLink::JumpToCockpit(const int pitnum)
{
	switch (pitnum)
	{
	case 1:
		return toggleswitch(DL_JUMP_TO_PILOT);
		break;
	case 2:
		return toggleswitch(DL_JUMP_TO_POS2);
		break;
	case 3:
		return toggleswitch(DL_JUMP_TO_POS3);
		break;
	case 4:
		return toggleswitch(DL_JUMP_TO_POS4);
		break;
	case 5:
		return toggleswitch(DL_JUMP_TO_POS5);
		break;
	case 6:
		return toggleswitch(DL_JUMP_TO_POS6);
		break;
	case 7:
		return toggleswitch(DL_JUMP_TO_POS7);
		break;
	case 8:
		return toggleswitch(DL_JUMP_TO_POS8);
		break;
	case 9:
		return toggleswitch(DL_JUMP_TO_POS9);
		break;
	case 10:
		return toggleswitch(DL_JUMP_TO_POS10);
		break;
	default:
		return false;
	}
}

/*! \brief query the game for the state of the tailwheel and set private var
\return \b boolean 
*/
bool C_DeviceLink::Query_Tailwheel(void)
{
	int ival = 0;
	ival = queryint(DL_GET_TAILWHEEL);
	if ((0.00 > ival) || (1.00 < ival))
	{
		errmsg("queryfloat returned an error in Query_Tailwheel.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_tailwheel = ival;
	return TRUE;
}
/*! \brief returns the private variable state of the tailwheel.
\return \b integer : 1 is deployed. 0 is not deployed
*/
int C_DeviceLink::Get_Tailwheel(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_tailwheel;
}
/* \brief sets the tailwheel on or off
\return \b boolean
*/
bool C_DeviceLink::Set_Tailwheel(void)
{
	return toggleswitch(DL_TOGGLE_TAILWHEEL);
}

/*! \brief returns the currently stored value for the weapon type passed in
\param \b WeapType weap : {MG, CANNON, ROCKETS, BOMBS, MGCANNON}
\return \b integer : status of the weapon. 1 means it is firing and 0 means it is not.
*/
int C_DeviceLink::Get_Weapon(WeapType weap)
{
		MC_Lock m_Lock(&my_critsec);
		return m_weap[weap];
}

/*! \brief increments the cowl flaps
\return \b boolean
\note There is no query or get for this because the game doesn't allow you to set
a specific cowl flap setting. It only increments forward through the options. Hopefully, this
will be changed.
*/
bool C_DeviceLink::Set_CowlFlaps(void)
{
	return toggleswitch(DL_SET_COWL_FLAPS);
}
/*! \brief returns the currently stored value for the weapon type passed in
\param \b WeapType weap : {MG, CANNON, ROCKETS, BOMBS, MGCANNON}
\param \b int ival : the value to send to the server. 0 or 1
\return \b boolean 
*/
bool C_DeviceLink::Set_Weapon(WeapType weap, int ival)
{
	char* code = NULL;
	switch (weap)
	{
		case MG:
			code = DL_GET_WEAP1;
			break;
		case CANNON:
			code = DL_GET_WEAP2;
			break;
		case ROCKETS:
			code = DL_GET_WEAP3;
			break;
		case BOMBS:
			code = DL_GET_WEAP4;
			break;
		case MGCANNON:
			code = DL_GET_WEAP1_2;
			break;
		default:
			code = DL_GET_WEAP1_2;
			break;
	}
	char tmp_cmd[32];
	memset(tmp_cmd,NULL,sizeof(tmp_cmd));
#if _MSC_VER >= 1400
	_snprintf_s(tmp_cmd,sizeof(tmp_cmd),_TRUNCATE,"%s%c%d",code,DELIM_2,ival);
#else
	_snprintf(tmp_cmd,sizeof(tmp_cmd),"%s%c%d",code,DELIM_2,ival);
#endif
	if (set_command_buff(tmp_cmd) == FALSE)
	{
		errmsg("set_command_buff returned FALSE in Set_Weapon.\n");
		return FALSE;
	}
	if (SendMsg()== TRUE)
	{
		return TRUE;
	} else
	{
		return FALSE;
	}
}

/*! \brief query the game for the state of the level stabilizer and set private var
\return \b boolean 
*/
bool C_DeviceLink::Query_LvlStab(void)
{
	int ival = 0;
	ival = queryint(DL_GET_LVL_STAB);
	if ((0.00 > ival) || (1.00 < ival))
	{
		errmsg("queryfloat returned an error in Query_LvlStab.\n");
		return FALSE;
	} 
	MC_Lock m_Lock(&my_critsec);
	m_lvlstab = ival;
	return TRUE;
}
/*! \brief returns the private variable state of the level stabilizer.
\return \b integer : 1 is deployed. 0 is not deployed
*/
int C_DeviceLink::Get_LvlStab(void)
{
	MC_Lock m_Lock(&my_critsec);
	return m_lvlstab;
}
/*! \brief sets the level stabilizer on or off
\return \b boolean
*/
bool C_DeviceLink::Set_LvlStab(void)
{
	return toggleswitch(DL_TOGGLE_STAB);
}