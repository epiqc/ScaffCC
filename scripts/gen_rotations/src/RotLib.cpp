#include "RotLib.h"
#include "Exception.h"

using namespace std;

RotLib::RotLib(PRECISION p, STORAGE s, int b, bool ph)
: lib_p(p), lib_s(s), basis(b), phase(ph)
{
	int N = p -> base;
	if (N != 2 && N != 10) {
		cout << "non base-2 or base-10 target precision." << endl;
		exit(1);
	}
}

RotLib::RotLib(const char* file)
{
	load(file);
}

ostream& operator<<(ostream& out, const RotLib& L) {
	out << "===== Rotation library info =====" << endl;
	out << " - Basis = " << L.basis << endl;
	out << " - Precision: " << std::scientific << pow(L.lib_p->base, -L.lib_p->digits) << endl;
	out << " - Max Storage: " << L.lib_s -> size << L.lib_s -> unit << endl;
	out << " - Number of sequences: " << L.seqs.size() << endl;
	out << "=================================" << endl;
	return out;
}

int RotLib::estimate_storage(double thres, int Na) {
	int K10 = -log10(thres);
	int T = 10 * K10 + 3; // estimated T-count for each angle
	int bytes = Na * (2.5 * T + 200) / 8;
	char s_unit[5];
	strcpy(s_unit, lib_s->unit);
	for (int i = 0; s_unit[i] != '\0'; i++) {
		s_unit[i] = toupper(s_unit[i]);
	}
	string str_unit(s_unit);
	string units[5] = {"B", "KB", "MB", "GB", "TB"};
	int i = 0;
	int mult = 1;
	while (i < 5 && str_unit.compare(units[i]) != 0) {
		mult *= 1024;
		i++;
	}
	if (i == 5) {
		cout << "Unrecgonized storage unit." << endl;
		exit(1);
	}	else if (lib_s->size * mult < bytes) {
		cout << "Need more storage space to achieve precision. ";
		cout << "Estimates: " << bytes << " Bytes." << endl;
		exit(1);
	}

	return bytes;
}	

void RotLib::generate() {
	
	int N = lib_p -> base;
	int K = lib_p -> digits;
	int B = basis;
	// determine digits in basis
	int J = K;
	double thres = pow(N,-K);
	if (N > B) {
		while (thres < pow(B,-J)) {
			J++;
		}
	} else if (N < B) {
		while (J > 0 && thres > pow(B,-J+1)) {
			J--;
		}
	}
	int Na = (B-1) * J + 1; // number of angles to generate
	
	// Estimate storage and verify if too much
	int bytes = estimate_storage(thres, Na);

	int Ka = K; // Precision for each angle
	string p_flag;
	stringstream fss;
	if (N == 2) {
		// generate all base-2 angles, up to K of them
		Ka = Ka + 3;  // Each seq has ~10x higher precision
		fss << "-b " << Ka;
	}
	else if (N == 10) {
		// generate all base-10 angles, up to 9K of them
		Ka = Ka + 1;  // Each seq has ~10x higher precision
		fss << "-d " << Ka; // Each seq has ~10x higher precision
	}

	// Set global phase flag
	if (phase) {
		fss << " -p";
	}
	p_flag = fss.str();

	// Invoke gridsynth for rotation sequences
  // Modify me if gridsynth not found
	string gridsynthpath = "../../Rotations/gridsynth/gridsynth";
  ifstream ifile(gridsynthpath.c_str());
	if (!ifile) {
		cout << "Rotation decomposer not found!";
    cout << " Check src/RotLib.cpp for gridsynthpath." << endl;
		exit(1);
	}
	double denom = 1.0;

	// First always add Pi to the library.

	stringstream argss;
	argss << "./" << gridsynthpath << " ";
	argss << "pi" << " " << p_flag;
	const char *args = argss.str().c_str();
	int l = 0;
	int s = 0;
	charBits *bit_buff = new charBits(); // new code vector
	if (!bit_buff) {
		cout << "charBits buffer error!" << endl;
		exit(1);
	}
	load_gridsynth(args, bit_buff, &l, &s);
	cout << "\tSequence: " << l << " chars, Code: " << s << " bits.\n" <<  endl;
	Code *word_ptr = (*bit_buff).flush_code();
	if (!word_ptr) {
		cout << "charBits buffer error!" << endl;
		delete bit_buff;
		exit(1);
	} else {
		seqs.push_back(RotLib::seq(l, s, 1, 0, *word_ptr));
		// clean up code vector
		delete bit_buff;
	}

	// Add more angles to library if necessary 

	for (int k = 1; k < J + 1; k++) {
		denom /= B;
		for (int c = 1; c < B; c++) {

			stringstream ckss;
			ckss << "./" << gridsynthpath << " ";
			ckss << c << "*pi*" << setprecision(15)<< fixed << denom << " " << p_flag;
			const char *ckstr = ckss.str().c_str();

			int ckl = 0;
			int cks = 0;
			charBits *ckbit_buff = new charBits(); // new code vector
			if (!ckbit_buff) {
				cout << "charBits buffer error!" << endl;
				delete ckbit_buff;
				exit(1);
			}
			load_gridsynth(ckstr, ckbit_buff, &ckl, &cks);
			cout <<"\tSequence: "<< ckl <<" chars, Code: "<< cks <<" bits.\n"<< endl;
			Code *ckword_ptr = (*ckbit_buff).flush_code();
			if (!ckword_ptr) {
				cout << "charBits buffer error!" << endl;
				delete ckbit_buff;
				exit(1);
			} else {
				seqs.push_back(RotLib::seq(ckl, cks, c, k, *ckword_ptr));
			}
			// clean up buffer
			delete ckbit_buff;
		}
	}
}

