#pragma once
#include <string>

class CBaseException
{
public:

	CBaseException(const char *file,
		unsigned int line,
		const std::string &note = "") :
		note(note),
		file(file),
		line(line)
	{}

	const std::string &GetNote() const
	{
		return note;
	}

	const std::string &GetFile() const
	{
		return file;
	}

	unsigned int GetLine() const 
	{
		return line;
	}

	std::string GetLocation() const
	{
		return std::string("Line [") + std::to_string(line) + "] in " + file;
	}

	virtual std::string GetFullMessage() const = 0;
	virtual std::string GetExceptionType() const = 0;

private:
	std::string note;
	std::string file;
	unsigned int line;
};

