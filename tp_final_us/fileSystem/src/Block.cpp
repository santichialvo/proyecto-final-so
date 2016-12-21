#include "Block.h"
#include <fstream>
#include <iostream>

using namespace std;

Block::Block(char *_path) 
{
	loadFromFile(_path);
}

byte* Block::getBlock(int _index)
{
	return data+(_index*BLOCK_SIZE);
}

void Block::loadFromFile(char *_path) 
{
	ifstream fs(_path, ios::binary);
	fs.read((char*)data, BYTE_AMOUNT);
	fs.close();
}

int Block::saveBlock(byte* _data, int _sizeofData)
{
	int index = 1;
	while(data[index] == (byte)1)
		index++;
	
	int initialIndex = index*BLOCK_SIZE;
	
	for(int i=0; i<_sizeofData;i++)
		data[initialIndex + i] = _data[i];
	
	data[index] = (byte)1;
	
//	cout<<index<<endl;
	return index;
}

void Block::deleteBlock(int _index)
{
	data[_index] = (byte)0;
}

void Block::catBlock(int _index, int BLOCK_REMAIN)
{	int i=0;
	int offset = (BLOCK_SIZE<=BLOCK_REMAIN)? BLOCK_SIZE : BLOCK_REMAIN ; 
	while(i<offset)
	{
		cout<< data[_index*BLOCK_SIZE + i];
		++i;
	}
	if(offset<BLOCK_SIZE) cout<<endl;
//	cout<<_index<<endl;
}

void Block::saveToFile(const char *_path) 
{
	ofstream fs(_path, ios::binary);
	fs.write((char*)data, BYTE_AMOUNT);
	fs.close();
}

Block::~Block() {
	
}

void Block::getBlockInfo() 
{
	cout<<"DATA BLOCKS USED: "<<endl;
	for(int i=0;i<BLOCK_QTY;i++) { 
		if (data[i] == (byte)1)
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
