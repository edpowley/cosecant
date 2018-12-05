#pragma once

// C runtime library
#include <stdio.h>
#include <float.h>

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
#undef small	// defined in RpcNdr.h

// STL
#include <vector>
#include <map>
#include <set>
#include <iomanip> // std::setprecision etc
#include <sstream>
#include <functional>

// Boost
//#include <boost/foreach.hpp>
//#include <boost/function.hpp>
//#include <boost/bind.hpp>
//#include <boost/random.hpp>

#define BOOST_FOREACH(a,b)  for (a : b)

// Qt
#include <QtCore>
#include <QtGui>
#include <QDomElement>
#include <QGridLayout>
//#include <QtNetwork>

// PortAudio
#include <portaudio.h>
#include <pa_asio.h>

// sqlite
#include "sqlite/sqlite3.h"

// zlib / minizip
#include <zip.h>
#include <unzip.h>

// Mine
#define COSECANT_API_HOST
