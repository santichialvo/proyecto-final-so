#ifndef PUNTEROS_H
#define PUNTEROS_H

#include <fstream>
#include <cstdlib>

#include "Bloques.h"


struct Puntero{
	int *Index;
	bool Used;
};

class Punteros {
private:
	
	const int CANT_PUNTEROS = 256;
	
	Puntero punteros[256];
	
	int FindNext();
	
public:
	
	Punteros(char* path);
	
	int SaveDirect(byte* data, int sizeOfData, Bloques &bloques);
	int SaveIndirect(byte* data, int sizeOfData, Bloques &bloques);
	
	void DeleteDirect(int index, Bloques &bloques, int sizeOfData);
	void DeleteIndirect(int index, Bloques &bloques, int sizeOfData);
	
	Puntero GetPuntero(int index);
	
	void SaveToFile(char* path);
	
};

#endif

