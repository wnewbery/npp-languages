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
/**Lexer for http://haml.info/
 * Because most of HAML is context sensitive, this implements a near complete parser, rather than
 * just lexing tokens.
 */
class Haml : public BaseLexer
{
public:
	virtual void style(StyleStream &stream)override;
private:
	enum Style;
	unsigned _currentIndent;

	/**Parsers a new line / statement. This must be called on a starting line, not in the
	 * middle of a multi-line structure as there is no way to determine the syntax from
	 * such a position.
	 *
	 * The indent of this line should have already been read as _currentIndent.
	 */
	void line(StyleStream &stream);

	/**Styles a block as a htmlComment.*/
	void htmlComment(StyleStream &stream);
	void hamlComment(StyleStream &stream);
	void comment(StyleStream &stream, Style style);
	/**'%' tag line.*/
	void tag(StyleStream &stream);
	/**'#' tag line (implicit div) or '%tag#'*/
	void tagId(StyleStream &stream);
	/**'.' tag line (implicit div) or '%tag.'*/
	void tagClass(StyleStream &stream);
	/**Rest of tag line after after '%'*/
	void tagStart(StyleStream &stream);
	/**Filter block starting with ':'.*/
	void filter(StyleStream &stream);

	/**Ruby block with '-', '=', at the previous position.*/
	void rubyBlock(StyleStream &stream);
	/**Text line with Ruby #{interpolation}.*/
	void textLine(StyleStream &stream);
};
