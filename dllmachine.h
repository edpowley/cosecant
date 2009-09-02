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

		virtual QString getDesc() { return m_desc; }

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

		virtual void work(const WorkContext* ctx);

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

		MiPattern* getMiPattern() { return m_mip; }

	protected:
		Ptr<MacDll> m_dll;
		Mi* m_mi;
		MiPattern* m_mip;
	};
};
