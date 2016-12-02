#ifndef INODOS_H
#define INODOS_H
#include "Block.h"
#include "Pointers.h"
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <cstring>

struct Inodo {
	
	char *maker;     //maker of the file
	int  file_mode;  //determines the file type and how can access the file
	char *name;      //name of the file
	int  father;     //inode father
	int  size;       //size of the file
	bool isFile;     //determines if is a file or folder
	bool used;       //determines if the inode is used
	int  *directs;   //direct pointers
	int  sipointer;  //singly indirect pointer
	int  dipointer;  //double indirect pointer
	
};

class Inodos {
private:
	
	int   quantity;    //the quantity of Inodes
	Inodo *Inodes;     //the Inodes
	int   next_free(); //next Inode free

public:
	
	Inodos(int _quantity, char* _path);
	
	void mkdir(const char* _name, int _permission, char* _maker, int _father);
	
	void mkfile(const char* _name, int _permission, char* _maker, int _father,
			byte* _data, int _sizeofData, Pointers &_pointers, Block &_blocks);
	
	unsigned char* GetData(int _index);
	
	void deleteInodo(int _index, Pointers &_pointers, Block &_blocks);
	
	void moveInodo(int _index, int _indexNewFather);
	
	Inodo getInodo(int _index);
	
	void changeFather(int _index, int _indexNewFather);
	
	void saveToFile(char* _path);
	
	~Inodos();
};

#endif

