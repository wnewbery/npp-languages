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
#include "Markdown.h"
#include "Ruby.h"
#include "Html.h"
#include "Scss.h"
#include <memory>
/**Lexer for http://slim-lang.com/
 * Because most of Slim is context sensitive, this implements a near complete parser, rather than
 * just lexing tokens.
 */
class Slim : public BaseLexer
{
public:
	Slim();

	virtual void style(StyleStream &stream)override;
	virtual void SCI_METHOD Lex(unsigned int startPos, int lengthDoc, int initStyle, IDocument *pAccess)override;
private:
	//line flags
	static const int SAFE_START = 1;

	enum Style
	{
		DEFAULT = Ruby::DEFAULT,
		ERROR = Ruby::ERROR,
		OPERATOR = Ruby::OPERATOR,
		ID = 2,
		CLS = 3,
		TAG = Html::TAG,
		HTMLATTRIBUTE = Html::ATTRIBUTE,
		COMMENT = 4,
		HTMLCOMMENT = Html::COMMENT,
		HTMLIECOMMENT = Html::COMMENT,
		FILTER = 5,
		INCLUDE = 6,
		DOCTYPE = Html::DOCTYPE
	};
	Html _html;
	Markdown _markdown;
	Ruby _ruby;
	Scss _css;
	Scss _scss;
	unsigned _currentIndent;

	void line(StyleStream &stream);
	void tagOrFilter(StyleStream &stream);

	void commentBlock(StyleStream &stream);
	void htmlCommentBlock(StyleStream &stream);
	void commentBlock(StyleStream &stream, Style style);
	void ieConditionalCommentBlock(StyleStream &stream);

	void textBlock(StyleStream &stream);
	void rubyBlock(StyleStream &stream);
	void filterBlock(StyleStream &stream, const std::string &engine);

	void includeLine(StyleStream &stream);
	void doctypeLine(StyleStream &stream);
	void textLine(StyleStream &stream);
	void rubyLine(StyleStream &stream);

	void dynamicTag(StyleStream &stream);
	void tagStart(StyleStream &stream);
	void tagName(StyleStream &stream);
	void tagId(StyleStream &stream);
	void tagCls(StyleStream &stream);
	void tagAttrs(StyleStream &stream);
	void tagWrappedAttrs(StyleStream &stream);
	void tagEnd(StyleStream &stream);
	void splatAttrs(StyleStream &stream);
	void attrValue(StyleStream &stream);
};


