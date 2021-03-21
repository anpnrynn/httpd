#include <httphandlers.h>


HttpHandler *HttpHandler::hdlrObj;

HttpHandler::HttpHandler()
{
	httpHdlrs = new MapHttpHdlr();
}

HttpHandler::~HttpHandler()
{
	if( httpHdlrs )
		httpHdlrs->clear();
}

HttpHandler* HttpHandler::createInstance() 
{
	if( !hdlrObj )
	{
		hdlrObj = new HttpHandler();
	}			
	return hdlrObj;
}

void* HttpHandler::getHandler( string name )
{
    void *data = NULL;
	MapHttpHdlr::iterator i = httpHdlrs->find(name );
	if( i != httpHdlrs->end() )
	{
		data = i->second; 
	}
	return data;
}

void HttpHandler::addHandler( string name, void *data)
{
	if(!data)
		return;
	httpHdlrs->insert( pair<string, void*>(name, data) );
}

void HttpHandler::delHandler( string name )
{
	MapHttpHdlr::iterator i = httpHdlrs->find(name );
	if( i != httpHdlrs->end() )
	{
		if( i->second != NULL )
			free( i->second); 
	}
	httpHdlrs->erase(i);
}
