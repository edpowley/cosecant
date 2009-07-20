#pragma once

class Gain : public Mi
{
public:
	static bool getInfo(MachineInfo* info, InfoCallbacks* cb);

	Gain(Callbacks* cb);

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
	static bool getInfo(MachineInfo* info, InfoCallbacks* cb);

	MonoMonoToStereo(Callbacks* cb) : Mi(cb) {}

	virtual void changeParam(ParamTag tag, ParamValue value) {}
	virtual void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe);
};

class StereoToMonoMono : public Mi
{
public:
	static bool getInfo(MachineInfo* info, InfoCallbacks* cb);

	StereoToMonoMono(Callbacks* cb) : Mi(cb) {}

	virtual void changeParam(ParamTag tag, ParamValue value) {}
	virtual void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe);
};

