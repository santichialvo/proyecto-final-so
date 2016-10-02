#ifndef BLOQUES_H
#define BLOQUES_H

#include <fstream>

#define byte unsigned char

class Bloques {
private:
	
	static const int BYTE_AMOUNT = 64*1024;
	int BlockAmount;
	int SizeOfBlock;
	
	byte data[BYTE_AMOUNT];
	
public:
	
	Bloques(int blockAmount, char* path);
	
	void SetBlockAmount(int blockAmoutn);
	byte* GetBlock(int index);
	
	int SaveBlock(byte* data, int sizeOfData);
	void DeleteBlock(int index);
	
	void LoadFromFile(char* path);
	void SaveToFile(char* path);
	
};

#endif

