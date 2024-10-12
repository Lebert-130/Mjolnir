#ifndef FGD_H
#define FGD_H

#pragma warning(disable : 4786)
#include <map>
#include <string>
#include <vector>

#include "vector.h"

enum LexType
{
	AT,
	EQUALS,
	COLON,
	OPENBRACKET,
	CLOSEBRACKET,
	OPENPAREN,
	CLOSEPAREN,
	PLUS,
	COMMA,
	VALUE, //Any unquoted string
	STRING, //Any quoted string
	COMMENT,
	ENDOFFILE,
	UNKNOWN
};

enum ClassType
{
	BASECLASS,
	SOLIDCLASS,
	POINTCLASS
};

class Token
{
public:
	Token(LexType type, const std::string& lexeme)
		: m_type(type), m_lexeme(lexeme) {}

	Token() : m_type(UNKNOWN), m_lexeme("") {}

	LexType GetType() const { return m_type; }
	const std::string& GetLexeme() const { return m_lexeme; }
private:
	LexType m_type;
	std::string m_lexeme;
};

class LexReader
{
public:
	LexReader(FILE* file)
		: m_file(file), m_currentPos(0)
	{
		if (m_file)
			LoadNextLine();
	}

	Token GetNextToken();
	int FindPattern(const std::vector<std::vector<LexType>>& pattern, size_t& matchedPatternIndex);
private:
	FILE* m_file;
	std::string m_line;
	size_t m_currentPos;

	bool isEOF() const {
		return feof(m_file) && m_currentPos >= m_line.size();
	}

	char GetNextChar(){
		if (m_currentPos >= m_line.size()){
			if (!LoadNextLine())
				return '\0';
		}
		return m_line[m_currentPos++];
	}

	bool LoadNextLine(){
		if (feof(m_file))
			return false;

		char buffer[256];
		if (fgets(buffer, sizeof(buffer), m_file)){
			m_line = buffer;
			m_currentPos = 0;
			return true;
		}
		return false;
	}

	void SkipWhitespace(){
		while (m_currentPos < m_line.size() && isspace(m_line[m_currentPos]))
			m_currentPos++;

		if (m_currentPos >= m_line.size())
			LoadNextLine();
	}
};

struct Attribute {
	std::string name;
	std::string type;
	std::string description;
	std::string defaultvalue;
	std::string value;

	std::vector<std::string> choices;
};

typedef struct BaseClass {
	Vector sizeMin;
	Vector sizeMax;
	Vector color;

	std::string classname;
	ClassType type;
} BaseClass;

typedef struct : public BaseClass {
	std::string description;
	std::vector<Attribute> attributes;
} SolidClass;

typedef struct : public BaseClass {
	std::string description;
} PointClass;

class CFgd
{
public:
	void LoadFGD(const char* filename);
	void FGDToList(wxArrayString& array, bool isPointClass);
private:
	FILE* fgdFile;
	//We need to allocate a temporary base class inside this class so it's much easier to access
	//to it when we need it, before sending it to the Map.
	void* m_pTmpClass;
	//Same here for attributes before sending them to the classes
	Attribute* m_pAttribute;
};

extern std::map<std::string, PointClass> pcMap;
extern std::map<std::string, SolidClass> scMap;

#endif