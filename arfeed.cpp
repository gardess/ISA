#include <iostream>
#include <string.h>
#include <cstring>
#include <cctype>
#include <fstream>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#define HTTP 80
#define HTTPS 443
#define NAZEV 1
#define AUTOR 2
#define AKTUALIZACE 3
#define NAZEVNOVINKY 4
#define ATOMAUTOR 5

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
int pocetRadku (string cesta);
int zpracovaniSouboru(string cesta, string * radky);
string adresa(string* zdroj, string* pozadav, string* adre, int* portC);
int port(string* zdroj, int* posun);
string portS(int index, string str, int portI, string* adre, int* portC);
void parseStoryMy (xmlDocPtr doc, xmlNodePtr cur, xmlChar* text, int param);
void getReference (xmlNodePtr cur);
static void parseDoc(char *docname, int xmlVelikost, Param* parametr);
int connectHTTP(char* prip, string pozadav, string adres, Param* parametr);
int connectHTTPS(char* prip, string pozadav, string adres, Param* parametr);


int main(int argc, char* argv[])
{
	// Zpracovani vstupnich parametru
	Param* parametr = new Param;
	int par = parseParameters(argc, argv, parametr);
	if (par == -1)
	{
		return -1;
	}

	// Zpracovani zdroju
	int ret = 0;
	string *novy;
	if (parametr->fParam == 1) // je zadan soubor se zdroji
	{
		int rad = pocetRadku(parametr->fParamStr);
		if (rad == -3)
		{
			return -3;
		}
		else if (rad == 0)
		{
			cerr << "Zadaný soubor neobsahuje zadny validni radek nebo je prazdny." << endl;
			return -4;
		}
		novy = new string[rad];
		ret = zpracovaniSouboru(parametr->fParamStr, novy);
		if (ret == -3)
		{
			return -3;
		}
	}
	else // Je zadan pouze jeden zdroj
	{
		novy = new string[1];
		ret = 1;
		novy[0].append(parametr->adresaParamStr);
	}

	// inicializace openSSL
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	SSL_library_init();

    // Cyklus, ktery ridi cely program
	for (int a = 0; a < ret; a++)
	{
		string pozadav;
		string adres;
		int portC;
		string adr = adresa(&novy[a], &pozadav, &adres, &portC);
		char *prip = new char[adr.length()+1];
		strcpy(prip,adr.c_str());
		//cout<< "PortC: " << portC << endl;
		// Pripojeni
		if (portC == HTTP)
		{
			connectHTTP(prip, pozadav, adres, parametr);
		}
		else if (portC == HTTPS)
		{
			connectHTTPS(prip, pozadav, adres, parametr);
		}
		delete[] prip;

		if (a+1 == ret)
		{
			
		}
		else
		{
			cout << "" << endl;
		}
	}
	delete[] novy;
	delete parametr;

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
	  "Program provádí výpis novinek ze stranek dle zadaných parametru.\n"
	  "Použití: arfeed --help\t\tVypise napovedu\n"
	  "         arfeed URL -a -T -u\tVypise vsechny prispevky spolecne s autory, aktualizaci a odkazem ze zadane stranky\n"
	  "         arfeed -f example -l\tVypise nazev nejnovejsiho prispevku na vsech strankach ktere jsou v example\n"
	  "         arfeed URL -c example\tPokud je platny certifikat example pak vypise vsechny prispevky ze zadane URL\n\n"
	  "Popis parametrů:\n"
	  "URL\t\turl se zdrojem\n"
	  "-f example\tsoubor obsahujici adresy se zdroji\n"
	  "-c example\tsoubor s certifikáty\n"
	  "-C example\tsložka obsahující soubory s certifikáty\n"
	  "-a\t\tzajisti ze u kazde novinky bude vypsan i jeji autor\n"
	  "-T\t\tzajisti ze u kazde novinky bude vypsan i cas zmeny\n"
	  "-u\t\tzajisti ze u kazde novinky bude vypsan i odkaz na ni\n"
	  "-l\t\tzajisti ze se vypisi pouze nejnovejsi prispevek\n";
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
			delete parametr;
			return -1;
		}
		else if ((strcmp("-h", argv[i]) == 0) || (strcmp("--help", argv[i]) == 0))
		{
			printHelp();
			delete parametr;
			return -1;
		}
		else if (('h' == argv[i][0])&&('t' == argv[i][1])&&('t' == argv[i][2])&&('p' == argv[i][3]))
		{
			parametr->adresaParam++;
			parametr->adresaParamStr = argv[i];
		}
		else
		{
			cerr << "Zadan neznamy parametr" << endl;
			delete parametr;
			return -1;
		}
	}
	if (((parametr->adresaParam > 0) && (parametr->fParam > 0)) || ((parametr->adresaParam == 0) && (parametr->fParam == 0)) || 
	(parametr->lParam > 1) || (parametr->TParam > 1) || (parametr->aParam > 1) || (parametr->uParam > 1) || (parametr->adresaParam > 1) || 
	(parametr->fParam > 1) || (parametr->cParam > 1) || (parametr->CParam > 1))
	{
		cerr << "Zadana nepovolena kombinace parametru" << endl;
		delete parametr;
		return -1;
	}
	if (parametr->CParam == 0)
	{
		parametr->CParam++;
		parametr->CParamStr.append("/etc/ssl/certs");
	}
	return 0;
}


