#include "CommandList.h"

CommandList::~CommandList()
{
	basic->Release();
}
