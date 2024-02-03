#pragma once

#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/JobSystem.hpp"

#define UNUSED(x) (void)(x);

//----------------------------------------------------------------------------------------------------------------------
class NamedStrings;

//----------------------------------------------------------------------------------------------------------------------
extern NamedStrings g_gameConfigBlackboard;
extern EventSystem* g_theEventSystem;
extern JobSystem*   g_theJobSystem;