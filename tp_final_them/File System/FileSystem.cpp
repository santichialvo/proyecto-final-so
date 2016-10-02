#include "FileSystem.h"

void FileSystem::Cd()
{
	
}

void FileSystem::CdUp()
{
	if(nodoActual != 0)
		nodoActual = inodos->GetINodo(nodoActual).Padre;
}

void FileSystem::CdDown(string name)
{
	
}

void FileSystem::Ls(string name)
{
	for(int i = 0; i < 500; i++)
	{
		if(inodos->GetINodo(i).Usado)
		{
			cout << inodos->GetINodo(i).Nombre << endl;
		}
	}
}
///
void FileSystem::Run()
{
	cout<<"File system - Sistemas Operativos - Ivan Dellizzotti" << endl << endl;
	bool end = true;
	while(end)
	{
		EscribirPrompt();
		
		char comando[150];
		cin.getline(comando, 150);
		
		int startCommand = 0;
		while(comando[startCommand] == ' ')
			startCommand++;
		
		///Check cds
		if(comando[startCommand] == 'c' &&
		   comando[startCommand+1] == 'd')
		{
			if((comando[startCommand+2] == '.' &&
				comando[startCommand+3] == '.' &&
				comando[startCommand+4] == '\0') ||
			   (comando[startCommand+2] == ' ' &&
				comando[startCommand+3] == '.' &&
				comando[startCommand+4] == '.' &&
				comando[startCommand+5] == '\0'))
			{
				CdUp();
				continue;
			}
			
			if(comando[startCommand+2] == '\0')
			{
				Cd();
				continue;
			}
			
			if(comando[startCommand+2] == ' ')
			{
				string s(comando + startCommand + 3, 150 - startCommand - 3);
				CdDown(s.substr(0, s.find_first_of('\0', 0)));
				continue;
			}
		}
		
		if(comando[startCommand] == 'l' &&
		   comando[startCommand+1] == 's' &&
		   (comando[startCommand+2] == '\0' || 
		   comando[startCommand+2] == ' '))
		{
			string s(comando + startCommand + 3, 150 - startCommand - 3);
			Ls(s.substr(0, s.find_first_of('\0', 0)));
			continue;
		}
		
		if(comando[startCommand] == 'c' &&
		   comando[startCommand+1] == 'f' &&
		   comando[startCommand+2] == ' ')
		{
			string s(comando + startCommand + 3, 150 - startCommand - 3);
			CrearArchivo(s.substr(0, s.find_first_of('\0', 0)));
			continue;
		}
		
		if(comando[startCommand] == 'm' &&
		   comando[startCommand+1] == 'd' &&
		   comando[startCommand+2] == ' ')
		{
			string s(comando + startCommand + 3, 150 - startCommand - 3);
			CrearDirectorio(s.substr(0, s.find_first_of('\0', 0)));
			continue;
		}
		
		if(comando[startCommand] == 's' &&
		   comando[startCommand+1] == 'f' &&
		   comando[startCommand+2] == ' ')
		{
			string s(comando + startCommand + 3, 150 - startCommand - 3);
			ShowFile(s.substr(0, s.find_first_of('\0', 0)));
			continue;
		}
		
		if(comando[startCommand] == 'l' &&
		   comando[startCommand+1] == 'f' &&
		   comando[startCommand+2] == ' ')
		{
			string s(comando + startCommand + 3, 150 - startCommand - 3);
			LoadFile(s.substr(0, s.find_first_of('\0', 0)));
			continue;
		}
		
		if(comando[startCommand] == 's' &&
		   comando[startCommand+1] == 't' &&
		   comando[startCommand+2] == 'f' &&
		   comando[startCommand+3] == ' ')
		{
			string s(comando + startCommand + 4, 150 - startCommand - 4);
			ShowFile(s.substr(0, s.find_first_of('\0', 0)));
			continue;
		}
	}
	
}
/// Crea las estructuras de datos a partir de los archivos
FileSystem::FileSystem()
{
	bloques = new Bloques(256, "default_block_data.bin");
	punteros = new Punteros("default_pointer_data.bin");
	inodos = new INodos(500, "default_inode_data.bin");
	
	inodos->CrearDirectorio("root", 0, "", -1);
	nodoActual = 0;
}
/// Escribe el mensaje del prompt
void FileSystem::EscribirPrompt()
{
	cout << NodoToPath(nodoActual) << "> ";
}
/// Guarda los datos a archivos y libera la memoria
FileSystem::~FileSystem()
{
	delete(inodos);
	delete(punteros);
	delete(bloques);
}
/// Crea un nuevo directorio en el FileSystem
void FileSystem::CrearDirectorio(string name)
{
	if(name.length() < 1)
	{
		cout << "No se puede crear archivo";
		return;
	}
	else if(FindNodo(name, nodoActual) == -1)
	{
		cout << "El nombre de archivo ya existe en el directorio actual";
		return;
	}
	inodos->CrearDirectorio(name.c_str(), 0, "", nodoActual);
}
/// Abre un archivo y lo muestra en pantalla
void FileSystem::ShowFile(string name)
{
	cout<< "showfile: " << name << endl;
}
/// Crea un archivo en el FileSystem
void FileSystem::CrearArchivo(string name)
{
	int pos = name.find_last_of('/');
	if(pos != string::npos)
	{
		string s(name, 0, pos-1);
		int nodo = PathToNodo(s, -1);
		inodos->CrearArchivo(name.substr(pos+1), nodo);
	}
	else
	{
		inodos->CrearArchivo(name, nodoActual);
	}
}
/// Dado el string del path devuelve el nodo correspondiente
int FileSystem::PathToNodo(string path, int nodoPadre)
{
	int pos = path.find_first_of('/', path.length());
	if(pos == string::npos)
		return FindNodo(path);
	else
		return PathToNodo(new string(path, pos+1),
						  FindNodo(new string(path, 0, pos-1), nodoPadre));
}
/// Dado un nodo devuelve el string del path
string FileSystem::NodoToPath(int node)
{
	string s("");
	string conc("");
	
	INodo inodo = inodos->GetINodo(node);
	conc.append(inodo.Nombre);
	s = conc;
	
	while(inodo.Padre != -1)
	{
		inodo = inodos->GetINodo(inodo.Padre);
		
		s = conc + "/" + s;
	}
	return s;
}
/// Carga el archivo en el path name al FileSystem
void FileSystem::LoadFile(string name)
{
	cout << "LoadFile: " << name << endl;
}
/// Toma el archivo en el path de FileSystem y lo guarda
void FileSystem::SaveToFile(string name)
{
	cout << "SaveToFile: " << name << endl;
}
/// Busca la primera coincidencia en el directorio especificado
int FileSystem::FindNodo(string name, int padre)
{
	for(int i = 0; i < 500; i ++)
	{
		if(inodos->GetINodo(i).Padre == padre)
		{
			string s(inodos->GetINodo(i).Nombre);
			if(s.compare(0, 10, s, 0, 10);
				return i;
		}
	}
	
	return -1;
}

