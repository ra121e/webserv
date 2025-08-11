#include "TmpDirCleaner.hpp"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>

TmpDirCleaner::TmpDirCleaner(const std::string &directory): dir(directory)
{
}

TmpDirCleaner::~TmpDirCleaner()
{
	DIR *dirp = opendir(dir.c_str());
	if (dirp == NULL)
	{
		// Directory might not exist; nothing to do
		return;
	}
	struct dirent *ent = NULL;
	while ((ent = readdir(dirp)) != NULL)
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
	closedir(dirp);
}
