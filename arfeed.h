#include <iostream>
#include <string.h>
#include <cstring>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

void printHelp();
int parseParameters(int argc, char* argv[], Param* parametr);
