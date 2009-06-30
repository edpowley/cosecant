#pragma once

class LadspaError : public std::exception
{
public:
	LadspaError(const char* msg) : std::exception(msg) {}
};

/////////////////////////////////////////////////////////////////////////////

class LadspaDll
{
public:
	LadspaDll();
	~LadspaDll();

	void init(const wchar_t* filename);

	const LADSPA_Descriptor* callDescriptorFunc(unsigned long index);

protected:
	HMODULE m_hDll;
	LADSPA_Descriptor_Function m_descFunc;
};

/////////////////////////////////////////////////////////////////////////////

class LadspaMachine : public Mi
{
public:
	LadspaMachine(HostMachine* mac, Callbacks* cb, const std::wstring& dllname, int index);
	~LadspaMachine();

	void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe);
	void changeParam(ParamTag tag, ParamValue value);

protected:
	LadspaDll m_dll;
	const LADSPA_Descriptor* m_desc;
	LADSPA_Handle m_handle;

	std::vector< std::vector<float> > m_portBuffers;
	std::map<ParamTag, float*> m_paramBuffers;
	std::vector<float*> m_inPinBuffers, m_outPinBuffers;
	std::vector<bool> m_outPinIsControl;
};

/////////////////////////////////////////////////////////////////////////////

class LadspaMachineFactory : public MiFactory
{
public:
	LadspaMachineFactory(const std::wstring& dllname, int index, const LADSPA_Descriptor* desc)
		: m_dllname(dllname), m_index(index), m_ladspaId(desc->UniqueID) {}

	virtual bool getInfo(MachineInfo* info, const InfoCallbacks* cb);
	
	virtual Mi* createMachine(HostMachine* mac, Callbacks* cb)
	{	return new LadspaMachine(mac, cb, m_dllname, m_index);   }

protected:
	std::wstring m_dllname;
	int m_index;
	unsigned long m_ladspaId;
};
