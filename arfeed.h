/*
 * @Projekt:    Ctecka novinek ve formatu Atom s podporou SSL/TLS 
 * @Autor:      Milan Gardas <xgarda04@stud.fit.vutbr.cz>
 * @file:       arfeed.h
 * @date:       22.11.2014
 */


#include <iostream>
#include <cstring>
#include <fstream>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#define NAZEV 1
#define AUTOR 2
#define AKTUALIZACE 3
#define NAZEVNOVINKY 4
#define HTTP 80
#define HTTPS 443

using namespace std;

// Struktura pro uložení parametrů příkazové řádky
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

// Deklarace funkčních prototypů
void printHelp();
int zpracujParametry(int argc, char* argv[], Param* parametr);
int pocetRadku(string cesta);
int zpracovaniSouboru(string cesta, string * radky);
string adresa(string* zdroj, string* pozadav, string* adre, int* portC);
int port(string* zdroj, int* posun);
string portS(int index, string str, int portI, string* adre, int* portC);
void obsahElementu(xmlDocPtr doc, xmlNodePtr cur, xmlChar* text, int param);
void odkaz(xmlNodePtr cur);
static void zpracujXML(char *docname, int xmlVelikost, Param* parametr);
int connectHTTP(char* prip, string pozadav, string adres, Param* parametr);
int connectHTTPS(char* prip, string pozadav, string adres, Param* parametr);
int navratovyKod(string stranka, Param* parametr);
string pozadavek(string pozadav, string adres);
int zpracujStranku(string stranka, Param* parametr);