#include "stdafx.h"
#include "common.h"

static voidpf ZCALLBACK open(voidpf opaque, const char* filename, int mode)
{
	QFile::OpenMode openmode = 0;
	if ((mode & ZLIB_FILEFUNC_MODE_READWRITEFILTER) == ZLIB_FILEFUNC_MODE_READ)
		openmode = QFile::ReadOnly;
	else if (mode & ZLIB_FILEFUNC_MODE_EXISTING)
		openmode = QFile::ReadWrite | QFile::Truncate;
	else if (mode & ZLIB_FILEFUNC_MODE_CREATE)
		openmode = QFile::WriteOnly | QFile::Truncate;

	if (filename && openmode)
	{
		QFile* file = new QFile(QString::fromUtf8(filename));
		if (file->open(openmode))
			return file;
		else
			delete file;
	}

	// If we get here, there's been an error
	return NULL;
}

static uLong ZCALLBACK read(voidpf opaque, voidpf stream, void* buf, uLong size)
{
	QFile* f = reinterpret_cast<QFile*>(stream);
	qint64 ret = f->read( reinterpret_cast<char*>(buf), size );
	return static_cast<uLong>(ret);
}

static uLong ZCALLBACK write(voidpf opaque, voidpf stream, const void* buf, uLong size)
{
	QFile* f = reinterpret_cast<QFile*>(stream);
	qint64 ret = f->write( reinterpret_cast<const char*>(buf), size );
	return static_cast<uLong>(ret);
}

static long ZCALLBACK tell(voidpf opaque, voidpf stream)
{
	QFile* f = reinterpret_cast<QFile*>(stream);
	return static_cast<long>(f->pos());
}

static long ZCALLBACK seek(voidpf opaque, voidpf stream, uLong offset, int origin)
{
	QFile* f = reinterpret_cast<QFile*>(stream);
	qint64 pos;
	switch (origin)
	{
	case ZLIB_FILEFUNC_SEEK_CUR:
		pos = f->pos() + offset;
		break;
	case ZLIB_FILEFUNC_SEEK_END:
		pos = f->size() - offset;
		break;
	case ZLIB_FILEFUNC_SEEK_SET:
		pos = offset;
		break;
	default:
		return -1;
	}
	f->seek(pos);
	return 0;
}

static int ZCALLBACK close(voidpf opaque, voidpf stream)
{
	QFile* f = reinterpret_cast<QFile*>(stream);
	delete f;
	return 0;
}

static int ZCALLBACK testerror(voidpf opaque, voidpf stream)
{
	QFile* f = reinterpret_cast<QFile*>(stream);
	return static_cast<int>(f->error());
}

zlib_filefunc_def g_zipFileFuncs = { open, read, write, tell, seek, close, testerror, NULL };

