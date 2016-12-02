#include "Pointers.h"
#include <fstream>
#include <cstdlib>

using namespace std;

Pointers::Pointers(char* _path) 
{
	ifstream fs(_path, ios::binary);
	for(int i=0;i<256;i++)
	{
		pointers[i].index = (int*)malloc(4*sizeof(int));
		
		fs.read((char*)pointers[i].index, 4*sizeof(int));
		fs.read((char*)&pointers[i].used, sizeof(bool));
	}
	fs.close();
}

int Pointers::saveDirect(byte* _data, int _sizeofData, Block &_blocks)
{
	int index=findNext();
	int i=0;
	while (_sizeofData<=0||i==4)
	{
		pointers[index].index[i] = 
			_blocks.saveBlock(_data+(256*i), _sizeofData>=256?256:_sizeofData);
		_sizeofData -= 256;
	}
	return index;
}

int Pointers::saveIndirect(byte* _data, int _sizeofData, Block &_blocks)
{
	int index=findNext();
	int i=0;
	while (_sizeofData<0||i==4)
	{
		pointers[i].index[i] = 
			saveDirect(_data+(1024*i),_sizeofData>=1024?1024:_sizeofData,_blocks);
		_sizeofData-=1024;
	}
	return index;
}

Pointer Pointers::getPointer(int _index)
{
	return pointers[_index];
}

int Pointers::findNext()
{
	for(int i=0;i<POINTERSQTY;i++)
		if(!pointers[i].used)
			return i;
}

void Pointers::saveToFile(char* _path)
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
	
	ofstream fs(_path, ios::binary);
	for(int i=0; i<256; i++)
	{
		fs.write((const char*)pointers[i].index, 4*sizeof(int));
		fs.write((const char*)&pointers[i].used, sizeof(bool));
	}
	fs.close();
}

void Pointers::deleteDirect(int _index, Block &_blocks, int _sizeofData)
{
	for(int i=0;i<4;i++)
	{
		_blocks.deleteBlock(pointers[_index].index[i]);
		_sizeofData -= 256;
		if(_sizeofData <= 0) return;
	}
}

void Pointers::deleteIndirect (int _index, Block &_blocks, int _sizeofData)
{
	for(int i=0;i<4;i++)
	{
		deleteDirect(pointers[_index].index[i],_blocks,_sizeofData<=256?_sizeofData:256);
		_sizeofData-=1024;
		if(_sizeofData <= 0) return;
	}
}


Pointers::~Pointers() {
	
}

