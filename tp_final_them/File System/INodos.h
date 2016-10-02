#ifndef INODOS_H
#define INODOS_H

#include <cstdlib>
#include <iostream>

#include "Bloques.h"
#include "Punteros.h"

struct INodo
{
	char *Creador;
	int Permisos;
	char *Nombre;
	int Padre;
	int Size;
	
	bool IsFile;
	bool Usado;
	
	int *Directos;
	int IndSimple;
	int IndDoble;
};

class INodos {
private:
	
	int cantINodos;
	
	INodo *inodos;
	
	int SiguienteLibre();
	
public:
	
	INodos(int cantINod, char* path);
	
	void CrearDirectorio(const char* nomb, int perm, char* creat, int pad);
	void CrearArchivo(const char* nomb, int perm, char* creat, int pad, 
					  byte* data, int sizeOfData, Punteros &punteros, Bloques &bloques);
	
	unsigned char* GetData(int nodo);
	
	void EliminarINodo(int index, Bloques &bloques, Punteros &punteros);
	void MoverINodo(int indexMov, int indexNuevoPadre);
	
	INodo GetINodo(int index);
	
	void CambiarPadre(int nodo, int nuevoPadre);
	
	void SaveToFile(char* path);
	
};

#endif

