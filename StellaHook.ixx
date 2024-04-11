module;
#include "pch.h"

#include <vector>
#include <string>
#include <algorithm>

export module StellaHook;

using std::vector;
using std::string;
using namespace std::literals;

/*TODO: CODECAVE MEMORY MANAGEMENT
*	
*	hook creation should not involve looking for suitable empty spaces in target code pages
*	therefore, we should allocate memory for code pages ourselves
*	this creates the possibility of storing both code and data in the same page, which can be avoided using a private heap (HeapCreate)
*	private heaps are guaranteed to not share pages with the rest of the process, so we can freely mark them as execute only
*
*/
export class BasicHook
{
	static HMODULE targetBase;

	vector<BYTE> swapBuffer;
	BYTE* target;
	bool enabled;

protected:

	static constexpr BYTE parseNybble(char a) {
		switch (a)
		{
		case '0':
			return 0;
		case '1':
			return 1;
		case '2':
			return 2;
		case '3':
			return 3;
		case '4':
			return 4;
		case '5':
			return 5;
		case '6':
			return 6;
		case '7':
			return 7;
		case '8':
			return 8;
		case '9':
			return 9;
		case 'A':
		case 'a':
			return 10;
		case 'B':
		case 'b':
			return 11;
		case 'C':
		case 'c':
			return 12;
		case 'D':
		case 'd':
			return 13;
		case 'E':
		case 'e':
			return 14;
		case 'F':
		case 'f':
			return 15;
		default:
			throw std::exception("Invalid hex digit");
		}
	}

	void toggle()
	{
		DWORD oldProtect;
		VirtualProtect(target, swapBuffer.size(), PAGE_READWRITE, &oldProtect);
		std::swap_ranges(swapBuffer.begin(), swapBuffer.end(), target);
		VirtualProtect(target, swapBuffer.size(), oldProtect, &oldProtect);
	}

public:

	static HMODULE getTargetBase() { return targetBase; }
	static void setTargetBase(HMODULE base) { targetBase = base; }

	BasicHook(int targetOffset, const string& hexData) :
		target((BYTE*)targetBase + targetOffset), enabled(false)
	{
		size_t j = hexData.size();
		if (j % 2) {
			throw std::exception("Odd-sized hex data string");
		}
		for (auto i = hexData.begin(); i != hexData.end(); i += 2) {
			swapBuffer.push_back(parseNybble(*i) << 4 | parseNybble(*(i + 1)));
		}
		if (swapBuffer.size() != hexData.size() / 2) {
			throw std::exception();
		}
	}

	BasicHook(int targetOffset, BYTE byte, size_t size) :
		swapBuffer(size, byte), target((BYTE*)targetBase + targetOffset), enabled(false) {}

	void enable()
	{
		if (!enabled) {
			toggle();
			enabled = true;
		}
	}

	void disable()
	{
		if (enabled) {
			toggle();
			enabled = false;
		}
	}
};

HMODULE BasicHook::targetBase = NULL;

export class NopHook : public BasicHook
{
public:

	NopHook(int targetOffset, size_t size) : BasicHook(targetOffset, 0x90, size) {}
};