#include <iostream>
#include <string.h>
#include <cstring>
#include <cctype>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define HTTP 80
#define HTTPS 443

// struktura pro uložení parametrů příkazové řádky
using Param = struct parametry {
	string adresaParamStr;
	string fParamStr;
	string cParamStr;
	string CParamStr;
	int adresaParam = 0;
	int fParam = 0;
	int cParam = 0;
	int CParam = 0;
	int lParam = 0;
	int TParam = 0;
	int aParam = 0;
	int uParam = 0;
};

// deklarace funkčních prototypů
void printHelp();
int parseParameters(int argc, char* argv[], Param* parametr);
string adresa(Param* parametr);
int port(Param* parametr, int* posun);
string portS(int index, string str, int portI);
