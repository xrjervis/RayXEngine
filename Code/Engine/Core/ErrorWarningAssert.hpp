#pragma once
//-----------------------------------------------------------------------------------------------
// ErrorWarningAssert.hpp
//
// Summary of error and assertion macros:
//	#define ERROR_AND_DIE( errorText )						// "MUST not reach this point"; Show error dialogue, then shut down the app
//	#define ERROR_RECOVERABLE( errorText )					// "SHOULD not reach this point"; Show warning dialogue, then proceed
//	#define GUARANTEE_OR_DIE( condition, errorText )		// "MUST be true"; If condition is false, show error dialogue then shut down the app
//	#define GUARANTEE_RECOVERABLE( condition, errorText )	// "SHOULD be true"; If condition is false, show warning dialogue then proceed
//	#define ASSERT_OR_DIE( condition, errorText )			// Same as GUARANTEE_OR_DIE, but removed if DISABLE_ASSERTS is #defined
//	#define ASSERT_RECOVERABLE( condition, errorText )		// Same as GUARANTEE_RECOVERABLE, but removed if DISABLE_ASSERTS is #defined
// 


//-----------------------------------------------------------------------------------------------
#include <string>


//-----------------------------------------------------------------------------------------------
enum SeverityLevel
{
	SEVERITY_INFORMATION,
	SEVERITY_QUESTION,
	SEVERITY_WARNING,
	SEVERITY_FATAL
};


//-----------------------------------------------------------------------------------------------
void DebuggerPrintf( const char* messageFormat, ... );
bool IsDebuggerAvailable();
__declspec( noreturn ) void FatalError( const char* filePath, const char* functionName, int lineNum, const std::string& reasonForError, const char* conditionText=nullptr );
void RecoverableWarning( const char* filePath, const char* functionName, int lineNum, const std::string& reasonForWarning, const char* conditionText=nullptr );
void SystemDialogue_Okay( const std::string& messageTitle, const std::string& messageText, SeverityLevel severity );
bool SystemDialogue_OkayCancel( const std::string& messageTitle, const std::string& messageText, SeverityLevel severity );
bool SystemDialogue_YesNo( const std::string& messageTitle, const std::string& messageText, SeverityLevel severity );
int SystemDialogue_YesNoCancel( const std::string& messageTitle, const std::string& messageText, SeverityLevel severity );



//-----------------------------------------------------------------------------------------------
// ERROR_AND_DIE
//
// Present in all builds.
// No condition; always triggers if reached.
// Depending on the platform, this typically:
//	- Logs an error message to the console and/or log file
//	- Opens an error/message dialogue box
//	- Triggers a debug breakpoint (if appropriate development suite is present)
//	- Shuts down the app
//
// Use this when reaching a certain line of code should never happen under any circumstances,
// and continued execution is dangerous or impossible.
//
#define ERROR_AND_DIE( errorMessageText )															\
{																									\
	FatalError( __FILE__,  __FUNCTION__, __LINE__, errorMessageText );								\
}


//-----------------------------------------------------------------------------------------------
// ERROR_RECOVERABLE
//
// Present in all builds.
// No condition; always triggers if reached.
// Depending on the platform, this typically:
//	- Logs a warning message to the console and/or log file
//	- Opens an warning/message dialogue box
//	- Triggers a debug breakpoint (if appropriate development suite is present)
//	- Continues execution
//
#define ERROR_RECOVERABLE( errorMessageText )														\
{																									\
	RecoverableWarning( __FILE__,  __FUNCTION__, __LINE__, errorMessageText );						\
}


//-----------------------------------------------------------------------------------------------
// GUARANTEE_OR_DIE
//
// Present in all builds.
// Triggers if condition is false.
// Depending on the platform, this typically:
//	- Logs an error message to the console and/or log file
//	- Opens an error/message dialogue box
//	- Triggers a debug breakpoint (if appropriate development suite is present)
//	- Shuts down the app
//
#define GUARANTEE_OR_DIE( condition, errorMessageText )												\
{																									\
	if( !(condition) )																				\
	{																								\
		const char* conditionText = #condition;														\
		FatalError( __FILE__,  __FUNCTION__, __LINE__, errorMessageText, conditionText );			\
	}																								\
}


//-----------------------------------------------------------------------------------------------
// GUARANTEE_RECOVERABLE
//
// Present in all builds.
// Triggers if condition is false.
// Depending on the platform, this typically:
//	- Logs a warning message to the console and/or log file
//	- Opens an warning/message dialogue box
//	- Triggers a debug breakpoint (if appropriate development suite is present)
//	- Continues execution
//
#define GUARANTEE_RECOVERABLE( condition, errorMessageText )										\
{																									\
	if( !(condition) )																				\
	{																								\
		const char* conditionText = #condition;														\
		RecoverableWarning( __FILE__,  __FUNCTION__, __LINE__, errorMessageText, conditionText );	\
	}																								\
}


//-----------------------------------------------------------------------------------------------
// ASSERT_OR_DIE
//
// Removed if DISABLE_ASSERTS is defined, typically in a Final build configuration.
// Triggers if condition is false.
// Depending on the platform, this typically:
//	- Logs an error message to the console and/or log file
//	- Opens an error/message dialogue box
//	- Triggers a debug breakpoint (if appropriate development suite is present)
//	- Shuts down the app
//
#if defined( DISABLE_ASSERTS )
#define ASSERT_OR_DIE( condition, errorMessageText ) { (void)( condition ); }
#else
#define ASSERT_OR_DIE( condition, errorMessageText )												\
{																									\
	if( !(condition) )																				\
	{																								\
		const char* conditionText = #condition;														\
		FatalError( __FILE__,  __FUNCTION__, __LINE__, errorMessageText, conditionText );			\
	}																								\
}
#endif


//-----------------------------------------------------------------------------------------------
// ASSERT_RECOVERABLE
//
// Removed if DISABLE_ASSERTS is defined, typically in a Final build configuration.
// Triggers if condition is false.
// Depending on the platform, this typically:
//	- Logs a warning message to the console and/or log file
//	- Opens an warning/message dialogue box
//	- Triggers a debug breakpoint (if appropriate development suite is present)
//	- Continues execution
//
#if defined( DISABLE_ASSERTS )
#define ASSERT_RECOVERABLE( condition, errorMessageText ) { (void)( condition ); }
#else
#define ASSERT_RECOVERABLE( condition, errorMessageText )											\
{																									\
	if( !(condition) )																				\
	{																								\
		const char* conditionText = #condition;														\
		RecoverableWarning( __FILE__,  __FUNCTION__, __LINE__, errorMessageText, conditionText );	\
	}																								\
}
#endif


