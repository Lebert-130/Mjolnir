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

auto ParseClassType(const std::string& type) -> SharedFields*
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

	void* tmp_class = nullptr;
	auto iterator = tokens.begin();
	int levelsOfBrackets = 0;
	std::vector<Attribute> attributes;

	while (iterator != tokens.end()){
		if (iterator->GetType() == AT){
			auto entityClass = ParseClassType(iterator->GetLexeme());

			if (entityClass != nullptr)
				tmp_class = (void*)entityClass;
			else {
				MessageBox(NULL, "Failed to create entity class", "FGD Error", MB_ICONERROR | MB_OK);
				exit(-1);
			}
		}

		if (iterator->GetType() == VALUE){
			if (levelsOfBrackets == 1){
				Attribute tmp_atr;

				tmp_atr.name = iterator->GetLexeme();

				iterator++;

				if (iterator->GetType() != OPENPAREN){
					MessageBox(NULL, "No type for attribute", "FGD Error", MB_ICONERROR | MB_OK);
					exit(-1);
				}

				iterator++;

				tmp_atr.type = iterator->GetLexeme();

				iterator++;

				if (iterator->GetType() != CLOSEPAREN){
					MessageBox(NULL, "Unclosed attribute", "FGD Error", MB_ICONERROR | MB_OK);
					exit(-1);
				}

				iterator++;
				iterator++;

				std::string tmp_lexeme = iterator->GetLexeme();
				tmp_lexeme.erase(std::remove(tmp_lexeme.begin(), tmp_lexeme.end(), '"'), tmp_lexeme.end());

				tmp_atr.description = tmp_lexeme;

				iterator++;

				if (iterator->GetType() == VALUE){
					attributes.push_back(tmp_atr);
					continue;
				}

				if (levelsOfBrackets == 1){
					if (iterator->GetType() == CLOSEBRACKET){
						attributes.push_back(tmp_atr);
						continue;
					}
				}

				iterator++;

				tmp_atr.defaultvalue = iterator->GetLexeme();

				iterator++;

				if (iterator->GetType() == VALUE){
					attributes.push_back(tmp_atr);
					continue;
				}

				iterator++;
				iterator++;
				iterator++;
				iterator++;

				levelsOfBrackets++;

				std::vector<std::string> choices;

				tmp_lexeme = iterator->GetLexeme();
				tmp_lexeme.erase(std::remove(tmp_lexeme.begin(), tmp_lexeme.end(), '"'), tmp_lexeme.end());

				choices.push_back(tmp_lexeme);

				iterator++;
				iterator++;
				iterator++;

				tmp_lexeme = iterator->GetLexeme();
				tmp_lexeme.erase(std::remove(tmp_lexeme.begin(), tmp_lexeme.end(), '"'), tmp_lexeme.end());

				choices.push_back(tmp_lexeme);
				iterator++;
				iterator++;
				iterator++;

				tmp_lexeme = iterator->GetLexeme();
				tmp_lexeme.erase(std::remove(tmp_lexeme.begin(), tmp_lexeme.end(), '"'), tmp_lexeme.end());

				choices.push_back(tmp_lexeme);

				tmp_atr.choices = choices;

				attributes.push_back(tmp_atr);
				iterator++;
				continue;
			}

			iterator--;
			if (iterator->GetType() == EQUALS){
				iterator++;
				BaseClass* base = (BaseClass*)tmp_class;
				strcpy(base->classname, iterator->GetLexeme().c_str());
			}
			else {
				iterator++;
			}

			if ((iterator->GetLexeme().compare("size") == 0)){
				iterator++;
				Vector minSize;

				for (int i = 0; i < 3; i++){
					iterator++;
					minSize[i] = atoi(iterator->GetLexeme().c_str());
				}

				iterator++;
				Vector maxSize;

				if (iterator->GetType() != COMMA){
					MessageBox(NULL, "No max size for entity found", "FGD Error", MB_ICONERROR | MB_OK);
					exit(-1);
				}

				for (int i = 0; i < 3; i++){
					iterator++;
					maxSize[i] = atoi(iterator->GetLexeme().c_str());
				}

				iterator++;
				if (iterator->GetType() != CLOSEPAREN){
					MessageBox(NULL, "Unclosed size", "FGD Error", MB_ICONERROR | MB_OK);
					exit(-1);
				}
				else {
					if (tmp_class != nullptr) {
						BaseClass* base = (BaseClass*)tmp_class;
						base->sizeMin = minSize;
						base->sizeMax = maxSize;
					}
					else {
						MessageBox(NULL, "Entity class is not initialized", "FGD Error", MB_ICONERROR | MB_OK);
						exit(-1);
					}
				}
			}
			if ((iterator->GetLexeme().compare("color") == 0)){
				iterator++;
				Vector color;

				for (int i = 0; i < 3; i++){
					iterator++;
					color[i] = atoi(iterator->GetLexeme().c_str());
				}

				iterator++;
				if (iterator->GetType() != CLOSEPAREN){
					MessageBox(NULL, "Unclosed color", "FGD Error", MB_ICONERROR | MB_OK);
					exit(-1);
				}
				else {
					if (tmp_class != nullptr) {
						BaseClass* base = (BaseClass*)tmp_class;
						base->color = color;
					}
					else {
						MessageBox(NULL, "Entity class is not initialized", "FGD Error", MB_ICONERROR | MB_OK);
						exit(-1);
					}
				}
			}
			if ((iterator->GetLexeme().compare("base") == 0)){
				char message[64];

				iterator++;

				if (iterator->GetType() != OPENPAREN){
					MessageBox(NULL, "No base class", "FGD Error", MB_ICONERROR | MB_OK);
					exit(-1);
				}

				iterator++;

				BaseClass* base = (BaseClass*)tmp_class;

				base->sizeMin = bcMap[iterator->GetLexeme()].sizeMin;
				base->sizeMax = bcMap[iterator->GetLexeme()].sizeMax;
				base->color = bcMap[iterator->GetLexeme()].color;

				iterator++;

				if (iterator->GetType() != CLOSEPAREN){
					MessageBox(NULL, "Unclosed base", "FGD Error", MB_ICONERROR | MB_OK);
					exit(-1);
				}
			}
		}

		if (iterator->GetType() == STRING){
			iterator--;
			if (iterator->GetType() != COLON){
				MessageBox(NULL, "Disconnected string", "FGD Error", MB_ICONERROR | MB_OK);
				exit(-1);
			}
			iterator++;

			std::string tmp_lexeme = iterator->GetLexeme();
			tmp_lexeme.erase(std::remove(tmp_lexeme.begin(), tmp_lexeme.end(), '"'), tmp_lexeme.end());

			if (levelsOfBrackets == 0){
				BaseClass* base = (BaseClass*)tmp_class;

				if (base->type == POINTCLASS){
					PointClass* point = static_cast<PointClass*>(base);
					strcpy(point->description, tmp_lexeme.c_str());
				}
				if (base->type == SOLIDCLASS){
					SolidClass* solid = static_cast<SolidClass*>(base);
					strcpy(solid->description, tmp_lexeme.c_str());
				}
			}
		}

		if (iterator->GetType() == OPENBRACKET)
			levelsOfBrackets++;

		if (iterator->GetType() == CLOSEBRACKET){
			levelsOfBrackets--;

			if (levelsOfBrackets == 0){
				BaseClass* base = (BaseClass*)tmp_class;

				if (base->type == BASECLASS){
					bcMap[base->classname] = *base;
					delete base;
				}
				else if (base->type == POINTCLASS){
					PointClass* point = static_cast<PointClass*>(base);
					pcMap[point->classname] = *point;
					delete point;
				}
				else {
					SolidClass* solid = static_cast<SolidClass*>(base);
					solid->attributes = attributes;
					scMap[solid->classname] = *solid;
					delete solid;
				}
			}
		}

		iterator++;
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