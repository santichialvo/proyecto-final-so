#include "Pointers.h"
#include <fstream>
#include <cstdlib>

using namespace std;

Pointers::Pointers(char* _path) 
{
	ifstream fs(_path, ios::binary);
	for(int i=0;i<POINTERSQTY;i++)
	{
		pointers[i].index = (int*)malloc(POINTERSPERBLOCKQTY*sizeof(int));
		fs.read((char*)pointers[i].index, POINTERSPERBLOCKQTY*sizeof(int));
		fs.read((char*)&pointers[i].used, sizeof(bool));
	}
	if (!fs.is_open()) 
	{
		for(int i=0;i<POINTERSQTY;i++)
			pointers[i].used = false;
	}
	fs.close();
}

int Pointers::saveDirect(byte* _data, int _sizeofData, Block &_blocks)
{
	int _index=findNext();
	int i=0;
	while (_sizeofData>0 && i<POINTERSPERBLOCKQTY)
	{
		pointers[_index].index[i] = 
			_blocks.saveBlock(_data+(BLOCK_SIZE*i), _sizeofData>=BLOCK_SIZE?BLOCK_SIZE:_sizeofData);
		_sizeofData -= BLOCK_SIZE;
		i++;
	}
	pointers[_index].used = true;
	return _index;
}

int Pointers::saveIndirect(byte* _data, int _sizeofData, Block &_blocks)
{
	int index=findNext();
	int i=0;
	while (_sizeofData>0 && i<POINTERSPERBLOCKQTY)
	{
//		pointers[index].index[i] = 
			saveDirect(_data+(BLOCK_SIZE*i),_sizeofData>=(BLOCK_SIZE*POINTERSPERBLOCKQTY)?(BLOCK_SIZE*POINTERSPERBLOCKQTY):_sizeofData,_blocks);
		_sizeofData-=(BLOCK_SIZE*POINTERSPERBLOCKQTY);
		if (_sizeofData<=0) break;
		i++;
	}
	pointers[index].used = true;
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
	pointers[_index].used = false;
	for(int i=0;i<POINTERSPERBLOCKQTY;i++)
	{
		_blocks.deleteBlock(pointers[_index].index[i]);
		_sizeofData -= BLOCK_SIZE;
		if(_sizeofData <= 0) return;
	}
}

void Pointers::catDirect(int _index, Block &_blocks, int _sizeofData)
{
	for(int i=0;i<POINTERSPERBLOCKQTY;i++)
	{
		_blocks.catBlock(pointers[_index].index[i],_sizeofData);
		_sizeofData -= BLOCK_SIZE;
		if(_sizeofData <= 0) return;
	}
}

void Pointers::deleteIndirect (int _index, Block &_blocks, int _sizeofData)
{
	pointers[_index].used = false;
	for(int i=0;i<POINTERSPERBLOCKQTY;i++)
	{
		deleteDirect(/*pointers[_index].index[i]*/_index,_blocks,_sizeofData<=(POINTERSPERBLOCKQTY*BLOCK_SIZE)?_sizeofData:(POINTERSPERBLOCKQTY*BLOCK_SIZE));
		_sizeofData-=(POINTERSPERBLOCKQTY*BLOCK_SIZE);
		if(_sizeofData <= 0) return;
	}
}

void Pointers::catIndirect (int _index, Block &_blocks, int _sizeofData)
{
	for(int i=0;i<POINTERSPERBLOCKQTY;i++)
	{
		catDirect(/*pointers[_index].index[i]*/_index,_blocks,_sizeofData<=(POINTERSPERBLOCKQTY*BLOCK_SIZE)?_sizeofData:(POINTERSPERBLOCKQTY*BLOCK_SIZE));
		_sizeofData-=(POINTERSPERBLOCKQTY*BLOCK_SIZE);
		if(_sizeofData <= 0) return;
	}
}

void Pointers::getPointerInfo()
{
	cout<<"POINTERS BLOCKS USED: "<<endl;
	for(int i=0;i<POINTERSQTY;i++) 
	{ 
		if (pointers[i].used)
		{
			rlutil::setColor(12);
			cout<<i<<" ";
		}
		else 
		{
			rlutil::setColor(15);
			cout<<i<<" ";
		}
	}
	cout<<endl<<endl;
	rlutil::setColor(15);
}

Pointers::~Pointers() {
	
}

