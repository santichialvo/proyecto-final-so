#include "Inodos.h"


using namespace std;

#define FILESIZEMAX 6144

Inodos::Inodos(int _quantity, char* _path) {
	Inodes = (Inodo*)malloc(_quantity*sizeof(Inodo));
	quantity = _quantity;
	
	ifstream fs(_path, ios::binary);
	for(int i=0;i<quantity;i++)
	{
		Inodes[i].maker = (char*)malloc(10*sizeof(char));
		Inodes[i].directs = (int*)malloc(4*sizeof(int));
		Inodes[i].name = (char*)malloc(10*sizeof(char));
		
		fs.read(Inodes[i].maker, 10*sizeof(char));
		fs.read((char*)&Inodes[i].file_mode, sizeof(int));
		fs.read(Inodes[i].name, 10*sizeof(char));
		
		fs.read((char*)&Inodes[i].father, sizeof(int));
		fs.read((char*)&Inodes[i].size, sizeof(int));
		
		fs.read((char*)&Inodes[i].isFile, sizeof(bool));
		fs.read((char*)&Inodes[i].used, sizeof(bool));
		
		fs.read((char*)Inodes[i].directs, 4*sizeof(int));
		fs.read((char*)&Inodes[i].sipointer, sizeof(int));
		fs.read((char*)&Inodes[i].dipointer, sizeof(int));
	}
	fs.close();
}

void Inodos::mkdir(const char* _name, int _permission, char* _maker, int _father) 
{
	int index = next_free();
	
	strcpy(Inodes[index].name, _name);
	Inodes[index].file_mode = _permission;
	Inodes[index].maker = _maker;
	Inodes[index].father = _father;
	Inodes[index].isFile = false;
	Inodes[index].used = true;
	Inodes[index].size = 0;
}

void Inodos::mkfile(const char* _name, int _permission, char* _maker, int _father,
				byte* _data, int _sizeofData, Pointers &_pointers, Block &_blocks)
{
	if(_sizeofData>FILESIZEMAX) {
		cout<<"Invalid size of file";
		return;
	}
	
	int index = next_free();
	
	strcpy(Inodes[index].name, _name);
	Inodes[index].file_mode = _permission;
	Inodes[index].maker = _maker;
	Inodes[index].father = _father;
	Inodes[index].isFile = true;
	Inodes[index].used = true;
	Inodes[index].size = _sizeofData;
	
	for (int i=0;i<4;i++) {
		Inodes[index].directs[i] = _blocks.saveBlock(_data+(256*i), _sizeofData>=256?256:_sizeofData);
		_sizeofData-=256;
		if (_sizeofData<=0) return;
	}
	if(_sizeofData>1024) {
		Inodes[index].sipointer = _pointers.saveDirect(_data,1024,_blocks);
		_sizeofData-=1024;
	}
	else {
		Inodes[index].sipointer = _pointers.saveDirect(_data,_sizeofData,_blocks);
		return;
	}
	Inodes[index].dipointer = _pointers.saveIndirect(_data, _sizeofData, _blocks);
}

void Inodos::deleteInodo(int _index, Pointers &_pointers, Block &_blocks) {
	Inodes[_index].used = false;
	int sizeofData = Inodes[_index].size;
	
	if(Inodes[_index].isFile)
	{
		for(int i=0;i<4;i++)
		{
			_blocks.deleteBlock(Inodes[_index].directs[i]);
			sizeofData -= 256;
			if(sizeofData<=0) return;
		}
		_pointers.deleteDirect(Inodes[_index].sipointer, _blocks, sizeofData); //TODO No sera indirect?
		sizeofData -= 1024;
		if(sizeofData<=0) return;
		
		_pointers.deleteIndirect(Inodes[_index].dipointer, _blocks, sizeofData);
	}
}


int Inodos::next_free() {
	for(int i=0; i<quantity; i++)
		if(!(Inodes[i].used))
			return i;
}

void Inodos::moveInodo(int _index, int _indexNewFather) {
	Inodes[_index].father = _indexNewFather;
}

Inodo Inodos::getInodo(int _index) {
	return Inodes[_index];
}

void Inodos::saveToFile(char *_path) {
	ofstream fs(_path, ios::binary);
	
	for(int i=0;i<quantity;i++)
	{
		fs.write(Inodes[i].maker, 10*sizeof(char));
		fs.write((const char*)&Inodes[i].file_mode, sizeof(int));
		fs.write(Inodes[i].name, 10*sizeof(char));
		
		fs.write((const char*)&Inodes[i].father, sizeof(int));
		fs.write((const char*)&Inodes[i].size, sizeof(int));
		
		fs.write((const char*)&Inodes[i].isFile, sizeof(bool));
		fs.write((const char*)&Inodes[i].used, sizeof(bool));
		
		fs.write((const char*)Inodes[i].directs, 4*sizeof(int));
		fs.write((const char*)&Inodes[i].sipointer, sizeof(int));
		fs.write((const char*)&Inodes[i].dipointer, sizeof(int));
	}
	
	fs.close();
}

void Inodos::changeFather(int _index, int _indexNewFather) {
	Inodes[_index].father = _indexNewFather;
}

Inodos::~Inodos() {
	
}

