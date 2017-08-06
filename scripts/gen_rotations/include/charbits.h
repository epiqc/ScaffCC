// CHAR_BITS defines the operations on bits
#ifndef _CHAR_BITS_H_
#define _CHAR_BITS_H_

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

class charBits {
public:
	typedef std::vector<unsigned char> Code;
	
	charBits();
	charBits(const Code& old);

	// Destroy
	~charBits();

	Code *buffer;

	void read_bits(std::string *output);

	void write_bits(const std::string bits, int num);

	Code *flush_code();

private:
	unsigned char mask;
	inline void alloc_byte();

};

#endif // _CHAR_BITS_H_
