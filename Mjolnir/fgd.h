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

class CFgd
{
public:
	void LoadFGD(const char* filename);
	void FGDToList(wxArrayString& array, bool isPointClass);
private:
	FILE* fgdFile;
};

struct Attribute {
	std::string name;
	std::string type;
	std::string description;
	std::string defaultvalue;
	std::string value;

	std::vector<std::string> choices;
};

struct SharedFields {
	Vector sizeMin;
	Vector sizeMax;
	Vector color;

	char classname[24];
	ClassType type;
};

typedef struct BaseClass : public SharedFields {

} BaseClass;

typedef struct : public BaseClass {
	char description[64];
	std::vector<Attribute> attributes;

	//std::vector<Attribute<std::vector<std::string>>> choiceAttributes;
	//std::vector<Attribute<std::string>> stringAttributes;
	//The reason why this vector exists is to keep consistency. I don't want to put all the choice attributes at the end of the list.
	//std::vector<std::string> allAttributes;
} SolidClass;

typedef struct : public BaseClass {
	char description[64];
} PointClass;

extern std::map<std::string, PointClass> pcMap;
extern std::map<std::string, SolidClass> scMap;

#endif