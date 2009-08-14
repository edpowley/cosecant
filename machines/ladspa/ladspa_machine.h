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
	enum { ptPluginFirst = 0x10000, };

	LadspaMachine(HostMachine* hm, const std::wstring& dllname, int index);
	~LadspaMachine();

	MachineInfo* getInfo();

	void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe);
	void changeParam(ParamTag tag, double value);

protected:
	LadspaDll m_dll;
	const LADSPA_Descriptor* m_desc;
	LADSPA_Handle m_handle;
	
	MachineInfo m_info;
	std::list<RealParamInfo> m_realParams;
	std::list<IntParamInfo> m_intParams;
	std::list<TimeParamInfo> m_timeParams;
	std::vector<ParamInfo*> m_paramPtrs;
	std::list<PinInfo> m_pins;
	std::vector<PinInfo*> m_inPinPtrs, m_outPinPtrs;

	std::vector< std::vector<float> > m_portBuffers;
	std::map<ParamTag, float*> m_paramBuffers;
	std::vector<float*> m_inPinBuffers, m_outPinBuffers;
	std::vector<bool> m_outPinIsControl;
};
