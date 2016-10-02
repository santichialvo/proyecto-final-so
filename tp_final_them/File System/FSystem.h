#ifndef FSYSTEM_H
#define FSYSTEM_H

#include <iostream>
#include <list>

#include "Bloques.h"
#include "Punteros.h"
#include "INodos.h"

using namespace std;

enum Grupo{
	ADMIN,
	USER,
	GUEST
};

class FSystem {
private:
	
	Grupo grupoActual = ADMIN;
	int nodoActual = 0;
	
	Bloques* bloques;
	Punteros* punteros;
	INodos* inodos;
	
	
	
	int CANT_INODOS = 500;
	
	bool CheckPermisoLeer(int node);
	bool CheckPermisoBorrar(int node);
	bool CheckPermisoEditar(int node);
	
	void ListarHijos(int node, int nivel);
	
	int BuscarEnNodo(string name, int nodo);
	int StringToNode(string comando);
	
public:
	
	FSystem();
	
	void EscribirPrompt();
	void ListarHijos();
	
	void SubirNivel();
	void CambiarDirectorioActual(string dir);
	
	void CrearArchivo(string name, int nodoPadre, int perm, byte *data, int sizeOfData);
	void CrearDirectorio(string name, int nodoPadre, int perm);
	
	void Borrar(int node);
	
	void MostrarArbol();
	
	void CambiarGrupo(Grupo nuevo);
	
	void CrearArchivo(string filePath, unsigned char* data, int size);
	void CrearDirectorio(string filePath);
	void Mover(string from, string to);
	void Eliminar(string path);
	
	~FSystem();
	
};

#endif

