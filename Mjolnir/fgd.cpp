#pragma warning(disable : 4786)
#include <Windows.h>
#include <stdio.h>
#include <map>
#include <string>
#include <algorithm>
#include <ctype.h>

#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#	include "wx/wx.h"
#endif

#include "vector.h"
#include "fgd.h"

std::map<std::string, PointClass> pcMap;
std::map<std::string, SolidClass> scMap;

auto ParseClassType(const std::string& type) -> BaseClass*
{
	if (type.compare("BaseClass") == 0){
		auto* tmp_baseclass = new BaseClass();
		tmp_baseclass->type = BASECLASS;
		return tmp_baseclass;
	}
	if (type.compare("PointClass") == 0){
		auto* tmp_pointclass = new PointClass();
		tmp_pointclass->type = POINTCLASS;
		return tmp_pointclass;
	}
	if (type.compare("SolidClass") == 0){
		auto* tmp_solidclass = new SolidClass();
		tmp_solidclass->type = SOLIDCLASS;
		return tmp_solidclass;
	}
	return nullptr;
}

std::vector<Token> tokens;
size_t currentTokenIndex = 0;

//TODO: Reinforce the names of some variables such as: base(), size() and color()

void CFgd::LoadFGD(const char* filename)
{
	fgdFile = fopen(filename, "r");
	if(!fgdFile){
		char message[64];
		sprintf(message, "Failed to load %s FGD file.", filename);
		MessageBox(NULL, message, "FGD Error", MB_ICONERROR|MB_OK);
		exit(-1);
	}

	std::map<std::string, BaseClass> bcMap;

	LexReader lexer(fgdFile);
	Token token;

	while ((token = lexer.GetNextToken()).GetType() != ENDOFFILE){
		if (token.GetType() == UNKNOWN){
			char message[64];
			sprintf(message, "Unknown token: %s", token.GetLexeme().c_str());
			MessageBox(NULL, message, "FGD Error", MB_ICONERROR | MB_OK);
			exit(-1);
		}

		tokens.push_back(token);
	}

	std::vector<std::vector<LexType>> patterns = {
		{ AT }, //@TypeClass
		{ EQUALS, VALUE }, //= value
		{ COLON, STRING, COLON, VALUE }, //: "string" : value
		{ VALUE, COLON, STRING }, //value : "string"
		{ COLON, STRING }, //: "string"
		{ EQUALS, OPENBRACKET }, //= [
		{ OPENBRACKET }, //[
		{ VALUE, OPENPAREN, VALUE, VALUE, VALUE, COMMA, VALUE, VALUE, VALUE, CLOSEPAREN }, //value(value value value, value value value)
		{ VALUE, OPENPAREN, VALUE, VALUE, VALUE, CLOSEPAREN }, //value(value value value)
		{ VALUE, OPENPAREN, VALUE, CLOSEPAREN }, //value(value)
		{ CLOSEBRACKET } //]
	};

	int matchIndex;
	int levelOfBrackets = 0;
	size_t matchedPatternIndex;

	while ((matchIndex = lexer.FindPattern(patterns, matchedPatternIndex)) != -1){
		if (matchedPatternIndex == 0){
			auto entityClass = ParseClassType(tokens[currentTokenIndex].GetLexeme());

			if (entityClass != nullptr)
				m_pTmpClass = (void*)entityClass;
			else{
				MessageBox(NULL, "Failed to create entity class", "FGD Error", MB_ICONERROR | MB_OK);
				exit(-1);
			}
		}
		else if (matchedPatternIndex == 1){
			BaseClass* tmp_baseclass = (BaseClass*)m_pTmpClass;
			if (tmp_baseclass){
				tmp_baseclass->classname = tokens[matchIndex + 1].GetLexeme();
			}
			else {
				MessageBox(NULL, "Entity class is not initialized", "FGD Error", MB_ICONERROR | MB_OK);
				exit(-1);
			}
		}
		else if (matchedPatternIndex == 2){
			if (m_pAttribute){
				std::string tmp_lexeme = tokens[matchIndex + 1].GetLexeme();
				tmp_lexeme.erase(std::remove(tmp_lexeme.begin(), tmp_lexeme.end(), '"'), tmp_lexeme.end());

				m_pAttribute->description = tmp_lexeme;
				m_pAttribute->defaultvalue = tokens[matchIndex + 3].GetLexeme();
			}
			else {
				MessageBox(NULL, "Attribute is not initialized", "FGD Error", MB_ICONERROR | MB_OK);
				exit(-1);
			}
		}
		else if (matchedPatternIndex == 3){
			if (levelOfBrackets == 2){
				if (m_pAttribute){
					std::string tmp_lexeme = tokens[matchIndex + 2].GetLexeme();
					tmp_lexeme.erase(std::remove(tmp_lexeme.begin(), tmp_lexeme.end(), '"'), tmp_lexeme.end());
					m_pAttribute->choices.push_back(tmp_lexeme);
				}
				else {
					MessageBox(NULL, "Attribute is not initialized", "FGD Error", MB_ICONERROR | MB_OK);
					exit(-1);
				}
			}
			else {
				MessageBox(NULL, "Outside brackets of the attribute", "FGD Error", MB_ICONERROR | MB_OK);
				exit(-1);
			}
		}
		else if (matchedPatternIndex == 4){
			std::string tmp_lexeme = tokens[matchIndex + 1].GetLexeme();
			tmp_lexeme.erase(std::remove(tmp_lexeme.begin(), tmp_lexeme.end(), '"'), tmp_lexeme.end());

			if (levelOfBrackets == 0){
				BaseClass* tmp_baseclass = (BaseClass*)m_pTmpClass;
				if (tmp_baseclass){
					if (tmp_baseclass->type == SOLIDCLASS){
						SolidClass* tmp_solidclass = static_cast<SolidClass*>(tmp_baseclass);
						tmp_solidclass->description = tmp_lexeme;
					}
					else if (tmp_baseclass->type == POINTCLASS){
						PointClass* tmp_pointclass = static_cast<PointClass*>(tmp_baseclass);
						tmp_pointclass->description = tmp_lexeme;
					}
				}
				else {
					MessageBox(NULL, "Entity class is not initialized", "FGD Error", MB_ICONERROR | MB_OK);
					exit(-1);
				}
			}
			else if (levelOfBrackets == 1){
				if (m_pAttribute){
					m_pAttribute->description = tmp_lexeme;
				}
				else {
					MessageBox(NULL, "Attribute is not initialized", "FGD Error", MB_ICONERROR | MB_OK);
					exit(-1);
				}
			}
		}
		else if (matchedPatternIndex == 5){
			//Sanity check
			if (!m_pAttribute){
				MessageBox(NULL, "Attribute is not initialized", "FGD Error", MB_ICONERROR | MB_OK);
				exit(-1);
			}
			levelOfBrackets++;
		}
		else if (matchedPatternIndex == 6){
			levelOfBrackets++;
		}
		else if (matchedPatternIndex == 7){
			BaseClass* tmp_baseclass = (BaseClass*)m_pTmpClass;
			if (tmp_baseclass){
				tmp_baseclass->sizeMin = Vector(std::stof(tokens[matchIndex + 2].GetLexeme()), std::stof(tokens[matchIndex + 3].GetLexeme()), std::stof(tokens[matchIndex + 4].GetLexeme()));
				tmp_baseclass->sizeMax = Vector(std::stof(tokens[matchIndex + 6].GetLexeme()), std::stof(tokens[matchIndex + 7].GetLexeme()), std::stof(tokens[matchIndex + 8].GetLexeme()));
			}
			else {
				MessageBox(NULL, "Entity class is not initialized", "FGD Error", MB_ICONERROR | MB_OK);
				exit(-1);
			}
		}
		else if (matchedPatternIndex == 8){
			BaseClass* tmp_baseclass = (BaseClass*)m_pTmpClass;
			if (tmp_baseclass){
				tmp_baseclass->color = Vector(std::stof(tokens[matchIndex + 2].GetLexeme()), std::stof(tokens[matchIndex + 3].GetLexeme()), std::stof(tokens[matchIndex + 4].GetLexeme()));
			}
			else {
				MessageBox(NULL, "Entity class is not initialized", "FGD Error", MB_ICONERROR | MB_OK);
				exit(-1);
			}
		}
		else if (matchedPatternIndex == 9){
			if (levelOfBrackets == 0){
				BaseClass* tmp_baseclass = (BaseClass*)m_pTmpClass;
				if (tmp_baseclass){
					tmp_baseclass->sizeMin = bcMap[tokens[matchIndex + 2].GetLexeme()].sizeMin;
					tmp_baseclass->sizeMax = bcMap[tokens[matchIndex + 2].GetLexeme()].sizeMax;
					tmp_baseclass->color = bcMap[tokens[matchIndex + 2].GetLexeme()].color;
				}
				else {
					MessageBox(NULL, "Entity class is not initialized", "FGD Error", MB_ICONERROR | MB_OK);
					exit(-1);
				}
			}
			if (levelOfBrackets == 1){
				if (m_pAttribute == nullptr){
					m_pAttribute = new Attribute();
				}
				else {
					BaseClass* tmp_baseclass = (BaseClass*)m_pTmpClass;
					if (tmp_baseclass){
						if (tmp_baseclass->type == SOLIDCLASS){
							SolidClass* tmp_solidclass = static_cast<SolidClass*>(tmp_baseclass);
							tmp_solidclass->attributes.push_back(*m_pAttribute);
						}
					}
					else {
						MessageBox(NULL, "Entity class is not initialized", "FGD Error", MB_ICONERROR | MB_OK);
						exit(-1);
					}

					delete m_pAttribute;
					m_pAttribute = new Attribute();
				}

				m_pAttribute->name = tokens[matchIndex].GetLexeme();
				m_pAttribute->type = tokens[matchIndex + 2].GetLexeme();
			}
		}
		else if (matchedPatternIndex == 10){
			levelOfBrackets--;

			if (levelOfBrackets == 0){
				BaseClass* tmp_baseclass = (BaseClass*)m_pTmpClass;
				if (tmp_baseclass){
					if (tmp_baseclass->type == BASECLASS){
						bcMap[tmp_baseclass->classname] = *tmp_baseclass;
					}
					if (tmp_baseclass->type == POINTCLASS){
						PointClass* tmp_pointclass = static_cast<PointClass*>(tmp_baseclass);
						pcMap[tmp_pointclass->classname] = *tmp_pointclass;
					}
					if (tmp_baseclass->type == SOLIDCLASS){
						SolidClass* tmp_solidclass = static_cast<SolidClass*>(tmp_baseclass);
						//Push the last remaining attribute
						tmp_solidclass->attributes.push_back(*m_pAttribute);
						scMap[tmp_solidclass->classname] = *tmp_solidclass;
					}
				}
				else {
					MessageBox(NULL, "Entity class is not initialized", "FGD Error", MB_ICONERROR | MB_OK);
					exit(-1);
				}
			}
		}

		currentTokenIndex = matchIndex + patterns[matchedPatternIndex].size();
	}
}

