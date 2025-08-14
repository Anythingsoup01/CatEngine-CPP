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

#include <signal.h>

#endif 

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <filesystem>

#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "CatEngine/Core/Core.h"
#include "CatEngine/Core/Log.h"
#include "CatEngine/Core/Utils.h"
#include "CatEngine/Debug/Instrumentor.h"

#ifdef CE_PLATFORM_WINDOWS
#include <Windows.h>
#endif
