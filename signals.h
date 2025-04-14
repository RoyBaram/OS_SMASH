#ifndef SIGNALS_H
#define SIGNALS_H

/*=============================================================================
* includes, defines, usings
=============================================================================*/
#include <iostream>
#include <signal.h>

#define NOT_SIGNALED 0
#define SIGNALED 1

using namespace std;
/*=============================================================================
* global functions
=============================================================================*/

void sigtstpHandler(int sig);

void sigintHandler(int sig);


#endif //__SIGNALS_H__