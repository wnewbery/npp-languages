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
class Scss : public BaseLexer
{
public:
	enum Style
	{
		DEFAULT = 0,
		ERROR = 1,
		TAG = 100,
		CLASS,
		ID,
		VARIABLE,
		NUMBER,
		STRING,
		OPERATOR,
		FUNCTION,
		IMPORTANT,
		CSS_COMMENT,
		SCSS_COMMENT,
		COLOR,
		PSEUDO
	};

	virtual void style(StyleStream &stream)override;


	void globalLine(StyleStream &stream);
	void globalStatement(StyleStream &stream);


	void errorStatement(StyleStream &stream);
	bool basicStatement(StyleStream &stream);

	void cssComment(StyleStream &stream);
	void variableDef(StyleStream &stream);
	void assignment(StyleStream &stream);
	void value(StyleStream &stream);
	void number(StyleStream &stream);
	void hexColor(StyleStream &stream);
	void string(StyleStream &stream);

	void selector(StyleStream &stream);
	bool selectorElement(StyleStream &stream);

	/**Declaration block can contain any basicStatement, including nested selectors plus CSS styles.*/
	void declarationBlock(StyleStream &stream);

	void mediaQuery(StyleStream &stream);
	void mixin(StyleStream &stream);
};
