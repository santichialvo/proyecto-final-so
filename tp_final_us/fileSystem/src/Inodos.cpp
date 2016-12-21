#include "Inodos.h"

//Asumiendo arquitectura de 32bits, tenemos bloques de 1Kbyte y cada enlace 
//tiene 4 bytes, por lo que cada bloque podra tener 1024/4 = 256 direcciones 
//Cual es la maxima cantidad de bloques de punteros?

using namespace std;

Inodos::Inodos(char* _path) {
//	Inodes = (Inodo*)malloc(INODESQTY*sizeof(Inodo));
	
	ifstream fs(_path, ios::binary);
	for(int i=0;i<INODESQTY;i++)
	{
		Inodes[i].directs = (int*)malloc(4*sizeof(int));
		Inodes[i].name = (char*)malloc(10*sizeof(char));
		
		fs.read((char*)&Inodes[i].ownerid,sizeof(int));
		fs.read((char*)&Inodes[i].groupid,sizeof(int));
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
	if (!fs.is_open()) 
	{
		for(int i=0;i<INODESQTY;i++)
			Inodes[i].used = false;
	}
	fs.close();
}

void Inodos::mkdir(const char* _name, int _permission, int _ownerid, int _groupid, int _father) 
{
	int index = strcmp(_name,"root")?next_free():0;
	
	strcpy(Inodes[index].name, _name);
	Inodes[index].file_mode = _permission;
	Inodes[index].ownerid = _ownerid;
	Inodes[index].groupid = _groupid;
	Inodes[index].father = _father;
	Inodes[index].isFile = false;
	Inodes[index].used = true;
	Inodes[index].size = 0;
}

void Inodos::mkfile(const char* _name, int _permission, int _ownerid, int _groupid, int _father,
				byte* _data, int _sizeofData, Pointers &_pointers, Block &_blocks)
{
	if(_sizeofData>FILESIZEMAX) {
		cout<<"Invalid size of file"<<endl;
		return;
	}

	int index = next_free();

	strcpy(Inodes[index].name, _name);
	Inodes[index].file_mode = _permission;
	Inodes[index].ownerid = _ownerid;
	Inodes[index].groupid = _groupid;
	Inodes[index].father = _father;
	Inodes[index].isFile = true;
	Inodes[index].used = true;
	Inodes[index].size = _sizeofData;

	for (int i=0;i<4;i++) 
	{
		Inodes[index].directs[i] = _blocks.saveBlock(_data+(BLOCK_SIZE*i), _sizeofData>=BLOCK_SIZE?BLOCK_SIZE:_sizeofData);
		_sizeofData-=BLOCK_SIZE;
		if (_sizeofData<=0) return;
	}
	Inodes[index].sipointer = _pointers.saveDirect(_data+(BLOCK_SIZE*4),_sizeofData,_blocks);
	_sizeofData-=(BLOCK_SIZE*POINTERSPERBLOCKQTY);
	if(_sizeofData<=0) return;
	Inodes[index].dipointer = _pointers.saveIndirect(_data+(BLOCK_SIZE*4 + BLOCK_SIZE*POINTERSPERBLOCKQTY), _sizeofData, _blocks);
}

void Inodos::deleteInodo(int _index, Pointers &_pointers, Block &_blocks) {
	Inodes[_index].used = false;
	int sizeofData = Inodes[_index].size;
	
	if(Inodes[_index].isFile)
	{
		for(int i=0;i<4;i++)
		{
			_blocks.deleteBlock(Inodes[_index].directs[i]);
			sizeofData -= BLOCK_SIZE;
			if(sizeofData<=0) return;
		}
		_pointers.deleteDirect(Inodes[_index].sipointer, _blocks, sizeofData);
		sizeofData -= (BLOCK_SIZE*POINTERSPERBLOCKQTY);
		if(sizeofData<=0) return;
		
		_pointers.deleteIndirect(Inodes[_index].dipointer, _blocks, sizeofData);
	}
}

void Inodos::catInodo(int _index, Pointers &_pointers, Block &_blocks) 
{
	int sizeofData = Inodes[_index].size;
	
	if(Inodes[_index].isFile)
	{
		for(int i=0;i<4;i++)
		{
			_blocks.catBlock(Inodes[_index].directs[i],sizeofData);
			sizeofData -= BLOCK_SIZE;
			if(sizeofData<=0) return;
		}
		_pointers.catDirect(Inodes[_index].sipointer, _blocks, sizeofData);
		sizeofData -= (BLOCK_SIZE*POINTERSPERBLOCKQTY);
		if(sizeofData<=0) return;
		
		_pointers.catIndirect(Inodes[_index].dipointer, _blocks, sizeofData);
	}
}

int Inodos::next_free() 
{
	for(int i=0; i<INODESQTY; i++)
		if(!(Inodes[i].used))
			return i;
}

void Inodos::moveInodo(int _index, int _indexNewFather) 
{
	Inodes[_index].father = _indexNewFather;
}

Inodo Inodos::getInodo(int _index) {
	return Inodes[_index];
}

void Inodos::saveToFile(const char *_path) {
	ofstream fs(_path, ios::binary);
	
	for(int i=0;i<INODESQTY;i++)
	{
		fs.write((const char*)&Inodes[i].ownerid, sizeof(int));
		fs.write((const char*)&Inodes[i].groupid, sizeof(int));
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

void Inodos::changeFileMode	(int _index, int _newFileMode) {
	Inodes[_index].file_mode = _newFileMode;
}

void Inodos::getInodoInfo() {
	cout<<"INODES USED: "<<endl;
	for(int i=0;i<INODESQTY;i++) 
	{ 
		if (Inodes[i].used)
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

Inodos::~Inodos() {
	
}

