#include <iostream>
#include <string.h>
#include <cstring>
#include <cctype>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define HTTP 80
#define HTTPS 443


using namespace std;

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


int main(int argc, char* argv[])
{
	Param* parametr = new Param;
	int par = parseParameters(argc, argv, parametr);
	if (par == -1)
	{
		//cerr << "Spatne zadane parametry" << endl;
		delete parametr;
		return -1;
	}
	else if (par == -2)
	{
		delete parametr;
		return 0;
	}
		
	string adr = adresa(parametr);

	char *prip = new char[adr.length()+1];
	strcpy(prip,adr.c_str());
	cout << "test: " << prip << endl;
	//--------------------------------------------------------------------------------------------------
	// Pripojeni

	//funkce pro openSSL
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	SSL_library_init();

	BIO * bio;
	bio = BIO_new_connect(prip);
	if (bio == NULL)
	{
		cerr << "CHYBA" << endl;
		return 1;
	}

	if (BIO_do_connect(bio) <= 0)
	{
		cerr << "Chyba spojeni" << endl;
		return 2;
	}
	cout << "Spojeni ok" << endl;
	//----------------------------------------------------------------
	//char const * request = "GET / HTTP/1.1\x0D\x0AHost: www.verisign.com\x0D\x0A\x43onnection: Close\x0D\x0A\x0D\x0A";
	//char const * request = "GET / HTTP/1.1\x0D\x0AHost:http://tools.ietf.org/agenda/atom\x0D\x0A\x43onnection: Close\x0D\x0A\x0D\x0A";
	//char const * request = "GET / HTTP/1.1\x0D\x0AHost: www.w3.org/pub/WWW/TheProject";
	//char const * request = "GET / HTTP/1.1\r\nHost: www.google.com\r\nConnection: close\r\n\r\n"; // relativne funkcni
	//char const * request = "GET /agenda/atom HTTP/1.1\r\nHost: tools.ietf.org\r\nConnection: close\r\n\r\n"; // 403 forbidden
	//char const * request = "GET /headlines.atom HTTP/1.1\r\nHost: www.theregister.co.uk\r\nConnection: close\r\n\r\n"; // SPRAVNY GET REQUEST
	//////////////////
	// skladani GET requestu
	string s1, s2, s3, s4, s5, s6;
	s1 = "GET ";
	s2 = "/headlines.atom";
	s3 = " HTTP/1.1\r\nHost: ";
	s4 = "www.theregister.co.uk";
	s5 = "\r\nConnection: close\r\n\r\n";
	s6 = s1;
	s6.append(s2);
	s6.append(s3);
	s6.append(s4);
	s6.append(s5);
	char *request = new char[s6.length()+1];
	strcpy(request,s6.c_str());
	//////////////////
    char r[1024];
	/* Send the request */

    BIO_write(bio, request, strlen(request));

    /* Read in the response */
    int p;
    for(;;)
    {
        p = BIO_read(bio, r, 1023);
        if(p <= 0) break;
        r[p] = 0;
        printf("%s", r);
    }
	//cout << "Prijato: " << buf << endl; (http://www.theregister.co.uk/headlines.atom)
	//----------------------------------------------------------------
	BIO_free_all(bio);
	return 0;	
}


/**
 * Funkce pro výpis nápovědy
 */
void printHelp()
{
	const char *HELPMSG =
	  "Ctecka novinek ve formatu Atom s podporou SSL/TLS\n"
	  "Autor: Milan Gardas (xgarda04@stud.fit.vutbr.cz)\n"
	  "Program provádí...\n"
	  "Použití: proj1 --help\n"
	  "         proj1 -t  znamená ...\n"
	  "         proj1 -h  znamená ... \n"
	  "         ..."
	  "Popis parametrů:\n"
	// sem doplňte svůj popis
	  "...\n";
	  cout << HELPMSG;
}


/**
 * Funkce pro kontrolu zadaných parametrů
 * @param   int		počet parametrů
 * @param   char	pole zadaných argumentů
 * @param	Param	struktura pro uložený zadaných argumentů
 * @return  int
 */
