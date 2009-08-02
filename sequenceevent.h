#pragma once

#include "cosecant_api.h"
#include "sequence.h"

namespace SequenceEvent
{
	class Base : public Object
	{
	public:
		virtual void work(const Ptr<Machine>& mac) = 0;
	};
};
