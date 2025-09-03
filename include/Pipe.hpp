#ifndef PIPE_HPP
#define PIPE_HPP
#include "BaseFile.hpp"

class Pipe
{
private:
	BaseFile read_end;
	BaseFile write_end;
public:
	Pipe();
	const BaseFile& operator[](int index) const;
	BaseFile& 		operator[](int index);
};

#endif
