#include "Bloques.h"

using namespace std;

// index = 0...255
byte * Bloques::GetBlock(int index)
{
	return data + (index*SizeOfBlock);
}

void Bloques::SetBlockAmount (int blockAmoutn)
{
	BlockAmount = blockAmoutn;
	SizeOfBlock = Bloques::BYTE_AMOUNT / blockAmoutn;
}

Bloques::Bloques(int blockAmount, char* path)
{
	SetBlockAmount(blockAmount);
	LoadFromFile(path);
}

void Bloques::LoadFromFile(char* path)
{
	//El primer bloque esta ocupado
	data[0] = (byte)1;
//	for(int i=1; i<256; i++)
//		data[i] = (byte)0;
	
	ifstream file(path, ios::binary);
	file.read((char*)data, BYTE_AMOUNT);
	file.close();	
}

int Bloques::SaveBlock(byte* blockData, int sizeOfData)
{
	int index = 1;
	while(data[index] == (byte)1)
		index++;
	
	int initialIndex = index * 256;
	for(int i = 0; i < sizeOfData; i++)
		data[initialIndex + i] = blockData[i];
	
	data[index] = (byte)1;
}

void Bloques::DeleteBlock(int index)
{
	data[index] = (byte)0;
}

void Bloques::SaveToFile(char * path)
{
	ofstream file(path, ios::binary);
	file.write((char*)data, BYTE_AMOUNT);
	file.close();	
}

