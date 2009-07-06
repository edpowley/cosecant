#include "stdafx.h"
#include "common.h"
#include "machinfo.h"
using namespace InfoImpl;

void MachineInfo::addPin(CosecantAPI::PinInfo* pin, std::vector<PinInfo*>& pins)
{
	PinInfo* ipin = dynamic_cast<PinInfo*>(pin);
	if (ipin)
		pins.push_back(ipin);
	else
		THROW_ERROR(Error, "pin is not an InfoImpl::Pin");
}

ParamInfo::Enum* ParamInfo::Enum::addItems(char separator, const char* text)
{
	m_items.append(QString::fromUtf8(text).split(separator));
	return this;
}
