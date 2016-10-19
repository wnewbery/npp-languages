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
#ifdef ERROR
#undef ERROR
#endif

#include "BaseLexer.h"
#include "Html.h"
#include <memory>
/**Lexer for Markdown.
 * Based loosley on http://spec.commonmark.org/0.26/
 *
 * Does not include HTML blocks
 */
class Markdown : public BaseLexer
{
public:
	static const unsigned TAB = 4;
	enum Style
	{
		DEFAULT = 0,
		ERROR   = 1,
		HTMLENTITY = 66,
		HEADER = 30,
		BOLD = 31,
		ITALIC = 32,
		CODE = 33,
		IMAGE = 36,
		LINKREF = 38,
		LINK = 38,
		BLOCKQUOTE= 39,
		LIST = 40,
		THEMATICBREAK = 41,
		STRIKETHROUGH = 42,
		HARDBREAK = 43
	};

	virtual void style(StyleStream &stream)override;

	void block(StyleStream &stream);
	/**All blocks except paragraph and indented code.*/
	bool paragraphBlock(StyleStream &stream);
	/**http://spec.commonmark.org/0.26/#thematic-breaks*/
	bool thematicBreak(StyleStream &stream);
	/**http://spec.commonmark.org/0.26/#atx-headings*/
	bool atxHeader(StyleStream &stream);
	/**http://spec.commonmark.org/0.26/#setext-headings*/
	bool setextHeading(StyleStream &stream);
	/**http://spec.commonmark.org/0.26/#indented-code-blocks*/
	bool indentedCode(StyleStream &stream);
	/**http://spec.commonmark.org/0.26/#fenced-code-blocks*/
	bool fencedCode(StyleStream &stream);
	/**http://spec.commonmark.org/0.26/#link-reference-definitions*/
	bool linkRefBlock(StyleStream &stream);
	/**http://spec.commonmark.org/0.26/#block-quotes*/
	bool blockQuote(StyleStream &stream);
	/**http://spec.commonmark.org/0.26/#paragraphs*/
	void paragraph(StyleStream &stream);
	/**http://spec.commonmark.org/0.26/#list-items*/
	bool list(StyleStream &stream);


	void styleInline(StyleStream &stream, Style defaultStyle=DEFAULT, char delimiter='\0', unsigned n = 0);
	/**http://spec.commonmark.org/0.26/#code-spans*/
	void inlineCode(StyleStream &stream);
	/**http://spec.commonmark.org/0.26/#links*/
	void link(StyleStream &stream, Style style);
};