void CFgd::FGDToList(wxArrayString& array, bool isPointClass)
{
	if (isPointClass){
		for (std::map<std::string, PointClass>::iterator it = pcMap.begin(); it != pcMap.end(); it++)
			array.Add(it->first);
	}
	else {
		for (std::map<std::string, SolidClass>::iterator it = scMap.begin(); it != scMap.end(); it++)
			array.Add(it->first);
	}
}

int LexReader::FindPattern(const std::vector<std::vector<LexType>>& patterns, size_t& matchedPatternIndex) {
	size_t startIndex = currentTokenIndex;

	while (currentTokenIndex < tokens.size()){
		for (size_t i = 0; i < patterns.size(); i++){
			const auto& pattern = patterns[i];
			size_t patternIndex = 0;
			size_t tokenIndex = currentTokenIndex;

			while (tokenIndex < tokens.size() && patternIndex < pattern.size()){
				if (tokens[tokenIndex].GetType() != pattern[patternIndex])
					break; //Stop if any token doesn't match
				patternIndex++;
				tokenIndex++;
			}

			if (patternIndex == pattern.size()){
				matchedPatternIndex = i;
				return startIndex;
			}
		}

		currentTokenIndex++;
		startIndex = currentTokenIndex;
	}

	//No matches found
	return -1;
}

