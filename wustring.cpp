#include "stdafx.h"
#include "common.h"
#include "wustring.h"

Glib::ustring wstring_to_ustring(const std::wstring& str)
{
	int nchars = WideCharToMultiByte(CP_UTF8,0, str.c_str(),-1, NULL,0, 0,0);
	std::vector<char> buf; buf.resize(nchars);
	WideCharToMultiByte(CP_UTF8,0, str.c_str(),-1, &buf[0],nchars, 0,0);
	return Glib::ustring(&buf[0]);
}

std::wstring ustring_to_wstring(const Glib::ustring& str)
{
	int nchars = MultiByteToWideChar(CP_UTF8,0, str.c_str(),-1, NULL,0);
	std::vector<wchar_t> buf; buf.resize(nchars);
	MultiByteToWideChar(CP_UTF8,0, str.c_str(),-1, &buf[0],nchars);
	return std::wstring(&buf[0]);

}
