///////////////////////////////////////////////////////////////////////////////
// Headers
///////////////////////////////////////////////////////////////////////////////
#include "Logger.h"

#include <assert.h>

ILogger* ILogger::myStaticInstance = NULL;
ILogger::LogLevel ILogger::myCurrentLogLevel = ILogger::INFORMATIVE;
ILogger::LogLevel ILogger::myLogThreshold = ILogger::INFORMATIVE;

/////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////
void ILogger::setLogger(ILogger* Logger)
{
    destroy();
    myStaticInstance = Logger;
}


/////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////
void ILogger::destroy()
{
    delete myStaticInstance;
    myStaticInstance = NULL;
}

ILogger::~ILogger(void)
{
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////
ILogger& ILogger::internalLog()
{
	if(myStaticInstance == NULL)
	{
		setLogger(new FileLogger);
	}

	return *myStaticInstance;
}

//=============================================================================
void ILogger::log(const char* Format, ...)
{
	assert(strlen(Format) < 512 && "Log : String size exceeds 512 characters");

    // Format the message in a buffer
    char sBuffer[512];
    va_list Params;
    va_start(Params, Format);
    #if (defined(WIN32) || defined(__WIN32__) || defined(_WIN32))
        _vsnprintf(sBuffer, 10, Format, Params);
    #else
        vsnprintf(sBuffer, 10, Format, Params);
    #endif
    va_end(Params);

    internalLog().write(sBuffer);
}

//=============================================================================
ILogger& ILogger::log(LogLevel n)
{
	myCurrentLogLevel = n;

	if(myCurrentLogLevel <= myLogThreshold)
	{
		internalLog().write(thresholdToString());
	}

    return internalLog();
}

////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////
std::string ILogger::currentDate() const
{
	struct tm* timeStruct;

    // Récupération et formatage de la date
    char sTime[24];
    time_t theTime = time(NULL);
//    #if (defined(WIN32) || defined(__WIN32__) || defined(_WIN32))
//        localtime_s(&tempsActuel, &tempsStocke);
//    #else
        timeStruct = localtime(&theTime);
//    #endif
    strftime(sTime, sizeof(sTime), "%d/%m/%Y", timeStruct);

    return sTime;
}


////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////
std::string ILogger::currentTime() const
{
	struct tm* currentTime;

    char sTime[24];
    //time_t theTime = time(NULL);
    time_t theTime = time(NULL);
//    #if (defined(WIN32) || defined(__WIN32__) || defined(_WIN32))
//        localtime_s(&currentTime, &theTime);
//    #else
        currentTime = localtime(&theTime);
//    #endif
    strftime(sTime, sizeof(sTime), "%H:%M:%S", currentTime);

    return sTime;
}

std::string ILogger::thresholdToString()
{
	switch(myCurrentLogLevel)
	{
	case ERRORS			: return "[Error] -> ";
	case INFORMATIVE	: return "[Information] -> ";
	case ALL			: return "[Note] -> ";
	default				: return "";
	}
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
// FileLogger
///////////////////////////////////////////////////////////////////////////////
//=============================================================================
FileLogger::FileLogger(std::string name, bool add)
:ILogger()
,myFile(name.c_str(), (add ? std::ios_base::app : (std::ios_base::out | std::ios_base::trunc)) )
{
	myFile	   << "===============================" << std::endl
			   << " Starting logs...	 "			<< currentDate()
			   << " à " << currentTime()			<< std::endl
			   << "===============================" << std::endl << std::endl;
}

FileLogger::~FileLogger()
{
	myFile.close();
}

void FileLogger::write(const std::string& m)
{
	myFile << m;
	myFile.flush();
}

//=============================================================================
///////////////////////////////////////////////////////////////////////////////
ConsoleLog::ConsoleLog(std::ostream& o)
:myStream(o)
{
}

void ConsoleLog::write(const std::string& m)
{
	myStream << m;
}