/**
 * Funkce pro zjištění počtu korektních řádků
 * @param   string	cesta k souboru
 * @return  int
 */
int pocetRadku (string cesta)
{
	string line;
	int radek = 0;
	ifstream myfile (cesta);
	if (myfile.is_open())
	{
		while (getline (myfile,line))
    	{
     	// cout << line << '\n';
			for(unsigned int i = 0; i < line.length(); i++)
			{
				if(line[i] == '#')
				{
					line.erase(line.begin()+i, line.end());
				}
			} 
			if (line[0] == 'h')
			{
				radek++;
			}
		//cout << line << endl;
    	}
    	//cout << "Pocet neprazdnych radku: " << radek << endl;
		myfile.close();
		return radek;
	} 
	else
	{
		cout << "Unable to open file" << endl;
		return -3;
	}
}


/**
 * Funkce pro získání korektních řádků ze souboru
 * @param   string	cesta k souboru
 * @param   string*	pole řetězců pro uložení obsahu souboru
 * @return  int
 */
int zpracovaniSouboru(string cesta, string * radky)
{
	string line;
	ifstream soubor (cesta);
	if (soubor.is_open())
	{
    	int pocet = 0;
    	while ( getline (soubor,line) )
    	{
			//cout << "TEST" << endl;
			for(unsigned int i = 0; i < line.length(); i++)
			{
				if(line[i] == '#')
				{
					line.erase(line.begin()+i, line.end());
				}
			}
				if (line[0] == 'h')
				{
					radky[pocet] = line;
					pocet++;
				}
    		
		}
    	soubor.close();
    	return pocet;
 	}
	else
	{
		cout << "Unable to open file" << endl;
		return -3; 
	}
}


/**
 * Funkce, která spojí adresu s portem
 * @param	Param	struktura pro uložený zadaných argumentů
 * @return  string
 */
string adresa(string* zdroj, string* pozadav, string* adre, int* portC)
{
	int portI = 0;
	int posun = 0;
	string str, str2;
	string lomitko = "/";
	portI = port(zdroj, &posun);
	str.assign(*zdroj,posun,zdroj->length());
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
	//pozadav = new string[pozadavek.length()];
	pozadav->append(pozadavek);
	string adresaStr;
	adresaStr = portS(index, str2, portI, adre, portC);
	/*cout << "-------------------------------------------------------------------" << endl;
	cout << "AdresaStr: " << adresaStr << endl;
	cout << "Pozadavek: " << pozadavek << endl;
	cout << "-------------------------------------------------------------------" << endl;*/
	return adresaStr;
}


/**
 * Funkce, která zjistí číslo portu z prefixu
 * @param	Param*	ukazatel na strukturu uložených argumentů
 * @param	int*	ukazatel na int pro uložení velikosti prefixu
 * @return  int 	cislo portu
 */
int port(string* zdroj, int* posun)
{
	string prefix1 = "http://";
	string prefix2 = "https://";
	int portI = 0;
	size_t found = zdroj->find(prefix1);
	if (found!=string::npos)
	{
		portI = HTTP;
		//cout << "http nalezeno na " << found << " port: " << portI << endl;
		*posun = 7;
	}
	found = zdroj->find(prefix2);
	if (found!=string::npos)
	{
		portI = HTTPS;
		//cout << "https nalezeno na " << found << " port: " << portI << endl;
		*posun = 8;
	}
	return portI;
}


/**
 * Funkce, která zjistí číslo portu z adresy a spoji adresu s cislem portem
 * @param	int		hodnota obsahující index lomítka v adrese
 * @param 	string 	zadaná adresa
 * @param	int		port zjistěný z prefixu
 * @return  string 	
 */
