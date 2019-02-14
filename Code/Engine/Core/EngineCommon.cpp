#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Logger.hpp"
#include "Engine/Core/ResourceManager.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/InputSystem/InputSystem.hpp"
#include "Engine/Profiler/Profiler.hpp"
#include "Engine/Renderer/d3d11/RHIInstance.hpp"

Uptr<RHIInstance>		g_theRHI;
Uptr<InputSystem>		g_theInput;
Uptr<AudioSystem>		g_theAudio;
Uptr<Profiler>			g_theProfiler;
Uptr<Logger>			g_theLogger;
Uptr<ResourceManager>	g_theResourceManager;

