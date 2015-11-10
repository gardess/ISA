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

#include <iostream>
#include <cstring>
#include <cctype>

#define NAZEV 1
#define AUTOR 2
#define AKTUALIZACE 3
#define NAZEVNOVINKY 4
#define ATOMAUTOR 5

using namespace std;

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
		    	// ulozit do promenne
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
 * @param	xmlDocPtr	xml strom
 * @param	xmlNodePtr	aktuální uzel xml stromu
 */
void getReference (xmlDocPtr doc, xmlNodePtr cur)
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

static void parseDoc(char *docname, Param* parametr)
{
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlNodePtr temp;

	//doc = xmlParseFile(docname); // pravděpodobně bude změna na xmlParseMemory
	doc = xmlParseMemory(docname, 50000);
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
			getReference (doc, cur);
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
					}
					temp = temp->next; 
				}
			}
			if ((parametr->aParam == 1) || (parametr->uParam == 1) || (parametr->TParam == 1))
			{
				cout << "" << endl; // mezera mezi jednotlivymi novinkami bez parametru tu nema co delat !!
			}
		}
		if ((parametr->lParam == 1) && (iterace == 1))
		{
			cout << "BREAK" << endl;
			break;
		}	 
	cur = cur->next;
	}
	xmlFreeDoc(doc);
	return;
}

