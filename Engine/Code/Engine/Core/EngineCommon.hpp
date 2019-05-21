#pragma once
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/StringId.hpp"
#include "Engine/Core/type.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/NamedFunctions.hpp"
#include "Game/EngineBuildPreferences.hpp"
#include <memory>
#include <bitset>

// Source from http://www.flipcode.com/archives/FIXME_TODO_Notes_As_Warnings_In_Compiler_Output.shtml
//---------------------------------------------------------------------------------------------
// FIXMEs / TODOs / NOTE macros
//---------------------------------------------------------------------------------------------
#define _QUOTE(x) # x
#define QUOTE(x) _QUOTE(x)
#define __FILE__LINE__ __FILE__ "(" QUOTE(__LINE__) ") : "

#define NOTE( x )  message( x )
#define FILE_LINE  message( __FILE__LINE__ )

#define TODO( x )  message( __FILE__LINE__"\n"           \
        " ------------------------------------------------\n" \
        "|  TODO :   " ##x "\n" \
        " -------------------------------------------------\n" )
#define FIXME( x )  message(  __FILE__LINE__"\n"           \
        " ------------------------------------------------\n" \
        "|  FIXME :  " ##x "\n" \
        " -------------------------------------------------\n" )
#define todo( x )  message( __FILE__LINE__" TODO :   " ##x "\n" ) 
#define fixme( x )  message( __FILE__LINE__" FIXME:   " ##x "\n" ) 

#define UNIMPLEMENTED()  TODO( "IMPLEMENT: " QUOTE(__FILE__) " (" QUOTE(__LINE__) ")" ); ASSERT(0) 
#define UNUSED(x) (void)(x)
#define SAFE_DELETE( x ) {if(x){delete (x);(x)=nullptr;}}
#define SAFE_DELETE_ARRAY( x ) {if(x){delete[] (x);(x)=nullptr;}}
#define WITHIN_RANGE(val, left, right) (left <= val && val <= right)

#define PROFILER_ENABLED

constexpr size_t MAX_WINDOWS = 2;
using namespace std::string_literals;

class RHIInstance;
class InputSystem;
class AudioSystem;
class Profiler;
class Logger;
class ResourceManager;


extern Uptr<RHIInstance>		g_theRHI;
extern Uptr<InputSystem>		g_theInput;
extern Uptr<AudioSystem>		g_theAudio;
extern Uptr<Profiler>			g_theProfiler;
extern Uptr<Logger>				g_theLogger;
extern Uptr<ResourceManager>	g_theResourceManager;

extern NamedFunctions			g_functionLibrary;

#pragma warning( disable : 4267)
#pragma warning( disable : 4006)


