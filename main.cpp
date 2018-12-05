#include "stdafx.h"
#include "common.h"
#include "application.h"
#include "version.h"

int main(int argc, char *argv[])
{
	try
	{
		Application& a = Application::initSingleton(argc, argv);
		
		return a.exec();
	}
	catch (const std::exception& err)
	{
		std::ostringstream ss;
		ss << "A fatal error occurred. This is probably a bug, in Cosecant or one of its machines.\n";
		ss << "You can press Ctrl+C to copy this text to the clipboard.\n\n";
		ss << err.what() << "\n\n";
		ss << "Please click OK. ";
		ss << "(If you are a developer and you want to rethrow the exception for debugging, click Cancel.)";

		if (MessageBoxA(NULL, ss.str().c_str(), "BTDSys Cosecant", MB_OKCANCEL | MB_ICONERROR) == IDCANCEL)
			throw;

		return 1;
	}
}
