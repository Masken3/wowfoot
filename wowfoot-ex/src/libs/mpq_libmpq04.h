#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef MPQ_H
#define MPQ_H

#include "visible.h"
#include "loadlib.h"
#include "libmpq/mpq.h"
#include <string.h>
#include <ctype.h>
#include <vector>
#include <iostream>
#include <deque>

using namespace std;

class MPQArchive
{

public:
    mpq_archive_s *mpq_a;

		MPQArchive(const char* filename, bool verbose = true);
    void close();

		void GetFileListCallback(void (*callback)(const string&)) {
        uint32_t filenum;
        if(libmpq__file_number(mpq_a, "(listfile)", &filenum)) return;
        libmpq__off_t size, transferred;
        libmpq__file_unpacked_size(mpq_a, filenum, &size);

        char *buffer = new char[(size_t)size];

        libmpq__file_read(mpq_a, filenum, (unsigned char*)buffer, size, &transferred);

        char seps[] = "\n";
        char *token;

        token = strtok( buffer, seps );
        uint32 counter = 0;
        while ((token != NULL) && (counter < size)) {
            //cout << token << endl;
            token[strlen(token) - 1] = 0;
            string s = token;
            callback(s);
            counter += strlen(token) + 2;
            token = strtok(NULL, seps);
        }

        delete[] buffer;
    }
};
typedef std::deque<MPQArchive*> ArchiveSet;
extern ArchiveSet gOpenArchives;

class MPQFile
{
    //MPQHANDLE handle;
    bool eof;
    char *buffer;
    size_t pointer,size;

    // disable copying
    MPQFile(const MPQFile &f) {}
    void operator=(const MPQFile &f) {}

public:
    // Searches open archives for the specified file.
    // Archive are searched in reverse order of opening,
    // so you'll want to open the oldest archive first.
    MPQFile(const char* filename) VISIBLE;    // filenames are not case sensitive
    ~MPQFile() { close(); }
    size_t read(void* dest, size_t bytes);
    size_t getSize() { return size; }
    size_t getPos() { return pointer; }
    char* getBuffer() { return buffer; }
    char* getPointer() { return buffer + pointer; }
    bool isEof() { return eof; }
    void seek(int offset);
    void seekRelative(int offset);
    void close() VISIBLE;
};

inline void flipcc(char *fcc)
{
    char t;
    t=fcc[0];
    fcc[0]=fcc[3];
    fcc[3]=t;
    t=fcc[1];
    fcc[1]=fcc[2];
    fcc[2]=t;
}

#endif
