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
#ifdef ERROR
#undef ERROR
#endif
/**Lexer for Ruby*/
class Ruby : public BaseLexer
{
public:
	enum Style
	{
		DEFAULT = 0,
		ERROR = 1,
		COMMENT = 4,
		NUMBER = 81,
		INSTRUCTION = 82,
		STRING = 83,
		CHARACTER = 84,
		CLASS_DEF = 85,
		METHOD_DEF = 86,
		OPERATOR = 87,
		REGEX= 89,
		GLOBAL = 90,
		SYMBOL = 91,
		MODULE_DEF = 92,
		INSTANCE_VAR = 93,
		CLASS_VAR = 94,
		BACKTICKS = 95
	};

	virtual void style(StyleStream &stream)override;
	/**Style a upto the end of the line. Used by HAML etc.*/
	void styleLine(StyleStream &stream);
	/**A quoted string, regex, etc. string with escapes and interpolation.*/
	void string(StyleStream &stream);
	void stringBody(StyleStream &stream,
		char delimL, char delimR,
		Style style, bool interpolated);
	void regex(StyleStream &stream);
	void regexModifiers(StyleStream &stream);
	/**Rest of the line as a string (no delimiter)*/
	void stringLine(StyleStream &stream);
	/**Interpolated string content.*/
	void stringInterp(StyleStream &stream);
	/**Some token on the line.*/
	void token(StyleStream &stream);
	/**Name string for variable, symbol, etc.*/
	void name(StyleStream &stream, Style style, bool method=false);

	/**Finds the number of elements to the next #{} interpolation on the current line.
	 * @return Offset of #{, end of line, or end of file.
	 */
	unsigned findNextInterp(StyleStream &stream);
};
