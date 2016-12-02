#ifndef SYSTEM_H
#define SYSTEM_H
#include <cstdlib>
#include "FileSystem.h"

class System {
private:
public:
	System();
	void runSystem     ();
	void splitBySpaces (string &command, list<string> &commands);
	void showHelp      ();
	~System();
};

#endif

