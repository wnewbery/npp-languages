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
#include "BaseLexer.h"
#include <memory>
/**Lexer for Ruby*/
class Ruby : public BaseLexer
{
public:
	enum Style
	{
		DEFAULT = 0,
		OPERATOR = 50,
		STRING = 51,
		INSTRUCTION = 52
	};

	virtual void style(StyleStream &stream)override;
	/**Style a upto the end of the line. Used by HAML etc.*/
	void styleLine(StyleStream &stream);
	/**A double or single quoted string.*/
	void string(StyleStream &stream);
	/**Rest of the line as a string (no delimiter)*/
	void stringLine(StyleStream &stream);
};