int RotLib::encode(string gates, int l, charBits *data) {
	const Codemap *et = &encode_table; // for encoding
	if (phase) {
		// no W gate
		et = &encode_table2;
	}
	int code_size = 4;
	int g = 0;
	while (g < l) {
		stringstream gss;
		gss << gates[g];
		string gate_str = gss.str();
		if (gate_str == "H" && g < l - 1) {
			stringstream htss;
			htss << gates[g] << gates[g + 1];
			if (htss.str() == "HT") {
				gate_str = htss.str();
				g++;
			}
		}
		string huff = (*et).at(gate_str);
		int num_bits = huff.length();
		data -> charBits::write_bits(huff, num_bits);
		code_size += num_bits;
		g++;
	}
	return code_size;
}

string RotLib::decode(charBits &data, int l) {
	string all_bits;
	stringstream ress;
	data.charBits::read_bits(&all_bits);
	int len = all_bits.size();
	if (phase) {
		// no W gate
		int i = 0;
		while (i < len) {
			if (all_bits[i] == '0') {
				// 0 : HT
				ress << "HT";
				i++;
			} else if (i + 1 < len && all_bits[i+1] == '0') {
				// 10 : S
				ress << "S";
				i += 2;
			} else if (i + 2 < len && all_bits[i+2] == '1') {
				// 111 : X
				ress << "X";
				i += 3;
			} else if (i + 3 < len && all_bits[i+3] == '0') {
				// 1100 : H
				ress << "H";
				i += 4;
			} else if (i + 3 < len && all_bits[i+3] == '1') {
				// 1101 : T
				ress << "T";
				i += 4;
			} else {
				cout << "Something went wrong while decoding." << endl;
				exit(1);
			}
		}
	} else {
		// with W gate
		int i = 0;
		while (i < len) {
			if (all_bits[i] == '0') {
				// 0 : HT
				ress << "HT";
				i++;
			} else if (i + 1 < len && all_bits[i+1] == '0') {
				// 10 : S
				ress << "S";
				i += 2;
			} else if (i + 2 < len && all_bits[i+2] == '1') {
				// 111 : W
				ress << "W";
				i += 3;
			} else if (i + 3 < len && all_bits[i+3] == '0') {
				// 1100 : X
				ress << "X";
				i += 4;
			} else if (i + 4 < len && all_bits[i+4] == '0') {
				// 11010 : H
				ress << "H";
				i += 5;
			} else if (i + 4 < len && all_bits[i+4] == '1') {
				// 11011 : T
				ress << "T";
				i += 5;
			} else {
				cout << "Something went wrong while decoding." << endl;
				exit(1);
			}

		}
	}
	return ress.str();
}

void RotLib::load_gridsynth(const char *args, charBits *data, int *l, int *s) {
	cout << "Calling: " << args << endl;
	string res = exec(args);
	cout << "\t" << res; 
	int ll = res.length() - 1; // excluding '\n' at the end
	int ss;
	if (!data) {
		cout << "charBits buffer error!" << endl;
		exit(1);
	} else {
		ss = encode(res, ll, data);
	}
	if (!l || !s) {
		cout << "l,s reference error!" << endl;
		exit(1);
	} else {
		*l = ll;
		*s = ss;
	}
	return;
}

int RotLib::load(const char *file) {
	char header[27];
	char check[] = "Gridsynth Rotation Library";
	char msg[128];

	ifstream infile;
	infile.open(file, ios::in | ios::binary);
	if (!infile) {
		sprintf(msg, "unable to open file '%s' for reading", file);
		cout << msg << endl;
		exit(1);
	}
	// Check Header
	infile.read((char *)header, sizeof(check));
	if (strncmp(header, check, 27*sizeof(char))) {
		sprintf(msg, "'%s' does not look like a Rotation Library.", file);
		cout << msg << endl;
		exit(1);
	}

	// Structural stuff
	infile.read((char *)&basis, sizeof(basis));
	infile.read((char *)&phase, sizeof(phase));
	infile.read((char *)&lib_p, sizeof(lib_p));
	infile.read((char *)&lib_s, sizeof(lib_s));

	// Content of sequences
	int total;
	infile.read((char *)&total, sizeof(int));

	for (int si = 0; si < total; si++) {
		int ls;
		int ss;
		int cs;
		int ks;
		Code words;

		infile.read((char *)&ls, sizeof(int));
		infile.read((char *)&ss, sizeof(int));
		infile.read((char *)&cs, sizeof(int));
		infile.read((char *)&ks, sizeof(int));
		int code_bytes;
		infile.read((char *)&code_bytes, sizeof(int));
		unsigned char tmp;
		for (int ci = 0; ci < code_bytes; ci++) {
			infile.read((char *)&tmp, sizeof(unsigned char));
			words.push_back(tmp);
		}
		
		seqs.push_back(RotLib::seq(ls, ss, cs, ks, words));
	}
	
	infile.close();
	return 1;

}

