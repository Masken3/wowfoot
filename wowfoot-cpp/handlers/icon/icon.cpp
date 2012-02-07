#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "blp/MemImage.h"
#include "mpq_libmpq04.h"
#include "icon.h"
#include "dbc.h"
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include "util/exception.h"

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
	printf("Loading icon %s\n", path);
	const char* fileName = strrchr(path, '\\');
	if(fileName)
		fileName++;
	else
		fileName = path;
	string mpqName = path + string(".blp");
	return getIconBase(fileName, mpqName.c_str());
}

string getIcon(const char* name) {
	printf("Loading icon %s\n", name);
	string mpqName = string("Interface\\ICONS\\") + name + ".blp";
	return getIconBase(name, mpqName.c_str());
}

static string getIconBase(const char* name, const char* mpqName) {
	DBC::load();
	MPQFile file(mpqName);

	string httpName = string("icon/") + name + ".jpg";
	if(file.getSize() > 0) {
		string localName = "build/" + httpName;
#if 0
		FileDescriptor fd = open(localName.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
		ERRNO(fd.writeFully(file.getBuffer(), file.getSize()));
#endif
		MemImage img;
		bool res = img.LoadFromBLP((const BYTE*)file.getBuffer(),
			(DWORD)file.getSize());
		EASSERT(res);
#if 0
		printf("Size: %i x %i\n", img.GetWidth(), img.GetHeight());
		res = img.RemoveAlpha();
		EASSERT(res);
#endif
		res = img.SaveToJPEG(localName.c_str());
		EASSERT(res);
	} else {
		printf("Warning: %s not found.\n", name);
	}
	return httpName;
}
