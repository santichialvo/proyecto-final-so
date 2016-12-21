#include "FileSystem.h"

FileSystem::FileSystem() 
{
	blocks = new Block(_TEMP_DIR"block_data.bin");
	pointers = new Pointers(_TEMP_DIR"pointer_data.bin");
	Inodes = new Inodos(_TEMP_DIR"inode_data.bin");
	
	User root,santiago;
	strcpy(root.userName,"root");
	root.UserID = 0;
	strcpy(santiago.userName,"santiago");
	santiago.UserID = 1;
	users = new User[2];
	users[0] = root;
	users[1] = santiago;
	
	currentUserID=currentGroupUserID=0; 								//root user
	Inodes->mkdir("root",0x1C0,currentUserID,currentGroupUserID,-1); 	//root 111000000
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
	string currentuser = users[currentUserID].userName;
	path = currentuser+"@"+currentuser+":"+path;
	cout<<path<<"$ ";
}

void FileSystem::listChildren(int _mode)
{
	if (!getPermission(actualInode,0))
	{
		cout<<"No tiene permisos para listar este directorio"<<endl;
		return;
	}
	for(int i=0;i<INODESQTY;i++)
	{
		if(Inodes->getInodo(i).father == actualInode 
			&& Inodes->getInodo(i).used)
		{
			if(!(Inodes->getInodo(i).isFile)) //Si es dir muestro verde, sino amarillo
				rlutil::setColor(2);
			else
				rlutil::setColor(14);
			if (_mode==0)
				cout<<Inodes->getInodo(i).name<<endl;
			else if (_mode==1)
				cout<<showFileMode(Inodes->getInodo(i).file_mode,2)<<
				showFileMode(Inodes->getInodo(i).file_mode,1)<<
				showFileMode(Inodes->getInodo(i).file_mode,0)<<" "<<
				users[currentUserID].userName<<
				" "<<Inodes->getInodo(i).name<<endl;
			rlutil::setColor(15);
		}
	}
}

int FileSystem::showFileMode(int _filemode,int _mode)
{
	int number = _mode==0?7:_mode==1?7<<3:7<<6;
	int offset = _mode==0?0:_mode==1?3:6;
	return (_filemode&number)>>offset;
}

bool FileSystem::verifyDirName(const char * _path, int _actualInode)
{
	for(int i=0;i<INODESQTY;i++)
	{
		if(Inodes->getInodo(i).father == _actualInode 
			&& Inodes->getInodo(i).used)
		{
			if(!(Inodes->getInodo(i).isFile))
				if (!strcmp(Inodes->getInodo(i).name,_path))
					return false;
		}
	}
	return true;
}

//Recursive listing
void FileSystem::listChildren(int _inode, int _level)
{
	for(int i=0; i<INODESQTY;i++)
	{
		if(Inodes->getInodo(i).father == _inode 
			&& Inodes->getInodo(i).used)
		{
			cout<<"|";
			for(int j = 0; j < _level; j++) cout<<"-";
			cout << Inodes->getInodo(i).name << endl;
			if(!Inodes->getInodo(i).isFile)
				listChildren(i, _level+1);
		}
	}
}