void RotLib::save(const char *file) {
	char header[] = "Gridsynth Rotation Library";
	char msg[128];

	ofstream outfile;
	outfile.open(file, ios::out | ios::binary);
	if (!outfile) {
		sprintf(msg, "unable to open file '%s' for writing", file);
		cout << msg << endl;
		exit(1);
	}
	// Header
	outfile.write(header, sizeof(header));
	// Structural stuff
	outfile.write((char *)&basis, sizeof(basis));
	outfile.write((char *)&phase, sizeof(phase));
	outfile.write((char *)&lib_p, sizeof(lib_p));
	outfile.write((char *)&lib_s, sizeof(lib_s));
	// Content of sequences
	int total = seqs.size();
	outfile.write((char *)&total, sizeof(int));
	vector<seq>::iterator si = seqs.begin();
	while (si != seqs.end()) {
		outfile.write((char *)&(*si).l, sizeof(int));
		outfile.write((char *)&(*si).s, sizeof(int));
		outfile.write((char *)&(*si).c, sizeof(int));
		outfile.write((char *)&(*si).k, sizeof(int));
		int code_bytes = (*si).word.size();
		outfile.write((char *)&code_bytes, sizeof(int));
		for (Code::iterator ci = (*si).word.begin(); ci != (*si).word.end(); ci++) {
			outfile.write((char *)&(*ci), sizeof(unsigned char));
		}
		si++;
	}
	
	outfile.close();

}

double RotLib::range_zero_two(double radian) {
  // assuming radian does not have pi factor 
  while (radian < 0) {
    radian += 2;
  }
  while (radian >= 2) {
    radian -= 2;
  }
  return radian;
}

bool RotLib::epsilon_close(double a, double b, double epsilon) {
	double big = (a > b)? a : b;
	double small = (a <= b)? a : b;
	return (big - small < epsilon); // strictly close
}

void RotLib::seq_combine(const vector<int> &indices, Rz *out) {
	// assuming indices are unique and check if seq is clean
	if (!out || out -> length != 0) {
		cout << "Invalid seq destination (NULL or non-empty)." << endl;
		exit(0);
	}
	// append sequences from left to right
	stringstream gss;
	vector<int>::const_iterator ii;
	for (ii = indices.begin(); ii != indices.end(); ii++) {
		const seq seqi = seqs.at(*ii);
		out -> length += seqi.l;
		charBits *bits = new charBits(seqi.word);
		gss << decode(*bits, seqi.l);
		// clean up bits
		delete bits;
	}
	out -> gates = gss.str();
}

void RotLib::concatenate(Rz *output, double radian, const char *factor) {
	double pi = 3.1415926535897;
	char pi_str[] = "pi";
	char no_str[] = "";
	char input[10];
	strcpy(input, factor);
	for (int i = 0; input[i] != '\0'; i++) {
		input[i] = tolower(input[i]);
	}
	double r_angle;
	bool radian_pi = strcmp(input, pi_str) == 0;
	if (strcmp(input, no_str) == 0) {
		radian /= pi;
		radian_pi = true;
	}
	if (radian_pi) {
		// angle: radian*pi
		radian = range_zero_two(radian);
		r_angle = radian;
		vector<int> indices;
		int J = seqs.size() / (basis - 1);
		int c_array[J + 1]; // basis representation of radian
		// c_array[i] stores c_i
		for (int i = 0; i < J + 1; i++) c_array[i] = 0;
		// integer part
		if (radian >= 1.0) {
			c_array[0] = 1;
			radian -= 1.0;
		}
		int j = 1;
		double bj = 1.0 / basis; // basis^(-j)
		double epsilon = pow(lib_p -> base, -(lib_p -> digits));
		while (!epsilon_close(radian, 0, epsilon) && j < J + 1) {
			while (radian - bj >= 0) {
				radian -= bj;
				c_array[j] += 1;
			}
			bj /= (double)basis;
			j++;
		}
		if (c_array[0] == 1) indices.push_back(0);
		for (int k = 1; k < J + 1; k++) {
			int c = c_array[k];
			if (c > 0) {
				int id = (basis - 1) * (k - 1) + c;
				if (id >= seqs.size()) {
					cout << "seqs out of bound: " << id << " out of " << seqs.size() << endl;
					exit(1);
				}
				indices.push_back(id); 
			}
		}
		// Now combine the selected sequences
		stringstream angless;
		angless << r_angle << "*pi";
		output -> theta = angless.str();
		output -> p = lib_p;
		seq_combine(indices, output);

	} else {
		cout << "non pi-factor angles are not yet supported!" << endl;
		exit(1);
  }
}

