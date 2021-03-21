#include <mimetypes.h>
#include <apptypes.h>
#include <iostream>
using namespace std;

MapStrStr * contentType = NULL;

void setupContentTypes() {
    if ( !contentType ) {
        contentType = new MapStrStr();
        ( *contentType ) ["a"] = "application/octet-stream";
        ( *contentType ) ["aab"] = "application/x-authorware-bin";
        ( *contentType ) ["aam"] = "application/x-authorware-map";
        ( *contentType ) ["aas"] = "application/x-authorware-seg";
        ( *contentType ) ["ai"] = "application/postscript";
        ( *contentType ) ["aif"] = "audio/x-aiff";
        ( *contentType ) ["aifc"] = "audio/x-aiff";
        ( *contentType ) ["aiff"] = "audio/x-aiff";
        ( *contentType ) ["asc"] = "text/plain";
        ( *contentType ) ["asf"] = "video/x-ms-asf";
        ( *contentType ) ["asx"] = "video/x-ms-asf";
        ( *contentType ) ["au"] = "audio/basic";
        ( *contentType ) ["avi"] = "video/x-msvideo";
        ( *contentType ) ["bcpio"] = "application/x-bcpio";
        ( *contentType ) ["bin"] = "application/octet-stream";
        ( *contentType ) ["bmp"] = "image/bmp";
        ( *contentType ) ["cdf"] = "application/x-netcdf";
        ( *contentType ) ["class"] = "application/x-java-vm";
        ( *contentType ) ["cpio"] = "application/x-cpio";
        ( *contentType ) ["cpt"] = "application/mac-compactpro";
        ( *contentType ) ["crl"] = "application/x-pkcs7-crl";
        ( *contentType ) ["crt"] = "application/x-x509-ca-cert";
        ( *contentType ) ["csh"] = "application/x-csh";
        ( *contentType ) ["css"] = "text/css";
        ( *contentType ) ["dcr"] = "application/x-director";
        ( *contentType ) ["dir"] = "application/x-director";
        ( *contentType ) ["djv"] = "image/vnd.djvu";
        ( *contentType ) ["djvu"] = "image/vnd.djvu";
        ( *contentType ) ["dll"] = "application/octet-stream";
        ( *contentType ) ["dms"] = "application/octet-stream";
        ( *contentType ) ["doc"] = "application/msword";
        ( *contentType ) ["dtd"] = "text/xml";
        ( *contentType ) ["dump"] = "application/octet-stream";
        ( *contentType ) ["dvi"] = "application/x-dvi";
        ( *contentType ) ["dxr"] = "application/x-director";
        ( *contentType ) ["eps"] = "application/postscript";
        ( *contentType ) ["etx"] = "text/x-setext";
        ( *contentType ) ["exe"] = "application/octet-stream";
        ( *contentType ) ["ez"] = "application/andrew-inset";
        ( *contentType ) ["fgd"] = "application/x-director";
        ( *contentType ) ["fh"] = "image/x-freehand";
        ( *contentType ) ["fh4"] = "image/x-freehand";
        ( *contentType ) ["fh5"] = "image/x-freehand";
        ( *contentType ) ["fh7"] = "image/x-freehand";
        ( *contentType ) ["fhc"] = "image/x-freehand";
        ( *contentType ) ["gif"] = "image/gif";
        ( *contentType ) ["gtar"] = "application/x-gtar";
        ( *contentType ) ["hdf"] = "application/x-hdf";
        ( *contentType ) ["hqx"] = "application/mac-binhex40";
        ( *contentType ) ["htm"] = "text/html; charset]=%s";
        ( *contentType ) ["html"] = "text/html; charset]=%s";
        ( *contentType ) ["ice"] = "x-conference/x-cooltalk";
        ( *contentType ) ["ief"] = "image/ief";
        ( *contentType ) ["iges"] = "model/iges";
        ( *contentType ) ["igs"] = "model/iges";
        ( *contentType ) ["iv"] = "application/x-inventor";
        ( *contentType ) ["jar"] = "application/x-java-archive";
        ( *contentType ) ["jfif"] = "image/jpeg";
        ( *contentType ) ["jpe"] = "image/jpeg";
        ( *contentType ) ["jpeg"] = "image/jpeg";
        ( *contentType ) ["jpg"] = "image/jpeg";
        ( *contentType ) ["js"] = "application/x-javascript";
        ( *contentType ) ["kar"] = "audio/midi";
        ( *contentType ) ["latex"] = "application/x-latex";
        ( *contentType ) ["lha"] = "application/octet-stream";
        ( *contentType ) ["lzh"] = "application/octet-stream";
        ( *contentType ) ["m3u"] = "audio/x-mpegurl";
        ( *contentType ) ["man"] = "application/x-troff-man";
        ( *contentType ) ["mathml"] = "application/mathml+xml";
        ( *contentType ) ["me"] = "application/x-troff-me";
        ( *contentType ) ["mesh"] = "model/mesh";
        ( *contentType ) ["mid"] = "audio/midi";
        ( *contentType ) ["midi"] = "audio/midi";
        ( *contentType ) ["mif"] = "application/vnd.mif";
        ( *contentType ) ["mime"] = "message/rfc822";
        ( *contentType ) ["mml"] = "application/mathml+xml";
        ( *contentType ) ["mov"] = "video/quicktime";
        ( *contentType ) ["movie"] = "video/x-sgi-movie";
        ( *contentType ) ["mp2"] = "audio/mpeg";
        ( *contentType ) ["mp3"] = "audio/mpeg";
        ( *contentType ) ["mp4"] = "video/mp4";
        ( *contentType ) ["mpe"] = "video/mpeg";
        ( *contentType ) ["mpeg"] = "video/mpeg";
        ( *contentType ) ["mpg"] = "video/mpeg";
        ( *contentType ) ["mpga"] = "audio/mpeg";
        ( *contentType ) ["ms"] = "application/x-troff-ms";
        ( *contentType ) ["msh"] = "model/mesh";
        ( *contentType ) ["mv"] = "video/x-sgi-movie";
        ( *contentType ) ["mxu"] = "video/vnd.mpegurl";
        ( *contentType ) ["nc"] = "application/x-netcdf";
        ( *contentType ) ["o"] = "application/octet-stream";
        ( *contentType ) ["oda"] = "application/oda";
        ( *contentType ) ["ogg"] = "application/x-ogg";
        ( *contentType ) ["pac"] = "application/x-ns-proxy-autoconfig";
        ( *contentType ) ["pbm"] = "image/x-portable-bitmap";
        ( *contentType ) ["pdb"] = "chemical/x-pdb";
        ( *contentType ) ["pdf"] = "application/pdf";
        ( *contentType ) ["pgm"] = "image/x-portable-graymap";
        ( *contentType ) ["pgn"] = "application/x-chess-pgn";
        ( *contentType ) ["png"] = "image/png";
        ( *contentType ) ["pnm"] = "image/x-portable-anymap";
        ( *contentType ) ["ppm"] = "image/x-portable-pixmap";
        ( *contentType ) ["ppt"] = "application/vnd.ms-powerpoint";
        ( *contentType ) ["ps"] = "application/postscript";
        ( *contentType ) ["qt"] = "video/quicktime";
        ( *contentType ) ["ra"] = "audio/x-realaudio";
        ( *contentType ) ["ram"] = "audio/x-pn-realaudio";
        ( *contentType ) ["ras"] = "image/x-cmu-raster";
        ( *contentType ) ["rdf"] = "application/rdf+xml";
        ( *contentType ) ["rgb"] = "image/x-rgb";
        ( *contentType ) ["rm"] = "audio/x-pn-realaudio";
        ( *contentType ) ["roff"] = "application/x-troff";
        ( *contentType ) ["rpm"] = "audio/x-pn-realaudio-plugin";
        ( *contentType ) ["rss"] = "application/rss+xml";
        ( *contentType ) ["rtf"] = "text/rtf";
        ( *contentType ) ["rtx"] = "text/richtext";
        ( *contentType ) ["sgm"] = "text/sgml";
        ( *contentType ) ["sgml"] = "text/sgml";
        ( *contentType ) ["sh"] = "application/x-sh";
        ( *contentType ) ["shar"] = "application/x-shar";
        ( *contentType ) ["silo"] = "model/mesh";
        ( *contentType ) ["sit"] = "application/x-stuffit";
        ( *contentType ) ["skd"] = "application/x-koan";
        ( *contentType ) ["skm"] = "application/x-koan";
        ( *contentType ) ["skp"] = "application/x-koan";
        ( *contentType ) ["skt"] = "application/x-koan";
        ( *contentType ) ["smi"] = "application/smil";
        ( *contentType ) ["smil"] = "application/smil";
        ( *contentType ) ["snd"] = "audio/basic";
        ( *contentType ) ["so"] = "application/octet-stream";
        ( *contentType ) ["spl"] = "application/x-futuresplash";
        ( *contentType ) ["src"] = "application/x-wais-source";
        ( *contentType ) ["stc"] = "application/vnd.sun.xml.calc.template";
        ( *contentType ) ["std"] = "application/vnd.sun.xml.draw.template";
        ( *contentType ) ["sti"] = "application/vnd.sun.xml.impress.template";
        ( *contentType ) ["stw"] = "application/vnd.sun.xml.writer.template";
        ( *contentType ) ["sv4cpio"] = "application/x-sv4cpio";
        ( *contentType ) ["sv4crc"] = "application/x-sv4crc";
        ( *contentType ) ["svg"] = "image/svg+xml";
        ( *contentType ) ["svgz"] = "image/svg+xml";
        ( *contentType ) ["swf"] = "application/x-shockwave-flash";
        ( *contentType ) ["sxc"] = "application/vnd.sun.xml.calc";
        ( *contentType ) ["sxd"] = "application/vnd.sun.xml.draw";
        ( *contentType ) ["sxg"] = "application/vnd.sun.xml.writer.global";
        ( *contentType ) ["sxi"] = "application/vnd.sun.xml.impress";
        ( *contentType ) ["sxm"] = "application/vnd.sun.xml.math";
        ( *contentType ) ["sxw"] = "application/vnd.sun.xml.writer";
        ( *contentType ) ["t"] = "application/x-troff";
        ( *contentType ) ["tar"] = "application/x-tar";
        ( *contentType ) ["tcl"] = "application/x-tcl";
        ( *contentType ) ["tex"] = "application/x-tex";
        ( *contentType ) ["texi"] = "application/x-texinfo";
        ( *contentType ) ["texinfo"] = "application/x-texinfo";
        ( *contentType ) ["tif"] = "image/tiff";
        ( *contentType ) ["tiff"] = "image/tiff";
        ( *contentType ) ["tr"] = "application/x-troff";
        ( *contentType ) ["tsp"] = "application/dsptype";
        ( *contentType ) ["tsv"] = "text/tab-separated-values";
        ( *contentType ) ["txt"] = "text/plain; charset]=%s";
        ( *contentType ) ["ustar"] = "application/x-ustar";
        ( *contentType ) ["vcd"] = "application/x-cdlink";
        ( *contentType ) ["vrml"] = "model/vrml";
        ( *contentType ) ["vx"] = "video/x-rad-screenplay";
        ( *contentType ) ["wav"] = "audio/x-wav";
        ( *contentType ) ["wax"] = "audio/x-ms-wax";
        ( *contentType ) ["wbmp"] = "image/vnd.wap.wbmp";
        ( *contentType ) ["wbxml"] = "application/vnd.wap.wbxml";
        ( *contentType ) ["wm"] = "video/x-ms-wm";
        ( *contentType ) ["wma"] = "audio/x-ms-wma";
        ( *contentType ) ["wmd"] = "application/x-ms-wmd";
        ( *contentType ) ["wml"] = "text/vnd.wap.wml";
        ( *contentType ) ["wmlc"] = "application/vnd.wap.wmlc";
        ( *contentType ) ["wmls"] = "text/vnd.wap.wmlscript";
        ( *contentType ) ["wmlsc"] = "application/vnd.wap.wmlscriptc";
        ( *contentType ) ["wmv"] = "video/x-ms-wmv";
        ( *contentType ) ["wmx"] = "video/x-ms-wmx";
        ( *contentType ) ["wmz"] = "application/x-ms-wmz";
        ( *contentType ) ["wrl"] = "model/vrml";
        ( *contentType ) ["wsrc"] = "application/x-wais-source";
        ( *contentType ) ["wvx"] = "video/x-ms-wvx";
        ( *contentType ) ["xbm"] = "image/x-xbitmap";
        ( *contentType ) ["xht"] = "application/xhtml+xml";
        ( *contentType ) ["xhtml"] = "application/xhtml+xml";
        ( *contentType ) ["xls"] = "application/vnd.ms-excel";
        ( *contentType ) ["xml"] = "text/xml";
        ( *contentType ) ["xpm"] = "image/x-xpixmap";
        ( *contentType ) ["xsl"] = "text/xml";
        ( *contentType ) ["xul"] = "application/vnd.mozilla.xul+xml";
        ( *contentType ) ["xwd"] = "image/x-xwindowdump";
        ( *contentType ) ["xyz"] = "chemical/x-xyz";
        ( *contentType ) ["zip"] = "application/zip";
    }
}

const char * identifyContentType ( char *dUrl ) {
// url is null terminated
    static const char *empty = "";

    if ( !dUrl )
    { return ( const char * ) empty; }

    char *url = dUrl;

    while ( *url != '\0' && *url != '.' ) {
        url++;
    }

    if ( *url == '.' )
    { url++; }
    else
    { return ( const char * ) empty; }

    MapStrStr::iterator i = contentType->find ( url );

    if ( i != contentType->end() ) {
        return i->second.c_str();
    }

    return ( const char * ) empty;
}
