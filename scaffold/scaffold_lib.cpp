
#include <string.h>
#include <iostream>
#include <map>
#include <utility>

#ifndef __SCAFFOLD_LIB
#define __SCAFFOLD_LIB

unsigned int HGateCount = 0;
unsigned int TGateCount = 0;
unsigned int TdagGateCount = 0;
unsigned int XGateCount = 0;
unsigned int ZGateCount = 0;
unsigned int RzGateCount = 0;

void X (qbit *x) {
    std::cout << "\tX " << x->getName() << std::endl;
    XGateCount++;
}

#endif // __SCAFFOLD_LIB

int main() {
}