/// Checkea los permisos de lectura, escritura y ejecucion
bool FileSystem::getPermission(int _inode, int _mode)
{
	int perm = _mode==0?4:_mode==1?2:1;
	
	if(currentUserID==Inodes->getInodo(_inode).ownerid) 		//OWNER
	{
		perm=perm<<6;
	}
	else if (currentUserID==Inodes->getInodo(_inode).groupid) 	//GROUP
	{
		perm=perm<<3;
	}
	else 														//OTHERS
	{
		perm=perm<<0;
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
	if (!getPermission(inode,2)&&inode!=-1)
	{
		cout<<"No tiene permisos para acceder a este directorio"<<endl;
		return;
	}
	if(inode==-1 || Inodes->getInodo(inode).isFile)
		cout<<"No existe el directorio"<<endl;
	else
		actualInode = inode;
}

int FileSystem::searchOnInode(string _name, int _inode)
{
	for(int i=0;i<INODESQTY;i++)
	{
		if(Inodes->getInodo(i).father == _inode 
			&& Inodes->getInodo(i).used)
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
		makeFile(_path.c_str(), actualInode, 0x1ff, _data, _size);
	}
	else {
		int father = stringToInode(_path.substr(0, _path.find_last_of('/')));
		makeFile(_path.substr(_path.find_last_of('/') + 1, string::npos), father, 0x1ff, _data, _size);
	}
}

void FileSystem::makeFile(string _name, int _father, int _permission, byte* _data, int _sizeofData)
{
	if(!getPermission(_father,1)&&!getPermission(_father,2))
	{
		cout<<"No tiene permiso para editar esta carpeta"<<endl;
		return;
	}
	if (searchOnInode(_name,_father)!=-1)
	{
		cout<<"touch: No se puede crear el archivo "+_name+": El archivo ya existe"<<endl;
		return;
	}
	Inodes->mkfile(_name.c_str(), _permission, currentUserID,currentGroupUserID, _father, _data, _sizeofData, *pointers, *blocks);
}

void FileSystem::makeDir(string _name, int _father, int _permission)
{
	if(!getPermission(_father,1)&&!getPermission(_father,2))
	{
		cout<<"No tiene acceso para editar esta carpeta"<<endl;
		return;
	}
	Inodes->mkdir(_name.c_str(), _permission, currentUserID,currentGroupUserID, _father);
}

void FileSystem::del(int _inode)
{
	if (_inode==-1)
	{
		cout<<"El archivo/directorio no existe"<<endl;
		return;
	}
	
	if(!getPermission(_inode,1)&&!getPermission(_inode,2))
	{
		cout<<"No tiene permisos para borrar este directorio o archivo"<<endl;
		return;
	}
	
	if(!Inodes->getInodo(_inode).isFile)
		for(int i=0;i<INODESQTY;i++)
			if(Inodes->getInodo(i).father == _inode && Inodes->getInodo(i).used)
				del(i);
	
	Inodes->deleteInodo(_inode,*pointers,*blocks);
}

void FileSystem::cat(int _inode)
{
	if (_inode==-1)
	{
		cout<<"El archivo/directorio no existe"<<endl;
		return;
	}
	
	if(!getPermission(_inode,1)&&!getPermission(_inode,2))
	{
		cout<<"No tiene permisos para borrar este directorio o archivo"<<endl;
		return;
	}
	
	if(!Inodes->getInodo(_inode).isFile)
	{
		cout<<"No es un archivo"<<endl;
		return;
	}
	Inodes->catInodo(_inode,*pointers,*blocks);
}

void FileSystem::showTree()
{
	cout<<"|"<<Inodes->getInodo(0).name<<endl;
	listChildren(0,1);
}

void FileSystem::changeUser(int _new)
{
	currentUserID = _new;
}

void FileSystem::makeDir(string _path)
{
	if(_path.find('/') == string::npos) {
		if (verifyDirName(_path.c_str(),actualInode))
			makeDir(_path.c_str(), actualInode, 0x1ff);
		else
			cout<<"mkdir: No se puede crear el directorio "<<_path<<": El archivo ya existe"<<endl;
	}
	else {
		int father = stringToInode(_path.substr(0, _path.find_last_of('/')));
		makeDir(_path.substr(_path.find_last_of('/') + 1, string::npos), father, 0x1ff);
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

void FileSystem::show(string _path)
{
	cat(stringToInode(_path));
}

void FileSystem::save_blocks(string _path)
{
	blocks->saveToFile(_path.c_str());
}

void FileSystem::save_pointers(string _path)
{
	pointers->saveToFile(_path.c_str());
}

void FileSystem::save_inodes(string _path)
{
	Inodes->saveToFile(_path.c_str());
}

void FileSystem::chmod(int _newmode, string _path)
{
	int inode = stringToInode(_path);
	if(inode == -1)
	{
		cout<<"Direccion invalida"<<endl;
		return;
	}
	Inodes->changeFileMode(inode,_newmode);
	
}

void FileSystem::show_BlockInfo()
{
	blocks->getBlockInfo();
}

void FileSystem::show_PointerInfo()
{
	pointers->getPointerInfo();
}

void FileSystem::show_InodesInfo()
{
	Inodes->getInodoInfo();
}

FileSystem::~FileSystem() {
	
}

