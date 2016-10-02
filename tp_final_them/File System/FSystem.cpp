#include "FSystem.h"

///----------------------------------------------------------------------------/
FSystem::FSystem()
{
	bloques = new Bloques(256, "default_block_data.bin");
	punteros = new Punteros("default_pointer_data.bin");
	inodos = new INodos(500, "default_inode_data.bin");
	
	inodos->CrearDirectorio("root", 0x01b6, "", -1);
}
///----------------------------------------------------------------------------/
void FSystem::CrearArchivo(string name, int nodoPadre, int perm, byte * data, int sizeOfData)
{
	if(!CheckPermisoEditar(nodoPadre))
	{
		cout << "No tiene acceso para editar esta carpeta." << endl;
		return;
	}
	
	inodos->CrearArchivo(name.c_str(), perm, "", nodoPadre, data, sizeOfData, *punteros, *bloques);
}
///----------------------------------------------------------------------------/
void FSystem::CrearDirectorio(string name, int nodoPadre, int perm)
{
	if(!CheckPermisoEditar(nodoPadre))
	{
		cout << "No tiene acceso para editar esta carpeta." << endl;
		return;
	}
	
	inodos->CrearDirectorio(name.c_str(), perm, "", nodoPadre);
}
///----------------------------------------------------------------------------/
void FSystem::Borrar(int node)
{
	if(!CheckPermisoBorrar(node))
	{
		cout << "No tiene permiso de borrar este directorio o archivo." << endl;
		return;
	}
	
	if(!inodos->GetINodo(node).IsFile)
		for(int i = 0; i < CANT_INODOS; i++)
			if(inodos->GetINodo(i).Padre == node && inodos->GetINodo(i).Usado)
				Borrar(i);
		
	inodos->EliminarINodo(node, *bloques, *punteros);
}
///----------------------------------------------------------------------------/
void FSystem::MostrarArbol()
{
	cout << inodos->GetINodo(0).Nombre << endl;
	ListarHijos(0, 1);
}
/// Elimina las estructuras de datos
FSystem::~FSystem()
{
	delete(inodos);
	delete(punteros);
	delete(bloques);
}
/// Lista los hijos de un nodo de forma 
void FSystem::ListarHijos(int node, int nivel)
{
	for(int i = 0; i < CANT_INODOS; i++)
	{
		if(inodos->GetINodo(i).Padre == node 
		   && inodos->GetINodo(i).Usado
		   && CheckPermisoLeer(i))
		{
			for(int j = 0; j < nivel; j++) cout << "-";
			cout << inodos->GetINodo(i).Nombre << endl;
			if(inodos->GetINodo(i).IsFile == false)
				ListarHijos(i, nivel+1);
		}
	}
}
/// Checkea los permisos de lectura ubicados en la tercera bander (Rxw)
bool FSystem::CheckPermisoLeer(int node)
{
	int perm = 4;
	
	switch(grupoActual)
	{
	case ADMIN:
	{
		perm = perm << 7;
		break;
	}
	case USER:
	{
		perm = perm << 3;
		break;
	}
	}
	
	return inodos->GetINodo(node).Permisos & perm;
}
/// Checkea los permisos de borrado ubicados en la segunda bander (rXw)
bool FSystem::CheckPermisoBorrar(int node)
{
	int perm = 2;
	
	switch(grupoActual)
	{
	case ADMIN:
	{
		perm = perm << 7;
		break;
	}
	case USER:
	{
		perm = perm << 3;
		break;
	}
	}
	
	return inodos->GetINodo(node).Permisos & perm;
}
/// Checkea los permisos de escritura ubicados en la primera bander (rxW)
bool FSystem::CheckPermisoEditar(int node)
{
	int perm = 1;
	
	switch(grupoActual)
	{
	case ADMIN:
	{
		perm = perm << 7;
		break;
	}
	case USER:
	{
		perm = perm << 3;
		break;
	}
	}
	
	return inodos->GetINodo(node).Permisos & perm;
}
/// Setea el grupo de usuarios actual.
void FSystem::CambiarGrupo(Grupo nuevo)
{
	grupoActual = nuevo;
}

void FSystem::EscribirPrompt()
{
	int node = nodoActual;
	string path("/");
	while(node != 0)
	{
		path = inodos->GetINodo(node).Nombre + path;
		path = "/" + path;
		node = inodos->GetINodo(node).Padre;
	}
	cout << path << ">";
}

