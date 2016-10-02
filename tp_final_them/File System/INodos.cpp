#include "INodos.h"
#include <cstring>


INodos::INodos(int cantINod, char * path)
{
	inodos = (INodo*)malloc(cantINod*sizeof(INodo));
	
	cantINodos = cantINod;
	
	std::ifstream fs(path, std::ios::binary);
	
	for(int i = 0; i < cantINodos; i++)
	{
		inodos[i].Creador = (char*)malloc(10*sizeof(char));
		inodos[i].Directos = (int*)malloc(4*sizeof(int));
		inodos[i].Nombre = (char*)malloc(10*sizeof(char));
		
		fs.read(inodos[i].Creador, 10*sizeof(char));
		fs.read((char*)&inodos[i].Permisos, sizeof(int));
		fs.read(inodos[i].Nombre, 10*sizeof(char));
		
		fs.read((char*)&inodos[i].Padre, sizeof(int));
		fs.read((char*)&inodos[i].Size, sizeof(int));
		
		fs.read((char*)&inodos[i].IsFile, sizeof(bool));
		fs.read((char*)&inodos[i].Usado, sizeof(bool));
		
		fs.read((char*)inodos[i].Directos, 4*sizeof(int));
		fs.read((char*)&inodos[i].IndSimple, sizeof(int));
		fs.read((char*)&inodos[i].IndDoble, sizeof(int));
	}
	
	fs.close();
}

void INodos::CrearDirectorio(const char* nomb, int perm, char * creat, int pad)
{
	int index = SiguienteLibre();
	
	strcpy(inodos[index].Nombre, nomb);
	inodos[index].Permisos = perm;
	inodos[index].Creador = creat;
	inodos[index].Padre = pad;
	inodos[index].IsFile = false;
	inodos[index].Usado = true;
	inodos[index].Size = 0;
}

void INodos::CrearArchivo(const char* nomb, int perm, char* creat, int pad,
						  byte* data, int sizeOfData, Punteros &punteros,
						  Bloques &bloques)
{
	
	if(sizeOfData > 6144)
	{
		std::cout<<"Tamaño de archivo invalido";
		return;
	}
	
	int index = SiguienteLibre();
	
	strcpy(inodos[index].Nombre, nomb);
	inodos[index].Permisos = perm;
	inodos[index].Creador = creat;
	inodos[index].Padre = pad;
	inodos[index].IsFile = true;
	inodos[index].Usado = true;
	inodos[index].Size = sizeOfData;
	
	for(int i=0; i<4; i++)
	{
		inodos[index].Directos[i] = 
			bloques.SaveBlock(data+(256*i), sizeOfData >= 256 ? 256 : sizeOfData);
		
		sizeOfData -= 256;
		if(sizeOfData <= 0) return;
	}
	if(sizeOfData > 1024)
	{
		inodos[index].IndSimple = punteros.SaveDirect(data, 1024, bloques);
		sizeOfData -= 1024;
	}
	else
	{
		inodos[index].IndSimple = punteros.SaveDirect(data, sizeOfData, bloques);
		return;
	}
	
	inodos[index].IndDoble = punteros.SaveIndirect(data, sizeOfData, bloques);
}

void INodos::EliminarINodo(int index, Bloques &bloques, Punteros &punteros) 
{
	inodos[index].Usado = false;
	int sizeOfData = inodos[index].Size;
	
	//Marcas como no usado todos los bloques y punteros.
	if(inodos[index].IsFile)
	{
		for(int i=0; i<4; i++)
		{
			bloques.DeleteBlock(inodos[index].Directos[i]);
			sizeOfData -= 256;
			if(sizeOfData <= 0) return;
		}
		
		punteros.DeleteDirect(inodos[index].IndSimple, bloques, sizeOfData);
		sizeOfData -= 1024;
		if(sizeOfData <= 0) return;
		
		punteros.DeleteIndirect(inodos[index].IndDoble, bloques, sizeOfData);
	}
}

int INodos::SiguienteLibre()
{
	for(int i = 0; i < cantINodos; i++)
		if(!(inodos[i].Usado))
			return i;
}

void INodos::MoverINodo(int indexMov, int indexNuevoPadre)
{
	inodos[indexMov].Padre = indexNuevoPadre;
}

INodo INodos::GetINodo(int index)
{
	return inodos[index];
}

void INodos::SaveToFile(char * path)
{
	std::ofstream fs(path, std::ios::binary);
	
	for(int i = 0; i < cantINodos; i++)
	{
		fs.write(inodos[i].Creador, 10*sizeof(char));
		fs.write((const char*)&inodos[i].Permisos, sizeof(int));
		fs.write(inodos[i].Nombre, 10*sizeof(char));
		
		fs.write((const char*)&inodos[i].Padre, sizeof(int));
		fs.write((const char*)&inodos[i].Size, sizeof(int));
		
		fs.write((const char*)&inodos[i].IsFile, sizeof(bool));
		fs.write((const char*)&inodos[i].Usado, sizeof(bool));
		
		fs.write((const char*)inodos[i].Directos, 4*sizeof(int));
		fs.write((const char*)&inodos[i].IndSimple, sizeof(int));
		fs.write((const char*)&inodos[i].IndDoble, sizeof(int));
	}
	
	fs.close();
}

void INodos::CambiarPadre(int nodo, int nuevoPadre)
{
	inodos[nodo].Padre = nuevoPadre;
}

