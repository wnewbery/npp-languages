// This file is part of Extra Languages for Notepad++.
// Copyright (C) 2016 William Newbery <wnewbery@hotmail.co.uk>
//
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include <ILexer.h> //Scintilla
#include <memory>
#include "StyleStream.h"

class BaseLexer : public ILexer
{
public:
	//BaseLexer API
	virtual ~BaseLexer() {}

	virtual void style(StyleStream &stream) = 0;

	//Scintilla API
	virtual int SCI_METHOD Version()const override
	{
		return 0;
	}
	virtual void SCI_METHOD Release()override
	{
		delete this;
	}
	virtual const char * SCI_METHOD PropertyNames()override
	{
		return "";
	}
	virtual int SCI_METHOD PropertyType(const char *name)override
	{
		return 0;
	}
	virtual const char * SCI_METHOD DescribeProperty(const char *name)
	{
		return "";
	}
	virtual int SCI_METHOD PropertySet(const char *key, const char *val)override
	{
		return 0;
	}
	virtual const char * SCI_METHOD DescribeWordListSets()override
	{
		return "HTML Tag Names";
	}
	virtual int SCI_METHOD WordListSet(int n, const char *wl)override
	{
		return 0;
	}
	virtual void SCI_METHOD Lex(unsigned int startPos, int lengthDoc, int initStyle, IDocument *pAccess)override
	{
		DocumentStyleStream stream(pAccess);
		style(stream);
		stream.finish();
	}
	virtual void SCI_METHOD Fold(unsigned int startPos, int lengthDoc, int initStyle, IDocument *pAccess)override
	{
	}
	virtual void * SCI_METHOD PrivateCall(int operation, void *pointer)override
	{
		return 0;
	}
};

struct LexerInfo
{
	const char *name;
	const wchar_t *desc;
	ILexer *(*factory)();
};
template<typename T>
ILexer *lexerFactory()
{
	return new T();
}