int
main() {

	char *docname = new char[50000];
	strcpy(docname, "<?xml version=\"1.0\" encoding=\"UTF-8\"?><feed xmlns=\"http://www.w3.org/2005/Atom\" xml:lang=\"en\"><id>tag:theregister.co.uk,2005:feed/theregister.co.uk/data_centre/</id><title>The Register - Data Centre</title><link rel=\"self\" type=\"application/atom+xml\" href=\"http://www.theregister.co.uk/data_centre/headlines.atom\"/><link rel=\"alternate\" type=\"text/html\" href=\"http://www.theregister.co.uk/data_centre/\"/><rights>Copyright © 2015, Situation Publishing</rights><author><name>Team Register</name><email>webmaster@theregister.co.uk</email><uri>http://www.theregister.co.uk/odds/about/contact/</uri></author><icon>http://www.theregister.co.uk/Design/graphics/icons/favicon.png</icon><subtitle>Biting the hand that feeds IT — sci/tech news and views for the world</subtitle><logo>http://www.theregister.co.uk/Design/graphics/Reg_default/The_Register_r.png</logo><updated>2015-11-05T14:00:59Z</updated><entry><id>tag:theregister.co.uk,2005:story/2015/11/05/aws_announces_korean_region/</id><updated>2015-11-05T03:57:06Z</updated><author><name>Simon Sharwood</name><uri>http://search.theregister.co.uk/?author=Simon%20Sharwood</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/05/aws_announces_korean_region/\"/><title type=\"html\">Seoul Man Jeff Bezos makes Korea-defining move</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;AWS promises new (South) Korean region for early 2016&lt;/h4&gt; &lt;p&gt;Amazon Web Services (AWS) has announced it will create a (South) Korean region in 2016.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/05/backup_software_that_cracks_web_servers_yup_its_a_thing/</id><updated>2015-11-05T02:57:04Z</updated><author><name>Simon Sharwood</name><uri>http://search.theregister.co.uk/?author=Simon%20Sharwood</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/05/backup_software_that_cracks_web_servers_yup_its_a_thing/\"/><title type=\"html\">Backup software that cracks web servers? Yup. It's a thing</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Crooked cookies can cook Commvault and allow arbitrary OS command execution&lt;/h4&gt; &lt;p&gt;Commvault's Edge Server offers users the chance to view and access their backups from mobile devices, a trick it enables in part by using a web server.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/04/micron_has_three_bits_between_its_pc_diskkilling_flash_teeth/</id><updated>2015-11-04T17:32:06Z</updated><author><name>Chris Mellor</name><uri>http://search.theregister.co.uk/?author=Chris%20Mellor</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/04/micron_has_three_bits_between_its_pc_diskkilling_flash_teeth/\"/><title type=\"html\">Micron has three bits between its PC disk-killing flash teeth</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Crucial SSD offers TLC for slow disk drives&lt;/h4&gt; &lt;p&gt;Micron is targeting consumers and SMBs with a disk-replacing three bit Crucial-brand SSD.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/04/speaking_in_tech_episode_184/</id><updated>2015-11-04T17:03:04Z</updated><author><name>Team Register</name><uri>http://search.theregister.co.uk/?author=Team%20Register</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/04/speaking_in_tech_episode_184/\"/><title type=\"html\">Speaking in Tech: Larry Ellison pipes up on EMC/Dell – it's all yours Michael</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Microsoft announces limited unlimited storage&lt;/h4&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/04/microsoft_and_red_hat_announce_cloud_partnership/</id><updated>2015-11-04T14:49:09Z</updated><author><name>Tim Anderson</name><uri>http://search.theregister.co.uk/?author=Tim%20Anderson</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/04/microsoft_and_red_hat_announce_cloud_partnership/\"/><title type=\"html\">Red Hat Enterprise Linux lands on Microsoft Azure cloud – no, we're not pulling your leg</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Distro to be reference operating system for .NET Core&lt;/h4&gt; &lt;p&gt;Microsoft has announced a &lt;a target=\"_blank\" href=\"http://news.microsoft.com/2015/11/04/microsoft-and-red-hat-to-deliver-new-standard-for-enterprise-cloud-experiences/\"&gt;partnership with Red Hat&lt;/a&gt; to support Red Hat Linux in the Azure cloud.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/04/nutanix_gets_its_entry_into_china/</id><updated>2015-11-04T14:00:10Z</updated><author><name>Chris Mellor</name><uri>http://search.theregister.co.uk/?author=Chris%20Mellor</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/04/nutanix_gets_its_entry_into_china/\"/><title type=\"html\">Nutanix summons China's Lenovo dragon to magic up hardware</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Deal gives hyperconverged upstart PRC creds&lt;/h4&gt; &lt;p&gt;Nutanix has scored Lenovo as a hardware manufacturer for its hyperconverged system software, effectively wedging its foot in China's front door.…&lt;/p&gt; &lt;p&gt;&lt;!--#include virtual='/data_centre/_whitepaper_textlinks_top.html' --&gt;&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/04/companies_need_answer_safe_harbour_minister/</id><updated>2015-11-04T12:14:06Z</updated><author><name>Kat Hall</name><uri>http://search.theregister.co.uk/?author=Kat%20Hall</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/04/companies_need_answer_safe_harbour_minister/\"/><title type=\"html\">Companies need answer to Safe Harbour worries, says minister</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Swift conclusion on revised agreement needed&lt;/h4&gt; &lt;p&gt;The move to strike down Safe Harbour has created worrying uncertainty for companies, the Conservative minister for intellectual property, Baroness Neville-Rolfe, has said.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/04/huawei_nvme_dorado_adding_ssds/</id><updated>2015-11-04T12:05:07Z</updated><author><name>Chris Mellor</name><uri>http://search.theregister.co.uk/?author=Chris%20Mellor</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/04/huawei_nvme_dorado_adding_ssds/\"/><title type=\"html\">Huawei adds NVMe accessed SSDs to Dorado all-flash array</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Supposedly five times faster than XtremIO&lt;/h4&gt; &lt;p&gt;Huawei is updating its dual-controller OceanStor Dorado all-flash array by adding NVMe-accessed SSDs, reportedly boosting IOPS performance by up to 30 per cent.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/04/de_sousa_outsourcery/</id><updated>2015-11-04T10:23:09Z</updated><author><name>Paul Kunert</name><uri>http://search.channelregister.co.uk/?author=Paul%20Kunert</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.channelregister.co.uk/2015/11/04/de_sousa_outsourcery/\"/><title type=\"html\">Former Dragon lures reseller exec into cloud cave</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Outgoing Insight UK veep de Sousa to grab controls at Outsourcery on April Fool's Day&lt;/h4&gt; &lt;p&gt;Manchester-based cloud-slinger Outsourcery has confirmed that outgoing Insight Enterprises UK boss Emma de Sousa will land as managing director in April.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/04/clearsky_data_dash_for_cash_colo/</id><updated>2015-11-04T09:33:10Z</updated><author><name>Chris Mellor</name><uri>http://search.theregister.co.uk/?author=Chris%20Mellor</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/04/clearsky_data_dash_for_cash_colo/\"/><title type=\"html\">Going loco for co-lo: ClearSky Data's dash for the cash</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;CEO says startup wants to increase US coverage&lt;/h4&gt; &lt;p&gt;Startup &lt;a target=\"_blank\" href=\"http://www.theregister.co.uk/2015/08/27/dump_your_iscsi_san_and_go_clearsky/\"&gt;ClearSky Data&lt;/a&gt; has taken in its second round of venture capital dosh this year with a $27m B-round following an earlier $12m A-round.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/04/juniper_junos_and_nfv_launch/</id><updated>2015-11-04T07:57:06Z</updated><author><name>Richard Chirgwin</name><uri>http://search.theregister.co.uk/?author=Richard%20Chirgwin</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/04/juniper_junos_and_nfv_launch/\"/><title type=\"html\">Juniper hardware and software to sleep in separate beds</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Run Junos on white boxes? Sure! How about other NOSes on our boxes? Yep to that too&lt;/h4&gt; &lt;p&gt;Juniper has decided to embrace the spirit of the white box, by loosening the bonds between Juniper hardware and its Junos software.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/04/emc_flash_copy_data_management/</id><updated>2015-11-04T04:04:11Z</updated><author><name>Chris Mellor</name><uri>http://search.theregister.co.uk/?author=Chris%20Mellor</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/04/emc_flash_copy_data_management/\"/><title type=\"html\">This will get pulses racing, thinks EMC: Copy data management</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Consolidate primary and secondary workloads on flash array&lt;/h4&gt; &lt;p&gt;EMC's XtremIO all-flash array has had a copy data management facility added as the storage goliath takes on &lt;a target=\"_blank\" href=\"http://www.theregister.co.uk/2014/07/04/actifio_addz_cloud_dr/\"&gt;Actifio&lt;/a&gt;, &lt;a target=\"_blank\" href=\"http://www.theregister.co.uk/2015/07/06/catalogic_copy_data_clean_tool/\"&gt;Catalogic&lt;/a&gt;, and &lt;a target=\"_blank\" href=\"http://www.theregister.co.uk/2015/07/14/delphix_gets_more_cash_for_daas/\"&gt;Delphix&lt;/a&gt;.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/03/microsoft_onedrive_changes/</id><updated>2015-11-03T23:42:23Z</updated><author><name>Tim Anderson</name><uri>http://search.theregister.co.uk/?author=Tim%20Anderson</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/03/microsoft_onedrive_changes/\"/><title type=\"html\">Microsoft's OneDrive price hike has wrecked its cloud strategy</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Users unhappy with chopped-down cloud storage&lt;/h4&gt; &lt;p&gt;&lt;strong&gt;Comment&lt;/strong&gt;  Microsoft's changes to its OneDrive personal cloud storage prices reflect badly on the company, and have left users angry and bewildered.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/03/intels_allflash_data_center/</id><updated>2015-11-03T21:36:51Z</updated><author><name>Chris Mellor</name><uri>http://search.theregister.co.uk/?author=Chris%20Mellor</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/03/intels_allflash_data_center/\"/><title type=\"html\">Guess who: Storage chip maker [blank] can't wait for all-storage-chip data centers</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;It's Intel, we're talking about Intel&lt;/h4&gt; &lt;p&gt;&lt;strong&gt;+Comment&lt;/strong&gt;  Intel is pushing the idea of an all-flash data center so it can make up for slowing processor revenue growth by selling 3D NAND and XPoint chips and SSDs.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/03/os2_returns_arca_noae/</id><updated>2015-11-03T20:22:17Z</updated><author><name>Shaun Nichols</name><uri>http://search.theregister.co.uk/?author=Shaun%20Nichols</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/03/os2_returns_arca_noae/\"/><title type=\"html\">Is the world ready for a bare-metal OS/2 rebirth?</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;IBM and Arca Noae to free 1990s operating system from its virtualized cage&lt;/h4&gt; &lt;p&gt;A US software company has signed on with IBM to release a new native build of Big Blue's OS/2.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/03/ibm_buys_gravitant/</id><updated>2015-11-03T16:56:12Z</updated><author><name>Gavin Clarke</name><uri>http://search.theregister.co.uk/?author=Gavin%20Clarke</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/03/ibm_buys_gravitant/\"/><title type=\"html\">IBM bets on cloud broker with Gravitant buyout</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Hybrid IT gamble will be rolled into Global Services&lt;/h4&gt; &lt;p&gt;IBM has bought an 11-year-old firm promising cloud brokerage services to sustain the move to hybrid IT.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/03/enterprise_storage_in_flash/</id><updated>2015-11-03T16:32:08Z</updated><author><name>David Gordon</name><uri>http://search.theregister.co.uk/?author=David%20Gordon</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/03/enterprise_storage_in_flash/\"/><title type=\"html\">Enterprise Storage in Flash: A real world story</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Join our experts live on 15 December 11.00 GMT to 12.00 GMT&lt;/h4&gt; &lt;p&gt;&lt;strong&gt;Live Regcast&lt;/strong&gt;  SSDs are now firmly established in enterprise storage. In many organisations they are now being deployed to support an expanding range of workloads, not just one or two specialist services such as high performance analytics or desktop virtualisation.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/03/dell_asset_sales/</id><updated>2015-11-03T15:21:05Z</updated><author><name>Chris Mellor</name><uri>http://search.theregister.co.uk/?author=Chris%20Mellor</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/03/dell_asset_sales/\"/><title type=\"html\">What the bleedin' Dell is this? IT giant mulls unit selloffs – report</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;AppAssure becomes RapidRecovery becomes RapidDeparture...?&lt;/h4&gt; &lt;p&gt;Reuters reports Dell is thinking of selling off non-core assets to clear the decks before the EMC acquisition and raise $10bn.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/03/gridstore_going_ganggbusters/</id><updated>2015-11-03T12:00:11Z</updated><author><name>Chris Mellor</name><uri>http://search.theregister.co.uk/?author=Chris%20Mellor</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/03/gridstore_going_ganggbusters/\"/><title type=\"html\">Gridstore going gangbusters</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Hyper-V-focused, hyper-converged appliance start-up hitting the spot&lt;/h4&gt; &lt;p&gt;&lt;a target=\"_blank\" href=\"http://www.theregister.co.uk/2014/10/01/gridstore_hyperconverged_appliance/\"&gt;Gridstore&lt;/a&gt;, the Hyper-V-focused, hyper-converged, all-flash appliance startup, is growing gangbusters after coming out, so to speak, a year ago.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/03/moshes_massive_7niner_booms/</id><updated>2015-11-03T10:29:11Z</updated><author><name>Chris Mellor</name><uri>http://search.theregister.co.uk/?author=Chris%20Mellor</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/03/moshes_massive_7niner_booms/\"/><title type=\"html\">Moshe's monster seven-nines disk box blooms</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Infinidat and Huawei might be up to something&lt;/h4&gt; &lt;p&gt;Infinidat, the creation of storage industry guru Moshe Yanai, saw 61 per cent quarter-over-quarter growth in the third quarter.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/03/whats_up_with_nvme/</id><updated>2015-11-03T09:29:10Z</updated><author><name>Trevor Pott</name><uri>http://search.theregister.co.uk/?author=Trevor%20Pott</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/03/whats_up_with_nvme/\"/><title type=\"html\">Quick as a flash: NVMe will have you in tiers</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Our man Trevor gives his first impressions&lt;/h4&gt; &lt;p&gt;&lt;strong&gt;Sysadmin Blog&lt;/strong&gt;  Non-Volatile Memory Express, or NVMe, is a game-changing storage standard for PCIe-connected drives. It is replacing AHCI and along with the U.2 (SFF-8639) connector it is &lt;a href=\"http://www.theregister.co.uk/2014/11/13/flash_connectors/\"&gt;replacing both SAS and SATA&lt;/a&gt; for high speed, low latency storage. It's the smart way to connect up flash and post-flash storage tech to your servers.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/03/microsoft_hits_delete_key_onedrive_unlimited_no_more/</id><updated>2015-11-03T04:29:25Z</updated><author><name>Richard Chirgwin</name><uri>http://search.theregister.co.uk/?author=Richard%20Chirgwin</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/03/microsoft_hits_delete_key_onedrive_unlimited_no_more/\"/><title type=\"html\">At Microsoft 'unlimited cloud storage' really means one terabyte</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Office 365 OneDrive plans slashed after some folk upload 76TB apiece&lt;/h4&gt; &lt;p&gt;A year ago, it probably looked like a brilliant idea: bait products like Office 365 with unlimited cloud storage: documents and PowerPoints and Excel don't take up that much space, do they?…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/03/five_storage_stories/</id><updated>2015-11-03T04:02:06Z</updated><author><name>Chris Mellor</name><uri>http://search.theregister.co.uk/?author=Chris%20Mellor</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/03/five_storage_stories/\"/><title type=\"html\">And for your third course, a platter of flash cache, object, file copy, and private cloud news</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Storage stuff that missed the cut last week&lt;/h4&gt; &lt;p&gt;&lt;strong&gt;Roundup&lt;/strong&gt;  Here are five storage stories that missed the cut last week but are interesting, as they show movement in the object, flash, operational data, and private/public cloud areas. There's so much product development and startup work going in it's hard to keep up.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/02/australian_cloud_ninefold_gives_up_in_face_of_rich_rivals/</id><updated>2015-11-02T20:54:53Z</updated><author><name>Simon Sharwood</name><uri>http://search.theregister.co.uk/?author=Simon%20Sharwood</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/02/australian_cloud_ninefold_gives_up_in_face_of_rich_rivals/\"/><title type=\"html\">Ninefold gives up race against richer rivals</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Users of Australian cloud face Christmas migration sprint after startup sends self TITSUP&lt;/h4&gt; &lt;p&gt;Australian cloud outfit Ninefold has declared it has a total inability to support unlimited payments – TITSUP – and announced it is “sunsetting” itself.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/02/ibm_linux_mainframes/</id><updated>2015-11-02T16:57:06Z</updated><author><name>Liam Proven</name><uri>http://search.theregister.co.uk/?author=Liam%20Proven</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/02/ibm_linux_mainframes/\"/><title type=\"html\">Old, not obsolete: IBM takes Linux mainframes back to the future</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Your KVMs, give them to me&lt;/h4&gt; &lt;p&gt;IBM introduced several significant new elements for its Linux server stack last month: support for KVM on its z Systems mainframes, Linux-only models in both the z Systems and Power Systems ranges, and a new purchasing model.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/02/hp_inc_hp_enterprise_shares_wall_street/</id><updated>2015-11-02T16:37:50Z</updated><author><name>Kelly Fiveash</name><uri>http://search.theregister.co.uk/?author=Kelly%20Fiveash</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/02/hp_inc_hp_enterprise_shares_wall_street/\"/><title type=\"html\">HP Inc shares rocket 13% on Wall Street debut after split</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;HP Enterprise, meanwhile, dips more than 5% on NYSE&lt;/h4&gt; &lt;p&gt;Hewlett Packard's split on Sunday created an interesting moment on the New York stock market today: shares in HP Inc jumped more than 13 per cent, while HP Enterprise watched its shares fall as much as five per cent.…&lt;/p&gt; &lt;p&gt;&lt;!--#include virtual='/data_centre/_whitepaper_textlinks_top.html' --&gt;&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/02/hewlett_packard_two_companies/</id><updated>2015-11-02T15:27:05Z</updated><author><name>Gavin Clarke</name><uri>http://search.theregister.co.uk/?author=Gavin%20Clarke</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/02/hewlett_packard_two_companies/\"/><title type=\"html\">Whitman's split: The end of Fiorina's HP grand expansion era</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Let's start at the very beginning&lt;/h4&gt; &lt;p&gt;Hewlett Packard became &lt;a target=\"_blank\" href=\"http://www.theregister.co.uk/2015/11/02/hewlett_packard_enterprises_brand_new_fortune_50_company/\"&gt;two companies&lt;/a&gt; on 1 November, splitting enterprise from consumer.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/02/huawei_makes_its_own_pcie_ssd/</id><updated>2015-11-02T13:18:07Z</updated><author><name>Chris Mellor</name><uri>http://search.theregister.co.uk/?author=Chris%20Mellor</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/02/huawei_makes_its_own_pcie_ssd/\"/><title type=\"html\">Huawei cooks own PCIe SSD: Flash IP in a flash</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Micron NAND stores blocks of bits but the rest is all Huawei&lt;/h4&gt; &lt;p&gt;&lt;strong&gt;Pics&lt;/strong&gt;  Chinese enterprise data centre IT supplier Huawei now makes its own PCIe flash cards, including the controller functionality.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/02/endpoint_protectors_spread_their_wings/</id><updated>2015-11-02T12:59:33Z</updated><author><name>Chris Mellor</name><uri>http://search.theregister.co.uk/?author=Chris%20Mellor</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/02/endpoint_protectors_spread_their_wings/\"/><title type=\"html\">Endpoint protectors spread wings, Druva flies into Microsoft cloud</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Code42 and Druva hover near data centres&lt;/h4&gt; &lt;p&gt;Endpoint data protector Druva is adding Microsoft's Azure to its public cloud target list, adding security and sharing features to its backup capabilities and trying to appeal more to enterprises – a link with Microsoft is good news in that department.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/02/gcloud_scle_government/</id><updated>2015-11-02T12:44:11Z</updated><author><name>Kat Hall</name><uri>http://search.theregister.co.uk/?author=Kat%20Hall</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/02/gcloud_scle_government/\"/><title type=\"html\">G-Cloud sellers hit out against cap on scaling services</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;'We've not heard any sensible justification for this'&lt;/h4&gt; &lt;p&gt;G-Cloud suppliers have hit out against new rules to be introduced in the next iteration of the cloudy framework this month. The new guidelines will slap a 20 per cap on how much buyers can scale their services.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/02/huawei_micron_new_nvm_tech/</id><updated>2015-11-02T11:59:28Z</updated><author><name>Chris Mellor</name><uri>http://search.theregister.co.uk/?author=Chris%20Mellor</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/02/huawei_micron_new_nvm_tech/\"/><title type=\"html\">Huawei mixes it up with Micron and its new NVM tech</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;3D XPoint adoption looks a given&lt;/h4&gt; &lt;p&gt;China-based telecom and enterprise IT supplier Huawei has entered a flash partnership with Micron, which means the Chinese firm will be using Micron flash in its OceanStor storage arrays.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/02/hds_adds_locality_play_data_ingestor_to_its_object_storage/</id><updated>2015-11-02T10:58:10Z</updated><author><name>Chris Mellor</name><uri>http://search.theregister.co.uk/?author=Chris%20Mellor</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/02/hds_adds_locality_play_data_ingestor_to_its_object_storage/\"/><title type=\"html\">Are you local? HDS adds locality, Data Ingestor to object storage</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Time to think about 'data sovereignty', 'compliance risks'&lt;/h4&gt; &lt;p&gt;HDS has revved its object-storing HCP (Hitachi Content Platform) with what looks to be better data management and protection features.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/02/helium_internet_of_things_platform/</id><updated>2015-11-02T10:04:43Z</updated><author><name>Wireless Watch</name><uri>http://search.theregister.co.uk/?author=Wireless%20Watch</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/02/helium_internet_of_things_platform/\"/><title type=\"html\">Helium has a go at Internet of Things thing – using ultra-low power tech</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Platform includes homegrown sensors and 802.15.4&lt;/h4&gt; &lt;p&gt;As the internet of things (IoT) gets closer to commercial reality, the solutions flooding into the market are increasingly targeted at a real world use case. Some of these are extremely specific – smart meters and smart streetlights are commonplace now, but startup Helium Systems says its initial focus is on smart refrigeration.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/02/veams_virtual_virtuosity_gets_more_customers_by_the_balls/</id><updated>2015-11-02T00:58:05Z</updated><author><name>Chris Mellor</name><uri>http://search.theregister.co.uk/?author=Chris%20Mellor</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/02/veams_virtual_virtuosity_gets_more_customers_by_the_balls/\"/><title type=\"html\">Veeam's virtual virtuosity gets more customers by the balls</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Ratmir thinks legacy backup sucks, yah-boo!&lt;/h4&gt; &lt;p&gt;One of these days Veeam is going to stop growing. Sadly, for its competitors that day appears not to be today.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/11/01/no_were_not_sorry_for_xen_security_snafus_says_ian_jackson/</id><updated>2015-11-01T23:24:11Z</updated><author><name>Simon Sharwood</name><uri>http://search.theregister.co.uk/?author=Simon%20Sharwood</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/11/01/no_were_not_sorry_for_xen_security_snafus_says_ian_jackson/\"/><title type=\"html\">No, we're not sorry for Xen security SNAFUs says Ian Jackson</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Gandalf-grade developer says everything is insecure, so why single out Xen?&lt;/h4&gt; &lt;p&gt;Open source luminary Ian Jackson has hit back at criticism of the Xen Project's security.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/10/31/ico_advice_us_data_transfer_safe_harbor_ruling/</id><updated>2015-10-31T09:04:06Z</updated><author><name>OUT-LAW.COM</name><uri>http://search.theregister.co.uk/?author=OUT-LAW.COM</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/10/31/ico_advice_us_data_transfer_safe_harbor_ruling/\"/><title type=\"html\">UK watchdog offers 'safe harbor' advice on US data transfers</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Less tough than Germany&lt;/h4&gt; &lt;p&gt;David Smith, deputy information commissioner, said &lt;a href=\"https://iconewsblog.wordpress.com/2015/10/27/the-us-safe-harbor-breached-but-perhaps-not-destroyed/\"&gt;businesses should \"take stock\" of their data transfer arrangements&lt;/a&gt; and review whether they provide adequate protection of personal data, as is required by EU law.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/10/30/bonfire_of_onpremises_storage/</id><updated>2015-10-30T20:14:19Z</updated><author><name>Chris Mellor</name><uri>http://search.theregister.co.uk/?author=Chris%20Mellor</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/10/30/bonfire_of_onpremises_storage/\"/><title type=\"html\">Warm your fingers by the bonfire of vanity on-premises storage</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Rome’s burning faster than thought but slower than hoped&lt;/h4&gt; &lt;p&gt;&lt;strong&gt;Comment&lt;/strong&gt;  Public cloud storage supporters couldn't hope for a better cheerleader than Nasuni CEO Andres Rodriguez, whose company provides cloud storage gateways, including an on-premises caching device.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/10/30/we_are_entering_the_dataaware_infrastructure_era/</id><updated>2015-10-30T17:33:06Z</updated><author><name>Enrico Signoretti</name><uri>http://search.theregister.co.uk/?author=Enrico%20Signoretti</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/10/30/we_are_entering_the_dataaware_infrastructure_era/\"/><title type=\"html\">We are entering the data-aware infrastructure era</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Storage systems analyse and operate on their own data&lt;/h4&gt; &lt;p&gt;&lt;strong&gt;Comment&lt;/strong&gt;  Last week, during &lt;a target=\"_blank\" href=\"http://techfieldday.com/event/sfd8/\"&gt;SFD8&lt;/a&gt;, I got to meet with two incredibly interesting storage startups: Cohesity and Coho Data.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/10/30/last_order_cost_quantum_dear_limps_to_loss/</id><updated>2015-10-30T15:25:06Z</updated><author><name>Chris Mellor</name><uri>http://search.theregister.co.uk/?author=Chris%20Mellor</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/10/30/last_order_cost_quantum_dear_limps_to_loss/\"/><title type=\"html\">Last orders cost Quantum dear as it limps to $11.2m loss</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;CEO: Overall market challenging, but disk and tape held up well&lt;/h4&gt; &lt;p&gt;Storage company Quantum has confirmed its &lt;a target=\"_blank\" href=\"http://www.theregister.co.uk/2015/10/08/quantum_walloped_by_backend_loading/\"&gt;early October revenue warning&lt;/a&gt;, with second fiscal 2016 quarter revenues of $117m, and not the hoped-for $125m.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/10/30/whats_the_key_to_successful_it_delivery_in_the_future/</id><updated>2015-10-30T14:34:13Z</updated><author><name>David Gordon</name><uri>http://search.theregister.co.uk/?author=David%20Gordon</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/10/30/whats_the_key_to_successful_it_delivery_in_the_future/\"/><title type=\"html\">When DevOps met hybrid-cloud: The key to successful IT delivery</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Join our experts live on 18 November 11.00 GMT to 12.00 GMT&lt;/h4&gt; &lt;p&gt;&lt;strong&gt;Regcast&lt;/strong&gt;  Some of the most important developments in IT are taking place around DevOps and hybrid-cloud. So what do these two topics have to do with each other?…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/10/30/applecart_upset_by_ddn/</id><updated>2015-10-30T10:03:12Z</updated><author><name>Chris Mellor</name><uri>http://search.theregister.co.uk/?author=Chris%20Mellor</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/10/30/applecart_upset_by_ddn/\"/><title type=\"html\">DataDirect Networks begins its storage apple cart disruption ops</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Mainstreamers flounder while data storage company booms&lt;/h4&gt; &lt;p&gt;&lt;strong&gt;Comment&lt;/strong&gt;  DataDirect Networks is now in the apple cart upsetting business, selling its arrays into enterprises which don’t buy kit from mainstream storage companies as a result.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/10/30/disaster_recovery_thats_not_a_disaster/</id><updated>2015-10-30T07:01:09Z</updated><author><name>Tom Baines</name><uri>http://search.theregister.co.uk/?author=Tom%20Baines</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/10/30/disaster_recovery_thats_not_a_disaster/\"/><title type=\"html\">Have a Plan A, and Plan B – just don't go down with the ship</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Paranoia and obsessiveness will keep you afloat&lt;/h4&gt; &lt;p&gt;When planning for disaster recovery, our natural inclination is to focus on the technical design. We work to strike the perfect balance between controlling infrastructure spend and the required capacity.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/10/29/xen_security/</id><updated>2015-10-29T19:09:13Z</updated><author><name>Chris Williams</name><uri>http://search.theregister.co.uk/?author=Chris%20Williams</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/10/29/xen_security/\"/><title type=\"html\">Patch this braXen bug: Hypervisor hole lets guest VMs hijack hosts</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Seven-year-old privilege escalation vulnerability caused by C code entanglement&lt;/h4&gt; &lt;p&gt;The Xen hypervisor project today released &lt;a target=\"_blank\" href=\"http://xenbits.xen.org/xsa/\"&gt;nine security patches&lt;/a&gt; that should be applied ASAP – particularly the one that stops guest virtual machines seizing control of host servers.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/10/29/getting_the_fun_stuff_back_in_your_data_centre/</id><updated>2015-10-29T11:28:05Z</updated><author><name>Dave Cartwright</name><uri>http://search.theregister.co.uk/?author=Dave%20Cartwright</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/10/29/getting_the_fun_stuff_back_in_your_data_centre/\"/><title type=\"html\">How to get the fun stuff back in your data centre</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Face it, cloud is not fun or cool&lt;/h4&gt; &lt;p&gt;The cloud is a fabulous concept. If you want to try something out, or prototype your latest idea, or give yourself a relatively inexpensive disaster recovery setup, get in there and run up a cloud-based installation.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/10/29/wd_revenues_hurt_by_slack_pc_demand/</id><updated>2015-10-29T10:24:08Z</updated><author><name>Chris Mellor</name><uri>http://search.theregister.co.uk/?author=Chris%20Mellor</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/10/29/wd_revenues_hurt_by_slack_pc_demand/\"/><title type=\"html\">WD revenues hurt by slack PC demand</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;SSD growth stalls but solid company set fair&lt;/h4&gt; &lt;p&gt;Western Digital saw depressed PC and notebook sales affect its disk drive business in its latest quarter, with stalling SSD sales showing negative sequential growth.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/10/29/data_inexorable_flow_public_clouds/</id><updated>2015-10-29T09:30:12Z</updated><author><name>Chris Mellor</name><uri>http://search.theregister.co.uk/?author=Chris%20Mellor</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/10/29/data_inexorable_flow_public_clouds/\"/><title type=\"html\">That great sucking sound? It's data going into the public cloud</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;The great slurp is inevitable, just you watch&lt;/h4&gt; &lt;p&gt;&lt;a target=\"_blank\" href=\"http://www.theregister.co.uk/2015/07/24/google_gives_away_100_petabytes_of_storage_to_irritate_aws/\"&gt;Nearline storage in Google’s cloud&lt;/a&gt; is a new front in the vigorous marketing war being fought by public cloud providers to grab your data and convert your storage CAPEX into OPEX, meaning income for them.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/10/29/seagate_spy_drive/</id><updated>2015-10-29T09:01:06Z</updated><author><name>Chris Mellor</name><uri>http://search.theregister.co.uk/?author=Chris%20Mellor</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/10/29/seagate_spy_drive/\"/><title type=\"html\">Seagate unveils enlarged spy drive with support for 64 spycams</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;8TB surveillance spinner&lt;/h4&gt; &lt;p&gt;Seagate has launched an 8TB disk drive for surveillance use, enabling up to 6PB of CCTV data in a rack.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/10/29/oracle_hard_cloud_digger/</id><updated>2015-10-29T07:09:08Z</updated><author><name>Gavin Clarke</name><uri>http://search.theregister.co.uk/?author=Gavin%20Clarke</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/10/29/oracle_hard_cloud_digger/\"/><title type=\"html\">Empires of the mind: Oracle digs deep on cloud</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Back-pedalling fast, but where's the difference?&lt;/h4&gt; &lt;p&gt;Larry Ellison laughed off the cloud seven years ago. If anything, cloud meant nothing more than taking out a new full-page ad on the back of &lt;i&gt;The Economist&lt;/i&gt;.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/10/29/vmware_ponders_vdi_teleporting_with_swipebetweendevices_patent/</id><updated>2015-10-29T02:58:04Z</updated><author><name>Simon Sharwood</name><uri>http://search.theregister.co.uk/?author=Simon%20Sharwood</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/10/29/vmware_ponders_vdi_teleporting_with_swipebetweendevices_patent/\"/><title type=\"html\">VMware ponders VDI teleporting with swipe-between-devices patent</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Is this Virtzilla's riposte to Microsoft Continuum?&lt;/h4&gt; &lt;p&gt;When Windows 10 makes it to mobile devices, one of the more interesting features will be “Continuum”, a tool that will make it possible to plug a phone into a keyboard and monitor and use it as a PC.…&lt;/p&gt;</summary></entry><entry><id>tag:theregister.co.uk,2005:story/2015/10/29/oracle_m7_silicon/</id><updated>2015-10-29T00:44:50Z</updated><author><name>Iain Thomson</name><uri>http://search.theregister.co.uk/?author=Iain%20Thomson</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/10/29/oracle_m7_silicon/\"/><title type=\"html\">Oracle hardwires encryption and SQL hastening algorithms into Sparc M7 silicon</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Claims world-record breaking performance&lt;/h4&gt; &lt;p&gt;&lt;strong&gt;OpenWorld&lt;/strong&gt;  Oracle execs used the final keynote of this week's OpenWorld to praise their &lt;a href=\"http://www.theregister.co.uk/2015/10/28/oracle_sparc_m7/\"&gt;Sparc M7&lt;/a&gt; processor's ability to accelerate encryption and some SQL queries in hardware.…&lt;/p&gt; &lt;p&gt;&lt;!--#include virtual='/data_centre/_whitepaper_textlinks_top.html' --&gt;&lt;/p&gt;</summary></entry></feed>");
	//strcpy(docname, "<?xml version=\"1.0\" encoding=\"UTF-8\"?><feed><id>tag:theregister.co.uk,2005:feed/theregister.co.uk/</id><title>atribut title</title><author><name>Team Register</name><email>webmaster@theregister.co.uk</email><uri>http://www.theregister.co.uk/odds/about/contact/</uri></author><entry><updated>2015-10-24T20:07:12Z</updated><author><name>Kelly Fiveash</name><uri>http://search.theregister.co.uk/?author=Kelly%20Fiveash</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/10/24/doctor_who_woman_who_lived_review/\"/><title type=\"html\">It's all Me, Me, Me! in &lt;i&gt;Doctor Who&lt;/i&gt;'s &lt;i&gt;The Woman Who Lived&lt;/i&gt; but what of Clara's fate?</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Ahh yes, the hybrids have it&lt;/h4&gt; &lt;p&gt;&lt;strong&gt;TV Review&lt;/strong&gt;  Readers please note: THIS IS A POST-UK BROADCAST REVIEW – THERE WILL BE SPOILERS!…&lt;/p&gt; &lt;p&gt;&lt;!--#include virtual='/data_centre/_whitepaper_textlinks_top.html' --&gt;&lt;/p&gt;</summary></entry><entry><updated>2015-10-24T20:07:12Z</updated><author><name>Kelly Fiveash</name><uri>http://search.theregister.co.uk/?author=Kelly%20Fiveash</uri></author><link rel=\"alternate\" type=\"text/html\" href=\"http://go.theregister.com/feed/www.theregister.co.uk/2015/10/24/doctor_who_woman_who_lived_review/\"/><title type=\"html\">It's all Me, Me, Me! in &lt;i&gt;Doctor Who&lt;/i&gt;'s &lt;i&gt;The Woman Who Lived&lt;/i&gt; but what of Clara's fate?</title><summary type=\"html\" xml:base=\"http://www.theregister.co.uk/\">&lt;h4&gt;Ahh yes, the hybrids have it&lt;/h4&gt; &lt;p&gt;&lt;strong&gt;TV Review&lt;/strong&gt;  Readers please note: THIS IS A POST-UK BROADCAST REVIEW – THERE WILL BE SPOILERS!…&lt;/p&gt; &lt;p&gt;&lt;!--#include virtual='/data_centre/_whitepaper_textlinks_top.html' --&gt;&lt;/p&gt;</summary></entry></feed>");
	

	
	Param* parametr = new Param;
	//docname = argv[1];
	parseDoc (docname, parametr);
	delete[] docname;
	delete parametr;

	return (1);
}