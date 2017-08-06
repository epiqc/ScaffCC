#ifndef _INC_ROTLIB_H
#define _INC_ROTLIB_H

#include <fstream>
#include <map>
#include <vector>
#include <string.h>
#include <complex>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <iomanip>
#include <math.h>

#include "precision.h"
#include "storage.h"
#include "charbits.h"
#include "Exception.h"

/*
 * This is a C++ implemetation of the library construction method 
 * for generating Rz rotations. The main features of this generator include:
 *  - Powered by gridsynth, the package can generate rotation sequences 
 *    that approximate arbitray *Rz* angles, up to given precision.
 *  - Generate libraries of rotation sequences given use-defined 
 *    precision and storage requirements, trading storage for execution time.
 *  - Dynamically concatenate rotation sequences at run time 
 *    using generated libraries.
 *
 * More concretely, when generating a rotation library for the first time, 
 * you would need to specify the following parameters as input:
 *  - The library will then consist of angles: {pi, pi/b, pi/b^2, pi/b^3, ... },
 *    until the required precision can be achieved.
 *  - *b*: basis of the angles. 
 *  - The guaranteed precision will be *k* base-*n* places. 
 *    E.g. if *n=10, k=10*, then the angles generated is guaranteed 
 *    to have precision up to 10 decimal places.
 *  - *n*: base of precision (can be 2 or 10).
 *  - *k*: number of places in base-*n* precision
 *  - For storage, you would need to provide the size and unit, such as 100 KB. 
 *  - You may also specify whether to have the rotations decomposed 
 *    up to global phase or not.
 *
 * Some important functions from RotLib class you may take advantage of are:
 *  - RotLib::generate() - which envokes the core rotation generator and 
 *    decompresses rotation sequences with Huffman encoding.
 *  - RotLib::save(filename) - which writes the encoded library in output file.
 *  - RotLib::load(filename) - which loads previously saved library from file.
 *  - RotLib::concatenate(angle[, factor]) - which automatically assembles 
 *    library angles for the desired angle, optionally with factor = "pi". 
 *    Note that angle is in RotLib::Rz type, whose members include:
 *     - angle -> theta: rotation angle in radian
 *     - angle -> gates: the decomposed rotation sequence
 *     - angle -> length: number of operations in sequence
 *
 * Yongshan (yongshan@uchicago.edu)
 *
 */

class RotLib
{
public:

	PRECISION lib_p;
	STORAGE lib_s;
	int basis;
	bool phase;

	struct Rz
	{
		int length;
		std::string gates;
		std::string theta;
		PRECISION p;
		Rz() : length(0), gates(""), theta(""), p(NULL) {}
		Rz(int _l, std::string _g, std::string _t, PRECISION _p) : length(_l), gates(_g), theta(_t), p(_p) {}
	};

	// Initialize a RotLib with given precision
	RotLib(PRECISION lib_p, STORAGE lib_s, int basis, bool phase);

	// Load a previously generated RotLib from file
	RotLib(const char* file);

	void generate();
	
	int load(const char* file);
	
	void save(const char* file);

	void concatenate(Rz *output, double radian, const char *factor = "");
	
	friend std::ostream& operator<<(std::ostream& out, const RotLib& L);


private:
	typedef std::vector<unsigned char> Code;
	typedef std::map<std::string, std::string> Codemap;
	
	struct seq
	{
		int l;    // Length(# gates) of the rotation sequence
		int s;    // Size(# bits) of compressed data
		int c;    // angle: Rz(c*pi/basis^k)
		int k;    // angle: Rz(c*pi/basis^k)
		Code word;
		seq() : l(0), s(0), c(0), k(0), word(Code()) {}
		seq(int _l, int _s, int _c, int _k, Code _word) : l(_l), s(_s), c(_c), k(_k), word(_word) {}
	};

	std::vector<seq> seqs;
	
	// Hardcoded Huffman encoding 
	
	const Codemap encode_table = {
		{"HT", "0"},
		{"S", "10"},
		{"W", "111"},
		{"X", "1100"},
		{"H", "11010"},
		{"T", "11011"}
	};
	const Codemap encode_table2 = {
		{"HT", "0"},
		{"S", "10"},
		{"X", "111"},
		{"H", "1100"},
		{"T", "1101"}
	};
	int estimate_storage(double thres, int Na);

	// Internal helpers for generating library
	int encode(std::string gates, int l, charBits *data);

	std::string decode(charBits &data, int l); 

	void load_gridsynth(const char *args, charBits *data, int *l, int *s);

	double range_zero_two(double radian);
 
	bool epsilon_close(double a, double b, double epsilon);
 
	void seq_combine(const std::vector<int> &indices, Rz *out);

	std::string exec(const char *cmd) {
		FILE *pipe = popen(cmd, "r");
		if (!pipe) return "ERROR!";
		char buffer[128];
		std::string res = "";
		while (!feof(pipe)) {
			if (fgets(buffer, 128, pipe) != NULL) {
				res += buffer;
			}
		}
		pclose(pipe);
		return res;
	}

};
#endif
