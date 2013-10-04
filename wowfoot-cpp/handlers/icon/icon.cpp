#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "blp/MemImage.h"
#include "mpq_libmpq04.h"
#include "icon.h"
#include "dbc.h"
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#ifdef WOWBOT
#define EASSERT(test) assert(test)
#define ERRNO(a) { if((a) < 0) { printf("On line %i in file %s:\n", __LINE__, __FILE__); abort(); } }
#else
#include "util/exception.h"
#define ICONDIR_BASE ""
#endif

#include "util/fileExists.h"

class FileDescriptor {
private:
	const int mFD;
public:
	FileDescriptor(int fd) : mFD(fd) {
		ERRNO(fd);
	}
	~FileDescriptor() {
		if(mFD > 0) {
			ERRNO(close(mFD));
		}
	}

	// operates like write(), except the full size is always written,
	// unless an error occurs.
	int writeFully(const void* src, size_t size) {
		size_t pos = 0;
		const char* ptr = (char*)src;
		while(pos < size) {
			ssize_t res = write(mFD, ptr, size - pos);
			assert(res != 0);
			if(res < 0)
				return res;
			pos += res;
			ptr += res;
		}
		return size;
	}
};

static string getIconBase(const char* name, const char* mpqName);

string getIconRaw(const char* path) {
	const char* fileName = strrchr(path, '\\');
	if(fileName)
		fileName++;
	else
		fileName = path;
	string mpqName = path + string(".blp");
	return getIconBase(fileName, mpqName.c_str());
}

string getIcon(const char* name) {
	string mpqName = string("Interface\\ICONS\\") + name + ".blp";
	return getIconBase(name, mpqName.c_str());
}

static string getIconBase(const char* name, const char* mpqName) {
	string httpName = string("icon/") + name + ".png";
	string localName = ICONDIR_BASE "build/" + httpName;
	if(fileExists(localName.c_str()))
#ifdef WOWBOT
		return localName;
#else
		return httpName;
#endif

	printf("Extracting icon %s...\n", name);

	MPQFile file(mpqName);
	//MemImage::s_bVerbose = true;

	if(file.getSize() > 0) {
#if 0
		FileDescriptor fd = open(localName.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
		ERRNO(fd.writeFully(file.getBuffer(), file.getSize()));
#endif
		MemImage img;
		bool res = img.LoadFromBLP((const BYTE*)file.getBuffer(),
			(DWORD)file.getSize());
		EASSERT(res);
#if 0
		string dumpName = string(name) + ".raw";
		FileDescriptor fd = open(dumpName.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
		ERRNO(fd.writeFully(img.GetBuffer(), img.GetBufferBytes()));
#endif
#if 0
		printf("Size: %i x %i\n", img.GetWidth(), img.GetHeight());
		res = img.RemoveAlpha();
		EASSERT(res);
#endif
		res = img.SaveToPNG(localName.c_str());
		EASSERT(res);
	} else {
		printf("Warning: %s not found.\n", name);
	}
#ifdef WOWBOT
	return localName;
#else
	return httpName;
#endif
}
