#pragma once

// C runtime library
#include <stdio.h>

#define _USE_MATH_DEFINES
#include <math.h>

// Windows
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define NOMINMAX				// don't define min/max macros
#include <windows.h>
#include <tchar.h>
#include <direct.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <commdlg.h>

// STL
#include <vector>
#include <map>
#include <set>
#include <iomanip> // std::setprecision etc

// Boost
#include <boost/thread.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
typedef boost::filesystem::wpath bpath;

// libxml++
// TODO: remove dependency, and remove associated include dir from project settings
#include <libxml++/libxml++.h>

// Qt
#include <QtGui>
#include <QtXml>
#include <QtScript>

// PortAudio
#include <portaudio.h>
#include <pa_asio.h>

// sqlite
#include "sqlite/sqlite3.h"

// Mine
#define COSECANT_API_HOST
