#pragma once

class Gain : public Mi
{
public:
	static bool getInfo(MachineInfo* info, const InfoCallbacks* cb);

	Gain(HostMachine* mac, Callbacks* cb);

	virtual void changeParam(ParamTag tag, ParamValue value);
	virtual void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe);

protected:
	float m_gain;
	float m_pan;
};

///////////////////////////////////////////////////////////////////////////

class MonoMonoToStereo : public Mi
{
public:
	static bool getInfo(MachineInfo* info, const InfoCallbacks* cb);

	MonoMonoToStereo(HostMachine* mac, Callbacks* cb) : Mi(mac,cb) {}

	virtual void changeParam(ParamTag tag, ParamValue value) {}
	virtual void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe);
};

class StereoToMonoMono : public Mi
{
public:
	static bool getInfo(MachineInfo* info, const InfoCallbacks* cb);

	StereoToMonoMono(HostMachine* mac, Callbacks* cb) : Mi(mac,cb) {}

	virtual void changeParam(ParamTag tag, ParamValue value) {}
	virtual void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe);
};

