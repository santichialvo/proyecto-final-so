#include "FileSystem.h"

FileSystem::FileSystem() 
{
	blocks = new Block(256,"default_block_data.bin");
	pointers = new Pointers("default_pointer_data.bin");
	Inodes = new Inodos(500,"default_inode_data.bin");
	
	Inodes->mkdir("root",0x01b6,"",-1); //root
	group=ADMIN;
	actualInode=0;
}

void FileSystem::WritePrompt()
{
	int Inode=actualInode;
	string path(!Inode?"/":"");
	while(Inode != 0)
	{
		path = Inodes->getInodo(Inode).name+path;
		path = "/"+path;
		Inode = Inodes->getInodo(Inode).father;
	}
	path = "user@user:"+path;
	cout<<path<<"$ ";
}

void FileSystem::listChildren()
{
	for(int i=0;i<INODESQTY;i++)
	{
		if(Inodes->getInodo(i).father == actualInode 
			&& Inodes->getInodo(i).used
			&& getPermission(i,0)) //TODO: Para mostrarlo tenes que tener read permisions?
		{
			if(!(Inodes->getInodo(i).isFile)) //Si es dir muestro verde, sino amarillo
				rlutil::setColor(2);
			else
				rlutil::setColor(14);
			cout<<Inodes->getInodo(i).name<<endl;
			rlutil::setColor(15);
		}
	}
}

//Recursive listing
void FileSystem::listChildren(int _inode, int _level)
{
	for(int i=0; i<INODESQTY;i++)
	{
		if(Inodes->getInodo(i).father == _inode 
			&& Inodes->getInodo(i).used
			&& getPermission(i,0))
		{
			cout<<"|";
			for(int j = 0; j < _level; j++) cout<<"-";
			cout << Inodes->getInodo(i).name << endl;
			if(!Inodes->getInodo(i).isFile)
				listChildren(i, _level+1);
		}
	}
}

/// Checkea los permisos de lectura, ejecucion y escritura
bool FileSystem::getPermission(int _inode, int _mode)
{
	int perm = _mode==0?4:_mode==1?2:1;
	switch(group)
	{
	case ADMIN:
		{
			perm=perm<<7;
			break;
		}
	case USER:
		{
			perm=perm<<3;
			break;
		}
		//TODO: y para GUEST?
	}
	return Inodes->getInodo(_inode).file_mode&perm;
}

void FileSystem::increaseLevel()
{
	if(actualInode != 0)
		actualInode = Inodes->getInodo(actualInode).father;
}

void FileSystem::changeDir(string _dir)
{
	int inode = stringToInode(_dir);
	if(inode==-1 || Inodes->getInodo(actualInode).isFile) //TODO: porque chequea si el actual es archivo?
		cout<<"No existe el directorio"<<endl;
	else
		actualInode = inode;
}

int FileSystem::searchOnInode(string _name, int _inode)
{
	for(int i=0;i<INODESQTY;i++)
	{
		if(Inodes->getInodo(i).father == _inode 
			&& Inodes->getInodo(i).used
			&& getPermission(i,2))
		{
			if(!_name.compare(Inodes->getInodo(i).name))
				return i;
		}
	}
	
	return -1;
}

int FileSystem::stringToInode(string _command)
{
	if(*_command.end() != '/')
		_command.push_back('/');
	
	if(_command[0] == '/')
		_command = _command.substr(1, string::npos);
	
	int pos=0;
	list<string> dir;
	for(int i=0; i<_command.length(); i++) {
		if(_command[i] == '/') {
			string s(_command, pos, i-pos);
			dir.push_back(s);
			pos=i+1;
		}
	}
	if(dir.begin()->empty()) //TODO por que podria estar vacio?
		dir.pop_back();
	
	int nodeReturn = actualInode;
	list<string>::iterator it = dir.begin();
	while(it != dir.end())
	{
		nodeReturn = searchOnInode(*it, nodeReturn);
		if(nodeReturn == -1) break;
		it++;
	}
	
	if(nodeReturn != -1)
		return nodeReturn;
	
	nodeReturn = 0;
	if(dir.begin()->compare("root")==0)
		dir.pop_front();
	it = dir.begin();
	while(it != dir.end()) {
		nodeReturn = searchOnInode(*it, nodeReturn);
		if(nodeReturn == -1) break;
		it++;
	}
	
	return nodeReturn;
}

void FileSystem::makeFile(string _path, unsigned char* _data, int _size)
{
	if(_path.find('/') == string::npos) {
		makeFile(_path.c_str(), actualInode, 0xffff, _data, _size);
	}
	else {
		int father = stringToInode(_path.substr(0, _path.find_last_of('/')));
		makeFile(_path.substr(_path.find_last_of('/') + 1, string::npos), father, 0xffff, _data, _size);
	}
}

void FileSystem::makeFile(string _name, int _father, int _permission, byte* _data, int _sizeofData)
{
	if(!getPermission(_father,2))
	{
		cout<<"No tiene permiso para editar esta carpeta"<<endl;
		return;
	}
	Inodes->mkfile(_name.c_str(), _permission, "", _father, _data, _sizeofData, *pointers, *blocks);
}

void FileSystem::makeDir(string _name, int _father, int _permission)
{
	if(!getPermission(_father,2))
	{
		cout<<"No tiene acceso para editar esta carpeta"<<endl;
		return;
	}
	Inodes->mkdir(_name.c_str(), _permission, "", _father);
}

void FileSystem::del(int _inode)
{
	if(!getPermission(_inode,1))
	{
		cout<<"No tiene permiso para borrar este directorio o archivo"<<endl;
		return;
	}
	
	if(!Inodes->getInodo(_inode).isFile)
		for(int i=0;i<INODESQTY;i++)
			if(Inodes->getInodo(i).father == _inode && Inodes->getInodo(i).used)
				del(i);
	
	Inodes->deleteInodo(_inode,*pointers,*blocks);
}

void FileSystem::showTree()
{
	cout<<"|"<<Inodes->getInodo(0).name<<endl;
	listChildren(0,1);
}

void FileSystem::changeGroup(Group _new)
{
	group = _new;
}

void FileSystem::makeDir(string _path)
{
	if(_path.find('/') == string::npos) {
		makeDir(_path.c_str(), actualInode, 0xffff);
	}
	else {
		int father = stringToInode(_path.substr(0, _path.find_last_of('/')));
		makeDir(_path.substr(_path.find_last_of('/') + 1, string::npos), father, 0xffff);
	}
}

void FileSystem::move(string _from, string _to)
{
	int inode = stringToInode(_from);
	int dest = stringToInode(_to);
	
	if(inode == -1 || dest == -1)
		cout<<"Direccion invalida"<<endl;
	else
		Inodes->changeFather(inode, dest);
}

void FileSystem::erase(string _path)
{
	del(stringToInode(_path));
}

FileSystem::~FileSystem() {
	
}

