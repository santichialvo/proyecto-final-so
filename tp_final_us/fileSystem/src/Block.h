#ifndef BLOCK_H
#define BLOCK_H

typedef unsigned char byte;
#define BLOCK_QTY 64
#define BLOCK_SIZE 1024
#define BYTE_AMOUNT BLOCK_SIZE*BLOCK_QTY

class Block {
private:
	byte data[BYTE_AMOUNT];
public:
	Block(char *_path);
	byte* getBlock(int _index);
	int saveBlock(byte* _data, int _sizeofData);
	void deleteBlock(int _index);
	void loadFromFile(char* _path);
	void saveToFile(const char* _path);
	~Block();
};

#endif

