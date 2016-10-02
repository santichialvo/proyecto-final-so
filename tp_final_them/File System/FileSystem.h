#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "Bloques.h"
#include "Punteros.h"
#include "INodos.h"

using namespace std;


class FileSystem {
private:
	
	char user[10];
	int nodoActual;
	
	Bloques* bloques;
	Punteros* punteros;
	INodos* inodos;
	
	void Cd();
	void CdUp();
	void CdDown(string name);
	void Ls(string name);
	void ShowFile(string name);
	
	void CrearDirectorio(string name);
	void CrearArchivo(string name);
	
	int PathToNodo(string path, int padre);
	string NodoToPath(int node);
	
	void LoadFile(string name);
	void SaveToFile(string name);
	int FindNodo(string name, int padre);
	
	void EscribirPrompt();
	
public:
	
	FileSystem();
	
	void Run();
	
	~FileSystem();
	
};

#endif