int parseParameters(int argc, char* argv[], Param* parametr)
{
	for(int i = 1; i < argc; i++)
	{
		if (strcmp("-l", argv[i]) == 0)
		{
			parametr->lParam++; 
		}
		else if (strcmp("-T", argv[i]) == 0)
		{
			parametr->TParam++; 
		}
		else if (strcmp("-a", argv[i]) == 0)
		{
			parametr->aParam++; 
		}
		else if (strcmp("-u", argv[i]) == 0)
		{
			parametr->uParam++; 
		}
		else if (strcmp("-f", argv[i]) == 0)
		{
			parametr->fParam++; 
			parametr->fParamStr = argv[i+1];
			i++;
		}
		else if (strcmp("-c", argv[i]) == 0)
		{
			parametr->cParam++;
			parametr->cParamStr = argv[i+1];
			i++;
		}
		else if (strcmp("-C", argv[i]) == 0)
		{
			parametr->CParam++; 
			parametr->CParamStr = argv[i+1];
			i++;
		}
		else if (((strcmp("-h", argv[i]) == 0) || (strcmp("--help", argv[i]) == 0)) && (argc != 2))
		{
			cerr << "Parametr napovedy nebyl zadan samostatne" << endl;
			return -1;
		}
		else if ((strcmp("-h", argv[i]) == 0) || (strcmp("--help", argv[i]) == 0))
		{
			printHelp();
			return -2;
		}
		else if (('h' == argv[i][0])&&('t' == argv[i][1])&&('t' == argv[i][2])&&('p' == argv[i][3]))
		{
			parametr->adresaParam++;
			parametr->adresaParamStr = argv[i];
		}
		else
		{
			cerr << "Zadan neznamy parametr" << endl;
			return -1;
		}
	}
	if (((parametr->adresaParam > 0) && (parametr->fParam > 0)) || ((parametr->adresaParam == 0) && (parametr->fParam == 0)) || 
	(parametr->lParam > 1) || (parametr->TParam > 1) || (parametr->aParam > 1) || (parametr->uParam > 1) || (parametr->adresaParam > 1) || 
	(parametr->fParam > 1) || (parametr->cParam > 1) || (parametr->CParam > 1))
	{
		cerr << "Zadana nepovolena kombinace parametru" << endl;
		return -1;
	}
	/*
	cout << "lParam: " << parametr->lParam << endl;
	cout << "TParam: " << parametr->TParam << endl;
	cout << "aParam: " << parametr->aParam << endl;
	cout << "uParam: " << parametr->uParam << endl;
	cout << "adresaParam: " << parametr->adresaParam << " " << "adresaParamStr: " << parametr->adresaParamStr << endl;
	cout << "fParam: " << parametr->fParam << " " << "fParamStr: " << parametr->fParamStr << endl;
	cout << "cParam: " << parametr->cParam << " " << "cParamStr: " << parametr->cParamStr << endl;
	cout << "CParam: " << parametr->CParam << " " << "CParamStr: " << parametr->CParamStr << endl;
	*/
	return 0;
}


/**
 * Funkce, která spojí adresu s portem
 * @param	Param	struktura pro uložený zadaných argumentů
 * @return  string
 */
string adresa(Param* parametr)
{
	int portI = 0;
	int posun = 0;
	string str, str2;
	string lomitko = "/";
	portI = port(parametr, &posun);
	str.assign(parametr->adresaParamStr,posun,parametr->adresaParamStr.length());
	int index;
	str.append(lomitko); // pridani lomitka na konec adresy => zajisti ze lomitko bude vzdy nalezeno 
	for(unsigned int i = 0; i < str.length(); i++)
	{
		if (str[i] == '/')
		{
			index = i;
			break;
		}
	}

	string pozadavek;
	str2.assign(str,0,index); // <host>:<port>
	pozadavek.assign(str,index,str.length()); // <path>?<searchpart>
	pozadavek.pop_back(); // odebrani lomitka na konci adresy
	string adresaStr;
	adresaStr = portS(index, str2, portI);

	return adresaStr;
}


/**
 * Funkce, která zjistí číslo portu z prefixu
 * @param	Param*	ukazatel na strukturu uložených argumentů
 * @param	int*	ukazatel na int pro uložení velikosti prefixu
 * @return  int
 */
int port(Param* parametr, int* posun)
{
	string prefix1 = "http://";
	string prefix2 = "https://";
	int portI = 0;
	size_t found = parametr->adresaParamStr.find(prefix1);
	if (found!=string::npos)
	{
		portI = HTTP;
		cout << "http nalezeno na " << found << " port: " << portI << endl;
		*posun = 7;
	}
	found = parametr->adresaParamStr.find(prefix2);
	if (found!=string::npos)
	{
		portI = HTTPS;
		cout << "https nalezeno na " << found << " port: " << portI << endl;
		*posun = 8;
	}
	return portI;
}


/**
 * Funkce, která zjistí číslo portu z adresy
 * @param	int		hodnota obsahující index lomítka v adrese
 * @param 	string 	zadaná adresa
 * @param	int		port zjistěný z prefixu
 * @return  string
 */
