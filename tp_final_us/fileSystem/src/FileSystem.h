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

#define USERSQTY 10
//enum UserTypes {OWNER,GROUP,OTHERS};

struct Group {
	char*	groupName;
	int 	membersqty;
	int* 	idembers;
};

struct User {
	char	userName[10];
	int		UserID;
};

class FileSystem {
private:
	int			currentUserID;
	int			currentGroupUserID;
	int    		actualInode;
	User		*users;
	Block    	*blocks;
	Pointers 	*pointers;
	Inodos   	*Inodes;
	bool     	getPermission(int _inode, int _mode); //0 read, 1 write, 2 execute (rwx)
	void     	listChildren(int _inode,int _level);
	int      	searchOnInode(string _name, int _inode);
	int      	stringToInode(string _command);
public:
	FileSystem();
	void WritePrompt    ();
	void listChildren   (int _mode);
	void increaseLevel  ();
	void changeDir      (string _dir);
	void makeFile       (string _name, int _father, int _permission, byte* _data, int _sizeofData);
	void makeDir        (string _name, int _father, int _permission);
	void del            (int _inode);
	void showTree       ();
	void changeUser    	(int _new);
	void makeFile       (string _path, unsigned char* _data, int _size);
	void makeDir        (string _path);
	void move           (string _from, string _to);
	void chmod 			(int _newmode, string _path);
	int showFileMode	(int _filemode,int _mode);
	void erase          (string _path);
	void save_blocks    (string _path);
	void save_inodes    (string _path);
	void save_pointers  (string _path);
	bool verifyDirName  (const char * _path,int _actualInode);
	~FileSystem();
};

#endif