string portS(int index, string str, int portI, string* adre, int* portC)
{
	int delka = 0;
	bool dvojtecka = false;
	// zjisteni zda adresa obsahuje cislo portu
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
	// priprava portu pro pridani do adresy
	if (dvojtecka == true)
	{
		portStr.assign(str,(index-delka),index-1);
		str.erase(index-delka-1,delka+1);
	}
	else
	{
		portStr = to_string(portI);
	}
	string::size_type vel;
	*portC = stoi(portStr, &vel);
	/*cout << "portStr: " << portStr << endl;
	cout<< "portI: "<<portI <<endl;*/
	string dvojteckaStr = ":";
	//cout << "Str: " << str << endl;
	adre->append(str);
	// spojeni adresy s cislem portu
	str.append(dvojteckaStr);
	str.append(portStr);
	return str;
}


/**
 * Funkce, která vypíše hodnotu xml elementu
 * @param	xmlDocPtr	xml strom
 * @param	xmlNodePtr	aktuální uzel xml stromu
 * @param	xmlChar*	element který vyhledáváme
 * @param	int			hodnota sloužící k vypsání hodnoty elementu se správným řetězcem
 */
void parseStoryMy (xmlDocPtr doc, xmlNodePtr cur, xmlChar* text, int param)
{
	xmlChar *key;
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
	    if ((!xmlStrcmp(cur->name, text))) {
		    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			switch(param)
		    {
		    case NAZEV :
		     	cout << "*** " << key << " ***" << endl;
		    	break;
		  	case AUTOR :
		  		cout << "Autor: " << key << endl;
		    	break;
		  	case AKTUALIZACE :
		    	cout << "Aktualizace: " << key << endl;
		    	break;
		   	case NAZEVNOVINKY :
		    	cout << key << endl;
		    	break;
		    case ATOMAUTOR :
		    	break;
		   	default :
		    	cerr << "Spatny parametr funkce pro parsovani XML." << endl;
		   	}
		    //printf("%s: %s\n",text, key);
		    xmlFree(key);
 	    }  
		cur = cur->next;
	}
    return;
}


/**
 * Funkce, která vypíše hodnotu parametru xml elementu
 * @param	xmlNodePtr	aktuální uzel xml stromu
 */
void getReference (xmlNodePtr cur)
{
	xmlChar *uri;
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
	    if ((!xmlStrcmp(cur->name, (const xmlChar *)"link"))) {
		    uri = xmlGetProp(cur, (xmlChar *)"href");
		    cout << "URL: " << uri << endl;
		    xmlFree(uri);
	    }
	    cur = cur->next;
	}
	return;
}


/**
 * Funkce, která zpracuje xml získané ze zadané stránky
 * @param	char*	uložení obsahu stránky
 * @param	int		velikost obsahu stránky
 * @param	Param*	struktura se zadanými parametry
 */
