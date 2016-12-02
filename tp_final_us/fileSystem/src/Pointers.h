#ifndef POINTERS_H
#define POINTERS_H

#include "Block.h"

#define POINTERSQTY 256

struct Pointer {
	int *index;        //index of the pointer
	bool used;         //indicates if it was used
};

class Pointers {
private:
	Pointer pointers[256];
	int findNext();
public:
	Pointers(char* _path);
	int saveDirect(byte* _data, int _sizeofData, Block &_blocks);
	int saveIndirect(byte* _data, int _sizeofData, Block &_blocks);
	void deleteDirect(int _index, Block &_blocks, int _sizeofData);
	void deleteIndirect(int _index, Block &_blocks, int _sizeofData);
	Pointer getPointer(int _index);
	void saveToFile(char* _path);
	~Pointers();
};

#endif

