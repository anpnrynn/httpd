//Copyright Anoop Kumar Narayanan <anoop.kumar.narayanan@gmail.com> , LICENSE - GPLv2 / GPLv3
#ifndef MIME_TYPES_H
#define MIME_TYPES_H

#include <iostream>
using namespace std;

void setupContentTypes();
const char * identifyContentType ( char *url );


#endif