string portS(int index, string str, int portI)
{
	int delka = 0;
	bool dvojtecka = false;
	for(int i = index-1; i > 0; i--)
	{
		if (isdigit(str[i]) == true)
		{
			delka++;
			continue;
		}
		else if (str[i] == ':')
		{
			dvojtecka = true;
			break;
		}
		else
		{
			break;
		}
	}

	string portStr;
	if (dvojtecka == true)
	{
		portStr.assign(str,(index-delka),index-1);
		str.erase(index-delka-1,delka+1);
	}
	else
	{
		portStr = to_string(portI);
	}
	string dvojteckaStr = ":";
	str.append(dvojteckaStr);
	str.append(portStr);
	return str;
}

// Parsování XML
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

void
parseStory (xmlDocPtr doc, xmlNodePtr cur) {

	xmlChar *key;
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
	    if ((!xmlStrcmp(cur->name, (const xmlChar *)"name"))) {
		    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    printf("name: %s\n", key);
		    xmlFree(key);
 	    }
	cur = cur->next;
	}
    return;
}

void
parseStoryMy (xmlDocPtr doc, xmlNodePtr cur, xmlChar* text) {

	xmlChar *key;
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
	    if ((!xmlStrcmp(cur->name, text))) {
		    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    printf("%s: %s\n",text, key);
		    xmlFree(key);
 	    }
	cur = cur->next;
	}
    return;
}

void getReference (xmlDocPtr doc, xmlNodePtr cur) {

	xmlChar *uri;
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
	    if ((!xmlStrcmp(cur->name, (const xmlChar *)"link"))) {
		    uri = xmlGetProp(cur, "href");
		    printf("href: %s\n", uri);
		    xmlFree(uri);
	    }
	    cur = cur->next;
	}
	return;
}

static void
parseDoc(char *docname) {

	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlNodePtr temp;
	xmlNodePtr pom;

	doc = xmlParseFile(docname);
	
	if (doc == NULL ) {
		fprintf(stderr,"Document not parsed successfully. \n");
		return;
	}
	
	cur = xmlDocGetRootElement(doc);
	
	if (cur == NULL) {
		fprintf(stderr,"empty document\n");
		xmlFreeDoc(doc);
		return;
	}
	
	if (xmlStrcmp(cur->name, (const xmlChar *) "feed")) {
		fprintf(stderr,"document of the wrong type, root node != feed");
		xmlFreeDoc(doc);
		return;
	}
	////////////////////////
	// uroven zanoreni 1
	if ((!xmlStrcmp(cur->name, (const xmlChar *)"feed"))){
			parseStoryMy (doc, cur, (xmlChar *)"title");
	}
	//vyhledavani elementu "author" v rodicovskem uzlu
	//temp = cur;
	cur = cur->xmlChildrenNode;
	//printf("jmeno cur elementu: %s\n", cur->name);
	while (cur != NULL) {
		// vyhledavani elementu "author v kořenovém elementu"
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"author"))){
			parseStoryMy (doc, cur, (xmlChar *)"name");
		}
		// vyhledani elementu "entry" v kořenovém elemnetu
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"entry"))){
			parseStoryMy (doc, cur, (xmlChar *)"title");
			parseStoryMy (doc, cur, (xmlChar *)"updated"); // Parametr -T
			//parseStoryMy(doc, cur, (xmlChar *)"link");
			/*pom = cur;
			/*temp = cur->xmlChildrenNode;
			/*xmlChar * uri;
			while (temp != NULL)
			{
				//printf("TEST\n");
				if ((!xmlStrcmp(temp->name, (const xmlChar *)"link")))
				{*/
				//	printf("TEST\n");
					//printf("LINK: %s\n", xmlNodeGetContent(temp->children));
					printf("//////////\n");
					//parseStoryMy(doc, temp, (xmlChar *)"href");
					getReference (doc, cur);
					printf("//////////\n");
			/*	}
				temp = temp->next;
			}*/

			temp = cur->xmlChildrenNode;
			//printf("jmeno temp elementu: %s\n", temp->name);
			while (temp != NULL)
			{
				if ((!xmlStrcmp(temp->name, (const xmlChar *)"author")))
				{
					//printf("KONTROLA\n");
					parseStoryMy(doc, temp, (xmlChar *)"name"); // Parametr -a
				}
				temp = temp->next; 
			}
			printf("\n");
		}
		 
	cur = cur->next;
	}
	/*cur = temp;
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *)"entry"))){
			parseStoryMy (doc, cur, (xmlChar *)"title");
			parseStoryMy (doc, cur, (xmlChar *)"updated");
		}
		 
	cur = cur->next;
	}*/
	////////////////////////
	xmlFreeDoc(doc);
	return;
}

int
main(int argc, char **argv) {

	char *docname;
		
	if (argc <= 1) {
		printf("Usage: %s docname\n", argv[0]);
		return(0);
	}

	docname = argv[1];
	parseDoc (docname);

	return (1);
}