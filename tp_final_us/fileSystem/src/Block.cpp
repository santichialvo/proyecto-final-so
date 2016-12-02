#include "Block.h"
#include <fstream>

using namespace std;

Block::Block(int _blockAmount, char *_path) 
{
	setBlockAmount(_blockAmount);
	loadFromFile(_path);
}

byte* Block::getBlock(int _index)
{
	return data+(_index*sizeofBlock);
}

void Block::setBlockAmount(int _blockAmount) 
{
	blockAmount = _blockAmount;
	sizeofBlock = BYTE_AMOUNT/blockAmount;
}

void Block::loadFromFile(char *_path) 
{
	//El primer bloque esta ocupado
	data[0] = (byte)1;
	
	ifstream fs(_path, ios::binary);
	fs.read((char*)data, BYTE_AMOUNT);
	fs.close();
}

int Block::saveBlock(byte* _data, int _sizeofData) 
{
	int index = 1;
	
	while(data[index] == (byte)1)
		index++;
	
	int initialIndex = index*256;
	
	for(int i=0; i<_sizeofData;i++)
		data[initialIndex + i] = _data[i];
	
	data[index] = (byte)1;
}

void Block::deleteBlock(int _index)
{
	data[_index] = (byte)0;
}

void Block::saveToFile(char *_path) 
{
	ofstream fs(_path, ios::binary);
	fs.write((char*)data, BYTE_AMOUNT);
	fs.close();
}

Block::~Block() {
	
}

