#pragma once

class Dll : public Object
{
public:
	static bool isDll(const QString& path);

	ERROR_CLASS(InitError);

	Dll(const QString& path);
	virtual ~Dll();

	void* getFunc(const char* name);

protected:
	HMODULE m_hmod;
};