Token LexReader::GetNextToken()
{
	SkipWhitespace();

	if (isEOF())
		return Token(ENDOFFILE, "");

	char ch = m_line[m_currentPos];

	if (ch == '\n'){
		SkipWhitespace();
		ch = m_line[m_currentPos];
	}

	if (ch == ' '){
		while (m_currentPos < m_line.size() && isspace(m_line[m_currentPos]))
			m_currentPos++;
		ch = m_line[m_currentPos];
	}

	if (isalnum(ch) || ch == '-'){
		std::string value;
		while (m_currentPos < m_line.size() && m_line.size() && m_line[m_currentPos] != '(' && m_line[m_currentPos] != ' ' && m_line[m_currentPos] != ')' && m_line[m_currentPos] != ',')
			value += m_line[m_currentPos++];
		return Token(VALUE, value);
	}

	if (ch == '"'){
		std::string string;
		while (m_currentPos < m_line.size() && m_line.size() && m_line[m_currentPos] != '\n'){
			string += m_line[m_currentPos++];
			if (m_line[m_currentPos] == '"'){
				string += m_line[m_currentPos++];
				return Token(STRING, string);
			}
		}

		MessageBox(NULL, "Found unclosed string", "FGD Error", MB_ICONERROR | MB_OK);
		exit(-1);
	}

	if (ch == '@'){
		m_currentPos++;
		std::string classtype;
		while (m_currentPos < m_line.size() && m_line.size() && m_line[m_currentPos] != '\n' && !isspace(m_line[m_currentPos]))
			classtype += m_line[m_currentPos++];
		return Token(AT, classtype);
	}

	if (ch == '='){
		m_currentPos++;
		return Token(EQUALS, std::string(1, ch));
	}

	if (ch == ':'){
		m_currentPos++;
		return Token(COLON, std::string(1, ch));
	}

	if (ch == '['){
		m_currentPos++;
		return Token(OPENBRACKET, std::string(1, ch));
	}

	if (ch == ']'){
		m_currentPos++;
		return Token(CLOSEBRACKET, std::string(1, ch));
	}

	if (ch == '('){
		m_currentPos++;
		return Token(OPENPAREN, std::string(1, ch));
	}

	if (ch == ')'){
		m_currentPos++;
		return Token(CLOSEPAREN, std::string(1, ch));
	}

	if (ch == ','){
		m_currentPos++;
		return Token(COMMA, std::string(1, ch));
	}

	return Token(UNKNOWN, std::string(1, ch++));
}