static void parseDoc(char *docname, int xmlVelikost, Param* parametr)
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlNodePtr temp;

	//doc = xmlParseFile(docname); // pravděpodobně bude změna na xmlParseMemory
	doc = xmlParseMemory(docname, xmlVelikost);
	if (doc == NULL )
	{
		//fprintf(stderr,"Document not parsed successfully. \n");
		cerr << "Document not parsed successfully." << endl;
		return;
	}
	
	cur = xmlDocGetRootElement(doc);
	
	if (cur == NULL)
	{
		//fprintf(stderr,"empty document\n");
		cerr << "empty document" << endl;
		xmlFreeDoc(doc);
		return;
	}
	
	if (xmlStrcmp(cur->name, (const xmlChar *) "feed"))
	{
		//fprintf(stderr,"document of the wrong type, root node != feed");
		cerr << "document of the wrong type, root node != feed" << endl;
		xmlFreeDoc(doc);
		return;
	}
	////////////////////////
	// uroven zanoreni 1
	if ((!xmlStrcmp(cur->name, (const xmlChar *)"feed")))
	{
		parseStoryMy (doc, cur, (xmlChar *)"title", NAZEV);
	}
	//vyhledavani elementu "author" v rodicovskem uzlu
	//temp = cur;
	cur = cur->xmlChildrenNode;
	//printf("jmeno cur elementu: %s\n", cur->name);
	int iterace = 0;
	int nalezl = 0;
	while(cur != NULL)
	{
		//iterace++;
		// vyhledavani elementu "author v kořenovém elementu"
		if (((!xmlStrcmp(cur->name, (const xmlChar *)"author"))) && (parametr->aParam == 1))
		{
			// volani funkce pro ulozeni jmeno autor celeho atom dokumentu
			parseStoryMy (doc, cur, (xmlChar *)"name", ATOMAUTOR);
		}
		// vyhledani elementu "entry" v kořenovém elementu
		else if ((!xmlStrcmp(cur->name, (const xmlChar *)"entry")))
		{
			iterace++;
			// volani funkce pro vypis nazvu clanku
			parseStoryMy (doc, cur, (xmlChar *)"title", NAZEVNOVINKY);
			// volani funkce pro vypis datumu aktualizace, pouze pokud je zadan parametr -T
			if (parametr->TParam == 1)
			{
				parseStoryMy (doc, cur, (xmlChar *)"updated", AKTUALIZACE); // Parametr -T
			}
			if (parametr->uParam == 1)
			{
			// volani funkce pro ziskani parametru z xml tagu a jeho nasledne vypsani
			getReference (cur);
			}
			if (parametr->aParam == 1)
			{
				temp = cur->xmlChildrenNode;
				while(temp != NULL)
				{
					// vyhledani elementu author v elementu entry
					if ((!xmlStrcmp(temp->name, (const xmlChar *)"author")))
					{
						// volani funkce pro vypis autora, pouze pokud je zadan parametr -a
						parseStoryMy(doc, temp, (xmlChar *)"name", AUTOR); // Parametr -a
						nalezl = 1;
					}
					temp = temp->next; 
				}
				if (nalezl == 0)
				{
				cout << "Autor: " << "No author" << endl;
				nalezl = 1;
				}
			}
			if (cur->next == NULL) // 
			{

			}
			else if (parametr->lParam == 1)
			{

			}
			else if ((parametr->aParam == 1) || (parametr->uParam == 1) || (parametr->TParam == 1))
			{
				cout << "" << endl;
			}
		}
		// pro zobrazeni pouze prvni novinky ("feedu")
		if ((parametr->lParam == 1) && (iterace == 1))
		{
			break;
		}
	nalezl = 0;	 
	cur = cur->next;
	}
	xmlFreeDoc(doc);
	return;
}


int connectHTTP(char* prip, string pozadav, string adres, Param* parametr)
{
	BIO * bio;
	bio = BIO_new_connect(prip);
	if (bio == NULL)
	{
		cerr << "CHYBA" << endl;
		return -1;
	}
	if (BIO_do_connect(bio) <= 0)
	{
		cerr << "Chyba spojeni" << endl;
		return -2;
	}

	// skladani GET requestu 
	string s1, s2, s3, s4, s5, s6;
	s1 = "GET ";
	s2 = pozadav;//"/hardware/headlines.atom";//"/headlines.atom";
	s3 = " HTTP/1.0\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\r\nHost: ";
	s4 = adres;//"www.theregister.co.uk";//"www.theregister.co.uk";
	s5 = "\r\nConnection: close\r\n\r\n";
	s6 = s1;
	s6.append(s2);
	s6.append(s3);
	s6.append(s4);
	s6.append(s5);
	char *request = new char[s6.length()+1];
	strcpy(request,s6.c_str());
    char r[1024];
	/* Send the request */
    BIO_write(bio, request, strlen(request));
    /* Read in the response */
    int p;
    string stranka = "";
    for(;;)
    {
        p = BIO_read(bio, r, 1023);
        if(p <= 0) break;
        r[p] = 0;
       	stranka.append(r);
	}
	BIO_free_all(bio);
	delete[] request;
	
	// Nalezení začátku xml z Get požadavku
	unsigned int delkaHlavicky = 0;
	for(unsigned int i = 0; i < stranka.length(); i++)
	{
		if ((stranka[i] == '<') && (stranka[i+1] == 'f') && (stranka[i+2] == 'e') && 
        (stranka[i+3] == 'e') && (stranka[i+4] == 'd') && (stranka[i+5] == ' ') && 
        (stranka[i+6] == 'x') && (stranka[i+7] == 'm') && (stranka[i+8] == 'l') && 
        (stranka[i+9] == 'n') && (stranka[i+10] == 's') && (stranka[i+11] == '='))
		{
			delkaHlavicky = i;
			break;
		}
	}

	//cout << stranka << endl;
	// Zjistena zda je 200 nebo 301

	stranka.erase(stranka.begin(), stranka.begin()+delkaHlavicky);
	int xmlVelikost = stranka.length() + 1;
	char *xmldoc = new char[xmlVelikost];
	strcpy(xmldoc,stranka.c_str());
	// Zpracování xml
	parseDoc (xmldoc, xmlVelikost, parametr);
	delete[] xmldoc;
	return 0;
}


