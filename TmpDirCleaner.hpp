#ifndef TMPDIRCLEANER_HPP
#define TMPDIRCLEANER_HPP

#include <string>

class TmpDirCleaner
{
private:
	std::string dir;

	TmpDirCleaner(const TmpDirCleaner&);
	TmpDirCleaner& operator=(const TmpDirCleaner&);
public:
	TmpDirCleaner(const std::string &directory);
	~TmpDirCleaner();
};

#endif
