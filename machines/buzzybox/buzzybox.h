#pragma once

class BuzzyMi : public Mi
{
public:
	BuzzyMi(HostMachine* hm) : Mi(hm) {}
};

///////////////////////////////////////////////////////////////////////////

class Gain : public BuzzyMi
{
public:
	enum { ptGain, ptPan };

	Gain(HostMachine* hm) : BuzzyMi(hm), m_gain(1), m_pan(1) {}

	MachineInfo* getInfo();

	void changeParam(ParamTag tag, double value);
	void work(const PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe);

protected:
	float m_gain;
	float m_pan;
};

///////////////////////////////////////////////////////////////////////////

class MonoMonoToStereo : public BuzzyMi
{
public:
	MonoMonoToStereo(HostMachine* hm) : BuzzyMi(hm) {}

	MachineInfo* getInfo();

	void changeParam(ParamTag tag, double value) {}
	void work(const PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe);
};

class StereoToMonoMono : public BuzzyMi
{
public:
	StereoToMonoMono(HostMachine* hm) : BuzzyMi(hm) {}

	MachineInfo* getInfo();

	void changeParam(ParamTag tag, double value) {}
	void work(const PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe);
};