void FSystem::ListarHijos()
{
	for(int i = 0; i < CANT_INODOS; i++)
	{
		if(inodos->GetINodo(i).Padre == nodoActual 
		   && inodos->GetINodo(i).Usado
		   && CheckPermisoLeer(i))
		{
			cout << inodos->GetINodo(i).Nombre;
			if(!(inodos->GetINodo(i).IsFile))
				cout << " <dir>";
			
			cout << endl;
		}
	}
}
///----------------------------------------------------------------------------/
void FSystem::SubirNivel()
{
	if(nodoActual != 0)
	{
		nodoActual = inodos->GetINodo(nodoActual).Padre;
	}
}
///----------------------------------------------------------------------------/
void FSystem::CambiarDirectorioActual(string dir)
{
	int node = StringToNode(dir);
	if(node == -1 || inodos->GetINodo(nodoActual).IsFile)
		cout << "Direccion invalida" << endl;
	else
		nodoActual = node;
}
///----------------------------------------------------------------------------/
void FSystem::CrearArchivo(string filePath, unsigned char * data, int size)
{
	if(filePath.find('/') == string::npos)
	{
		CrearArchivo(filePath.c_str(), nodoActual, 0xffff, data, size);
	}
	else
	{
		int nodoPadre = StringToNode(filePath.substr(0, filePath.find_last_of('/')));
		CrearArchivo(filePath.substr(filePath.find_last_of('/') + 1, string::npos), nodoPadre, 0xffff, data, size);
	}
	
}
///----------------------------------------------------------------------------/
void FSystem::CrearDirectorio(string filePath)
{
	if(filePath.find('/') == string::npos)
	{
		CrearDirectorio(filePath.c_str(), nodoActual, 0xffff);
	}
	else
	{
		int nodoPadre = StringToNode(filePath.substr(0, filePath.find_last_of('/')));
		CrearDirectorio(filePath.substr(filePath.find_last_of('/') + 1, string::npos), nodoPadre, 0xffff);
	}
}
///----------------------------------------------------------------------------/
void FSystem::Mover(string from, string to)
{
	int nodo = StringToNode(from);
	int dest = StringToNode(to);
	
	if(nodo == -1 || dest == -1)
		cout << "Direccion invalida" << endl;
	else
		inodos->CambiarPadre(nodo, dest);
}
///----------------------------------------------------------------------------/
void FSystem::Eliminar(string path)
{
	Borrar(StringToNode(path));
} 
///----------------------------------------------------------------------------/
int FSystem::StringToNode(string comando)
{
	if(*comando.end() != '/')
		comando.push_back('/');
	
	if(comando[0] == '/')
		comando = comando.substr(1, string::npos);
	
	int pos = 0;
	list<string> dir;
	for(int i = 0; i<comando.length(); i++)
	{
		if(comando[i] == '/')
		{
			string s(comando, pos, i - pos);
			dir.push_back(s);
			
			pos = i + 1;
		}
	}
	
	if(dir.begin()->empty())
		dir.pop_back();
	
	
	int nodeReturn = nodoActual;
	list<string>::iterator it = dir.begin();
	while(it != dir.end())
	{
		nodeReturn = BuscarEnNodo(*it, nodeReturn);
		if(nodeReturn == -1) break;
		it++;
	}
	
	if(nodeReturn != -1)
		return nodeReturn;
	
	nodeReturn = 0;
	if(dir.begin()->compare("root") == 0) dir.pop_front();
		it = dir.begin();
	while(it != dir.end())
	{
		nodeReturn = BuscarEnNodo(*it, nodeReturn);
		if(nodeReturn == -1) break;
		it++;
	}
	
	return nodeReturn;
}
///----------------------------------------------------------------------------/
int FSystem::BuscarEnNodo(string name, int nodo)
{
	for(int i = 0; i < CANT_INODOS; i++)
	{
		if(inodos->GetINodo(i).Padre == nodo 
		   && inodos->GetINodo(i).Usado
		   && CheckPermisoEditar(i))
		{
			if(name.compare(inodos->GetINodo(i).Nombre) == 0)
				return i;
		}
	}
	
	return -1;
}
