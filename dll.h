#pragma once

class Dll : public Object
{
public:
	static bool isDll(const bpath& path);

	ERROR_CLASS(InitError);

	Dll(const bpath& path);
	virtual ~Dll();

	void* getFunc(const char* name);

protected:
	HMODULE m_hmod;
};
