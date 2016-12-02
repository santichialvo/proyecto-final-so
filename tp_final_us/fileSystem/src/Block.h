#ifndef BLOCK_H
#define BLOCK_H

typedef unsigned char byte;
#define BYTE_AMOUNT 65536

class Block {
private:
	int  blockAmount;
	int  sizeofBlock;
	byte data[BYTE_AMOUNT];
public:
	Block(int _blockAmount, char *_path);
	void setBlockAmount(int _blockAmount);
	byte* getBlock(int _index);
	int saveBlock(byte* _data, int _sizeofData);
	void deleteBlock(int _index);
	void loadFromFile(char* _path);
	void saveToFile(char* _path);
	~Block();
};

#endif

