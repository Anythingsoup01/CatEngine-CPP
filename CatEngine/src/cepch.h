#pragma once

// TODO We need platform detection for some of these to be included
#include "CatEngine/Core/PlatformDetection.h"


#ifdef CE_PLATFORM_WINDOWS
#ifndef NOMINMAX
// See github.com/skypjack/entt/wiki/Frequently-Asked-Questions#warning-c4003-the-min-the-max-and-the-macro
#define NOMINMAX
#endif
#endif

#ifdef CE_PLATFORM_LINUX

#include <signal.h> // IWYU pragma: keep

#endif 

#include <iostream> // IWYU pragma: keep
#include <memory> // IWYU pragma: keep
#include <utility> // IWYU pragma: keep
#include <algorithm> // IWYU pragma: keep
#include <functional> // IWYU pragma: keep
#include <filesystem> // IWYU pragma: keep

#include <string> // IWYU pragma: keep
#include <sstream> // IWYU pragma: keep
#include <array> // IWYU pragma: keep
#include <vector> // IWYU pragma: keep
#include <unordered_map> // IWYU pragma: keep
#include <unordered_set> // IWYU pragma: keep

#include "CatEngine/Core/Core.h" // IWYU pragma: keep
#include "CatEngine/Core/Log.h" // IWYU pragma: keep
#include "CatEngine/Core/Buffer.h" // IWYU pragma: keep
#ifndef CE_SCRIPT_COMPILATION
#include "CatEngine/Core/Utils.h" // IWYU pragma: keep
#include "CatEngine/Debug/Instrumentor.h" // IWYU pragma: keep
#endif

#ifdef CE_PLATFORM_WINDOWS
#include <Windows.h> // IWYU pragma: keep
#endif