int connectHTTPS(char* prip, string pozadav, string adres, Param* parametr)
{
    BIO * bio;
    SSL * ssl;
    SSL_CTX * ctx;

    int p;

    
    char r[1024];



    // Set up the SSL context 

    ctx = SSL_CTX_new(SSLv23_client_method());

    // Load the trust store 
    if (parametr->cParam == 1)
    {
    	char *cFile = new char[parametr->cParamStr.length()+1];
		strcpy(cFile,parametr->cParamStr.c_str());
        if(! SSL_CTX_load_verify_locations(ctx, cFile, NULL))
        {
            fprintf(stderr, "Error loading trust store\n");
            ERR_print_errors_fp(stderr);
            SSL_CTX_free(ctx);
            delete[] cFile;
            return -5;
        }
        delete[] cFile;
    }

    char *CDirectory = new char[parametr->CParamStr.length()+1];
	strcpy(CDirectory,parametr->CParamStr.c_str());
    if(! SSL_CTX_load_verify_locations(ctx, NULL, CDirectory))
    {
        fprintf(stderr, "Error loading trust store\n");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        delete[] CDirectory;
        return -5;
    }
    delete[] CDirectory;

    // Setup the connection 

    bio = BIO_new_ssl_connect(ctx);

    // Set the SSL_MODE_AUTO_RETRY flag 

    BIO_get_ssl(bio, & ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    // Create and setup the connection 

    BIO_set_conn_hostname(bio, prip);

    if(BIO_do_connect(bio) <= 0)
    {
        fprintf(stderr, "Error attempting to connect\n");
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        return -6;
    }

    //Check the certificate 

    if(SSL_get_verify_result(ssl) != X509_V_OK)
    {
    	string zdroj(prip); 
    	zdroj.erase(zdroj.end()-4, zdroj.end());
    	cerr << "Chyba: nepodarilo se overit platnost certifikatu serveru " << zdroj << endl;
        //fprintf(stderr, "Certificate verification error: %ld\n", SSL_get_verify_result(ssl));
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        return -6;
    }

    string s1, s2, s3, s4, s5, s6;
    s1 = "GET ";
    s2 = pozadav;//"/hardware/headlines.atom";//"/headlines.atom";
    s3 = " HTTP/1.0\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\r\nHost: ";
    s4 = adres;//"www.theregister.co.uk";//"www.theregister.co.uk";
    s5 = "\r\nConnection: close\r\n\r\n";
    s6 = s1;
    s6.append(s2);
    s6.append(s3);
    s6.append(s4);
    s6.append(s5);
    char *request = new char[s6.length()+1];
    strcpy(request,s6.c_str());

    // Send the request 

    BIO_write(bio, request, strlen(request));

    //Read in the response 
    string stranka = "";
    for(;;)
    {
        p = BIO_read(bio, r, 1023);
        if(p <= 0) break;
        r[p] = 0;
        //printf("%s", r);
        stranka.append(r);
    }
    /*cout << "---------------------------------------------------" << endl;
    cout << stranka << endl;
    cout << "---------------------------------------------------" << endl;*/
    // Close the connection and free the context 

    BIO_free_all(bio);
    SSL_CTX_free(ctx);
    delete[] request;

    // Nalezení začátku xml z Get požadavku
    unsigned int delkaHlavicky = 0;
    for(unsigned int i = 0; i < stranka.length(); i++)
    {
        if ((stranka[i] == '<') && (stranka[i+1] == 'f') && (stranka[i+2] == 'e') && 
        (stranka[i+3] == 'e') && (stranka[i+4] == 'd') && (stranka[i+5] == ' ') && 
        (stranka[i+6] == 'x') && (stranka[i+7] == 'm') && (stranka[i+8] == 'l') && 
        (stranka[i+9] == 'n') && (stranka[i+10] == 's') && (stranka[i+11] == '='))
        {
            delkaHlavicky = i;
            break;
        }
    }
    stranka.erase(stranka.begin(), stranka.begin()+delkaHlavicky);
    int xmlVelikost = stranka.length() + 1;
    char *xmldoc = new char[xmlVelikost];
    strcpy(xmldoc,stranka.c_str());
    // Zpracování xml
    parseDoc (xmldoc, xmlVelikost, parametr);
    delete[] xmldoc;
    return 0;
}