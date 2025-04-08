#ifndef PRINTS_H
#define PRINTS_H

#include <iostream>
#include <vector>

using namespace std;

typedef enum {
	DNE = 0,
	INV_CMD,
} ExternalErr;

void perrorSmashInternal(const string cmd, const string msg);
void perrorSmashExternal(ExternalErr err);

#endif // PRINTS_H