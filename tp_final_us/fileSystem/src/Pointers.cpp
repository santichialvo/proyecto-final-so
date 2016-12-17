#include "Pointers.h"
#include <fstream>
#include <cstdlib>

using namespace std;

Pointers::Pointers(char* _path) 
{
	ifstream fs(_path, ios::binary);
	for(int i=0;i<POINTERSQTY;i++)
	{
		pointers[i].index = (int*)malloc(sizeof(int));
		fs.read((char*)pointers[i].index, sizeof(int));
		fs.read((char*)&pointers[i].used, sizeof(bool));
	}
	fs.close();
}

int Pointers::saveDirect(byte* _data, int _sizeofData, Block &_blocks)
{
	int index=findNext();
	int i=0;
	while (_sizeofData>0 && i<POINTERSQTY)
	{
		pointers[index].index[i] = 
			_blocks.saveBlock(_data+(BLOCK_SIZE*i), _sizeofData>=BLOCK_SIZE?BLOCK_SIZE:_sizeofData);
		_sizeofData -= BLOCK_SIZE;
		i++;
	}
	return index;
}

int Pointers::saveIndirect(byte* _data, int _sizeofData, Block &_blocks)
{
	int index=findNext();
	int i=0;
	while (_sizeofData>0 && i<POINTERSQTY)
	{
		pointers[i].index[i] = 
			saveDirect(_data+(BLOCK_SIZE*i),_sizeofData>=BLOCK_SIZE?BLOCK_SIZE:_sizeofData,_blocks);
		_sizeofData-=(BLOCK_SIZE*POINTERSPERBLOCKQTY);
		if (_sizeofData<=0) break;
		i++;
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

void Pointers::saveToFile(const char* _path)
{	
	ofstream fs(_path, ios::binary);
	for(int i=0; i<POINTERSQTY; i++)
	{
		fs.write((const char*)pointers[i].index, POINTERSPERBLOCKQTY*sizeof(int));
		fs.write((const char*)&pointers[i].used, sizeof(bool));
	}
	fs.close();
}

void Pointers::deleteDirect(int _index, Block &_blocks, int _sizeofData)
{
	for(int i=0;i<POINTERSPERBLOCKQTY;i++)
	{
		_blocks.deleteBlock(pointers[_index].index[i]);
		_sizeofData -= BLOCK_SIZE;
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

