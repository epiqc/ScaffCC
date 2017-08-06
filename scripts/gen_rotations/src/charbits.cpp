#include "charbits.h"
#include <iostream> // std::cout std::endl


charBits::charBits() {
	buffer = new charBits::Code();
	(*buffer).push_back(0x00);

	// mask = 0000 1000, where the first four bits are reserved
	// to indicate how many unused tailing bits in the last byte
	mask = 0x08; // indicate next available slot
}

charBits::charBits(const Code& old) {
	buffer = new charBits::Code();
	Code::const_iterator ci = old.begin();
	(*buffer).push_back(*ci);
	unsigned char tmp = *ci;
	tmp >>= 4;
	if (tmp == 0) {
		mask = 0;
	} else {
		mask = 0x01 << (tmp - 1);
	}
	for (ci = ci + 1; ci != old.end(); ci++) {
		(*buffer).push_back(*ci);
	}
}

charBits::~charBits() {
	if (buffer)
		delete buffer;
}

inline void charBits::alloc_byte() {
	if (mask == 0) {
		(*buffer).push_back(0x00);
		mask = 0x80; // 1000 0000
	}
}

void charBits::read_bits(std::string *output) {
	std::stringstream bss;
	Code::iterator ci;
	for (ci = (*buffer).begin(); ci != (*buffer).end(); ci++) {
		unsigned char start;
		if (ci == (*buffer).begin()) {
			start = 0x08; // 0000 1000
		} else {
			start = 0x80; // 1000 0000
		}
		unsigned char stop;
		if (ci == (*buffer).end() - 1) {
			stop = mask;
		} else {
			stop = 0;
		}
		unsigned char read_mask;
		for (read_mask = start; read_mask != stop; read_mask >>= 1) {
			bss << (((*ci & read_mask) != 0)? "1" : "0");
		}
			
	}
	*output = bss.str();
	return;
}

void charBits::write_bits(const std::string bits, int num) {
	for (int i = 0; i < num; i++) {
		alloc_byte(); // if necessary
		if (bits[i] == '1') { // write 1 to last byte
			(*buffer).back() |= mask;
		}
		mask >>= 1;
	}
}

charBits::Code *charBits::flush_code() {
	// determine number of unused bit
	unsigned char unused = 0;
	for (; mask != 0; mask >>= 1) {
		unused++;
	}
	// write to first byte of buffer vector
	unused <<= 4;//
	(*buffer).front() |= unused;
	return buffer;
}
