#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include "Block.h"
#include "Pointers.h"
#include "Inodos.h"
#include "../src/color/rlutil.h"
#include <string>
#include <iostream>
#include <list>
#include <cstdlib>
using namespace std;

#define INODESQTY 500
enum Group {ADMIN,USER,GUEST};

class FileSystem {
private:
	Group    group;
	int      actualInode;
	Block    *blocks;
	Pointers *pointers;
	Inodos   *Inodes;
	bool     getPermission(int _inode, int _mode); //0 read, 1 execute, 2 edit
	void     listChildren(int _inode,int _level);
	int      searchOnInode(string _name, int _inode);
	int      stringToInode(string _command);
public:
	FileSystem();
	void WritePrompt    ();
	void listChildren   ();
	void increaseLevel  ();
	void changeDir      (string _dir);
	void makeFile       (string _name, int _father, int _permission, byte* _data, int _sizeofData);
	void makeDir        (string _name, int _father, int _permission);
	void del            (int _inode);
	void showTree       ();
	void changeGroup    (Group _new);
	void makeFile       (string _path, unsigned char* _data, int _size);
	void makeDir        (string _path);
	void move           (string _from, string _to);
	void erase          (string _path);
	~FileSystem();
};

#endif

