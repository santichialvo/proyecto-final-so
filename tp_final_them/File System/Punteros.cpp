#include "Punteros.h"

Punteros::Punteros(char* path)
{
	std::ifstream fs(path, std::ios::binary);
	for(int i = 0; i < 256; i++)
	{
		punteros[i].Index = (int*)malloc(4*sizeof(int));
		
		fs.read((char*)punteros[i].Index, 4*sizeof(int));
		fs.read((char*)&punteros[i].Used, sizeof(bool));
	}
	fs.close();
}

int Punteros::SaveDirect(byte * data, int sizeOfData, Bloques &bloques)
{
	int index = FindNext();
	
	int i = 0;
	while (sizeOfData <= 0 || i == 4)
	{
		punteros[index].Index[i] = 
			bloques.SaveBlock(data+(256*i), sizeOfData >= 256 ? 256 : sizeOfData);
		sizeOfData -= 256;
	}
	
	return index;
}

int Punteros::SaveIndirect(byte * data, int sizeOfData, Bloques &bloques)
{
	int index = FindNext();
	
	int i = 0;
	while (sizeOfData < 0 || i == 4)
	{
		punteros[i].Index[i] = 
			SaveDirect(data+(1024*i), sizeOfData >= 1024 ? 1024 : sizeOfData, bloques);
		sizeOfData -= 1024;
	}
	
	return index;
}



Puntero Punteros::GetPuntero(int index)
{
	return punteros[index];
}

int Punteros::FindNext()
{
	for(int i = 0; i < CANT_PUNTEROS; i++)
		if(punteros[i].Used == false)
			return i;
}

void Punteros::SaveToFile(char* path)
{
	
	///Para crear el archivo de nuevo en blanco.
//	Puntero punt;
//	punt.Index[0] = 0;
//	punt.Index[1] = 0;
//	punt.Index[2] = 0;
//	punt.Index[3] = 0;
//	punt.Used = false;
//	
//	for(int i=0; i<256; i++)
//		punteros[i] = punt;
	
	std::ofstream fs(path, std::ios::binary);
	for(int i = 0; i < 256; i++)
	{
		fs.write((const char*)punteros[i].Index, 4*sizeof(int));
		fs.write((const char*)&punteros[i].Used, sizeof(bool));
	}
	fs.close();
}


void Punteros::DeleteDirect(int index, Bloques &bloques, int sizeOfData)
{
	for(int i = 0; i<4; i++)
	{
		bloques.DeleteBlock(punteros[index].Index[i]);
		
		sizeOfData -= 256;
		if(sizeOfData <= 0) return;
	}
}

void Punteros::DeleteIndirect (int index, Bloques &bloques, int sizeOfData)
{
	for(int i = 0; i<4; i++)
	{
		DeleteDirect(punteros[index].Index[i], bloques, sizeOfData <= 256 ? sizeOfData : 256);
		
		sizeOfData -= 1024;
		if(sizeOfData <= 0) return;
	}
}

