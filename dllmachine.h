#pragma once

#include "machine.h"
#include "dll.h"

namespace DllMachine
{
	ERROR_CLASS(DllMachineError);

	class Factory : public MachineFactory
	{
	public:
		static void scan(const QString& path);

		Factory(const QString& dllpath, const QString& id, const QString& desc, void* user, unsigned int userSize);

	protected:
		QString m_dllpath, m_id, m_desc;
		std::vector<char> m_userData;

		virtual Ptr<Machine> createMachineImpl();
	};

	class MacDll : public Dll
	{
	public:
		MacDll(const QString& path);
		PluginFunctions* m_funcs;
	};

	class Mac : public Machine
	{
	public:
		Mac(const QString& dllpath, const std::vector<char>& user);
		virtual ~Mac();

		Ptr<MacDll> getDll() { return m_dll; }
		Mi* getMi() { return m_mi; }

		virtual void changeParam(ParamTag tag, double value);
		virtual void work(PinBuffer*, PinBuffer*, int, int);

		virtual QScriptValue callScriptFunction(QScriptContext* ctx, QScriptEngine* eng, int id);

	protected:
		Ptr<MacDll> m_dll;
		Mi* m_mi;

		virtual void initInfo();
		virtual void initImpl();

		virtual Ptr<Sequence::Pattern> createPatternImpl(double length);
	};

	class Pattern : public Sequence::Pattern
	{
	public:
		Pattern(const Ptr<Mac>& mac, MiPattern* mip, double length);
		virtual ~Pattern();

	protected:
		Ptr<MacDll> m_dll;
		Mi* m_mi;
		MiPattern* m_mip;
	};
};
