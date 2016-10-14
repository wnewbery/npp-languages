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
#include <memory>
#include <cassert>
class IDocument; //Scintilla
/**Interface for IDocument to read source text and write styles.*/
class StyleStream
{
public:
	StyleStream(IDocument *doc);

	/**Sends any remainig style output to notepad++.*/
	void finish();
	/**@return True if _srcPos == _len*/
	bool eof()const
	{
		assert(_srcPos <= _len);
		return _srcPos == _len;
	}

	/**Get the next source element, but do not advance.*/
	char peek()const
	{
		assert(!eof());
		return _src[_srcPos];
	}
	/**Peeks further ahead. Returns '\0' if out of range.*/
	char peek(unsigned offset)const
	{
		if (_srcPos + offset >= _len) return '\0';
		else return _src[_srcPos + offset];
	}
	/**Advance a byte (e.g. after peek).
	 * _stylePos must equal _srcPos.
	 */
	void advance(char style)
	{
		assert(!eof());
		assert(_stylePos == _srcPos);
		_styles[_stylePos] = style;
		++_stylePos;
		++_srcPos;
	}

	/**Start a line and return its indent level.
	 * The current _srcPos should be a line start, or the results will be incorrect.
	 * _stylePos must equal _srcPos, and style will be advanced by setting the default style (0).
	 * Tabs and spaces are used for indent, tabs counting as one space.
	 * @return The number of indents processed.
	 */
	unsigned nextIndent();

	/**Advance to the next line, setting its style.
	 * _stylePos must equal _srcPos.
	 */
	void readRestOfLine(char style);
private:
	IDocument *_doc;
	//TODO: Investigate different strategies here
	//The basic assumption with this implementation is that reading and writing single bytes to
	//Notepad++ will be slow, and that documents will never be large enough that making complete
	//copies will be an issue.
	/**Position within the document where _styles and _str start.*/
	unsigned _docPos;
	/**Length of _styles and _str.*/
	unsigned _len;
	/**Current style position in _styles.*/
	unsigned _stylePos;
	/**Current read position in _str.*/
	unsigned _srcPos;
	/**Styles for each source text byte.*/
	std::unique_ptr<char[]> _styles;
	/**Source text to style.*/
	std::unique_ptr<char[]> _src;
};
