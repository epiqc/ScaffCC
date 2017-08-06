#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <math.h> // for pow

#include "RotLib.h"
#include "Exception.h"
using namespace std;


int main(int argc, char *argv[])
{
	
	int sk_begin, sk_end;
	if(argc!=6){
		cout << "Example Usage: ./example 2 10 3 100 0"<< endl;
		cout << "./example b n k s p" << endl;
		cout << "\t b - basis of angles. Library will have angles: Rz(c*pi/(b^j)), for c=1..b, j=0..k" << endl;
		cout << "\t n - precision base. Can be 2 or 10." << endl;
		cout << "\t k - precision power. Library precision: n^(-k) " << endl;
		cout << "\t s - max storage. Library size at most s (KB)." << endl;
		cout << "\t p - global phase. Indication of precision up to global phase: true(1)/false(0)).\n" << endl;
		exit(1);
	}
	// Library Angles: c*pi/(basis^j), for c=1..basis and j=0..k
	int basis = atoi(argv[1]); 
	int n=atoi(argv[2]);
	int k=atoi(argv[3]);
	int s=atoi(argv[4]);
	bool p = (atoi(argv[5]) == 1)? true : false;

	// Setting up outputs
	char fname[200];
	sprintf(fname,"example.out");
	fstream out;
	out.open(fname, fstream::out);

	time_t rawtime;
	struct tm * timeinfo;
	time_t now=time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	cout << "The current date/time is: " << asctime (timeinfo) << endl;
	out << "The current date/time is: " << asctime (timeinfo) << endl;

	char name[200];
	sprintf(name,"rotations_%d_%dB.lib",basis, s);
	cout << "Generating " << name << endl;
	out << "Generating " << name << endl;
	
	// Use the following to generate a rotation library
	PRECISION	myprecision = new struct pre;
	myprecision -> base = n; // e.g. 2 or 10
	myprecision -> digits = k; // e.g. 3
	myprecision -> epsilon = pow((double)n, -k); // n^(-k): e.g. 0.001
	
	STORAGE mystorage = new struct sto;
	mystorage -> unit = (char *)"b"; // e.g. B, KB, MB, etc. (case insensitive)
	mystorage -> size = s;

	// Now generating RotLib
	RotLib rlib(myprecision, mystorage, basis, p); 
	rlib.generate();
	cout << "Generation finished!" << endl;
	out << "Generation finished!" << endl;
	
	// Optionally save the RotLib into a file
	rlib.save(name);
	cout << "Library saved to " << name << endl;
	out << "Library saved to " << name << endl;

	cout << rlib << endl;
	out << rlib << endl;
	
	// Which can be loaded next time as follows
	RotLib rlib2(name);
	
	cout << rlib2 << endl;
	out << rlib2 << endl;


	/***************************************************************
 	 *  Now we can use the library to construct Rz rotation angles *
	 ***************************************************************/
	RotLib::Rz *angle1 = new RotLib::Rz(); // Rz pointer
	RotLib::Rz *angle2 = new RotLib::Rz();
	RotLib::Rz *angle3 = new RotLib::Rz();

	clock_t start;
	clock_t end;
	start = clock();
	rlib2.concatenate(angle1, 0.625, "Pi"); // angle1: 0.625*pi
	end = clock();
	cout << angle1->theta << " (";
	cout << ((float)end-(float)start) / CLOCKS_PER_SEC << " seconds):" << endl;
	cout << angle1 -> gates << "\n" << endl;
	out << angle1->theta << " (";
	out << ((float)end-(float)start) / CLOCKS_PER_SEC << " seconds):" << endl;
	out << angle1 -> gates << "\n" << endl;


	start = clock();
	rlib2.concatenate(angle2, 1.625, "Pi"); // angle2: 1.625*pi
	end = clock();
	cout << angle2->theta << " (";
	cout << ((float)end-(float)start) / CLOCKS_PER_SEC << " seconds):" << endl;
	cout << angle2 -> gates << "\n" << endl;
	out << angle2->theta << " (";
	out << ((float)end-(float)start) / CLOCKS_PER_SEC << " seconds):" << endl;
	out << angle2 -> gates << "\n" << endl;


	start = clock();
	rlib2.concatenate(angle3, -1.5); // angle3: -1.5
	end = clock();
	cout << angle3->theta << " (";
	cout << ((float)end-(float)start) / CLOCKS_PER_SEC << " seconds):" << endl;
	cout << angle3 -> gates << "\n" << endl;
	out << angle3->theta << " (";
	out << ((float)end-(float)start) / CLOCKS_PER_SEC << " seconds):" << endl;
	out << angle3 -> gates << "\n" << endl;


	cout << "Finished loading." << endl;
	out << rlib << endl;

	time_t nowlib=time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	cout << "The current date/time is: " << asctime (timeinfo) << endl;
	cout << "Seconds passed: " << nowlib-now << endl;
	out << "The current date/time is: " << asctime (timeinfo) << endl;
	out << "Seconds passed: " << nowlib-now << endl;
	
	out.close();
}
