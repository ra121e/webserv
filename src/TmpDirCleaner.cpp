#include "../include/TmpDirCleaner.hpp"
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <cerrno>
#include <stdexcept>
#include "../include/Directory.hpp"

TmpDirCleaner::TmpDirCleaner(const std::string &directory): dir(directory)
{
	if (mkdir(directory.c_str(), 0755) == -1 && errno != EEXIST)
	{
		throw std::runtime_error("Failed to create temporary directory: "
			+ std::string(strerror(errno)));
	}
}

TmpDirCleaner::~TmpDirCleaner()
{
	Directory dirp(dir.c_str());

	struct dirent *ent = NULL;
	while ((ent = dirp.read()) != NULL)
	{
		// skip . and ..
		const std::string name(static_cast<const char*>(ent->d_name));
		if (name == "." || name == "..")
		{
			continue;
		}
		std::string path = dir;
		if (!path.empty() && path[path.size() - 1] != '/')
		{
			path += '/';
		}
		path += name;

		struct stat stbuf = {};
		if (lstat(path.c_str(), &stbuf) == 0)
		{
			if (S_ISREG(stbuf.st_mode))
			{
				// Best-effort unlink; ignore errors
				unlink(path.c_str());
			}
		}
	}
}
