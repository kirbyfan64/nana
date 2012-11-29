/*
 *	Basic Types definition
 *	Copyright(C) 2003-2012 Jinhao(cnjinhao@hotmail.com)
 *
 *	Distributed under the Nana Software License, Version 1.0. 
 *	(See accompanying file LICENSE_1_0.txt or copy at 
 *	http://nanapro.sourceforge.net/LICENSE_1_0.txt)
 *
 *	@file: nana/basic_types.cpp
 */

#include <nana/basic_types.hpp>

namespace nana
{
	//struct point
		point::point():x(0), y(0){}
		point::point(int x, int y):x(x), y(y){}

		bool point::operator==(const point& rhs) const
		{
			return ((x == rhs.x) && (y == rhs.y));
		}
		bool point::operator!=(const point& rhs) const
		{
			return ((x != rhs.x) || (y != rhs.y));
		}
		bool point::operator<(const point& rhs) const
		{
			return ((y < rhs.y) || (y == rhs.y && x < rhs.x));
		}
		bool point::operator<=(const point& rhs) const
		{
			return ((y < rhs.y) || (y == rhs.y && x <= rhs.x));
		}
		bool point::operator>(const point& rhs) const
		{
			return ((y > rhs.y) || (y == rhs.y && x > rhs.x));
		}
		bool point::operator>=(const point& rhs) const
		{
			return ((y > rhs.y) || (y == rhs.y && x >= rhs.x));
		}
	//end struct point

	//struct upoint
		upoint::upoint():x(0), y(0){}
		upoint::upoint(unsigned x, unsigned y):x(x), y(y){}
		bool upoint::operator==(const upoint& rhs) const
		{
			return ((x == rhs.x) && (y == rhs.y));
		}
		bool upoint::operator!=(const upoint& rhs) const
		{
			return ((x != rhs.x) || (y != rhs.y));
		}
		bool upoint::operator<(const upoint& rhs) const
		{
			return ((y < rhs.y) || (y == rhs.y && x < rhs.x));
		}
		bool upoint::operator<=(const upoint& rhs) const
		{
			return ((y < rhs.y) || (y == rhs.y && x <= rhs.x));
		}
		bool upoint::operator>(const upoint& rhs) const
		{
			return ((y > rhs.y) || (y == rhs.y && x > rhs.x));
		}
		bool upoint::operator>=(const upoint& rhs) const
		{
			return ((y > rhs.y) || (y == rhs.y && x >= rhs.x));
		}
	//end struct upoint

	//struct size
		size::size():width(0), height(0){}
		size::size(unsigned width, unsigned height):width(width), height(height){}

		bool size::operator==(const size& rhs) const
		{
			return (width == rhs.width) && (height == rhs.height);
		}

		bool size::operator!=(const size& rhs) const
		{
			return (width != rhs.width) || (height != rhs.height);
		}
	//end struct size

	//struct rectangle
		rectangle::rectangle()
			:x(0), y(0), width(0), height(0)
		{}

		rectangle::rectangle(int x, int y, unsigned width, unsigned height)
			:x(x), y(y), width(width), height(height)
		{}

		rectangle::rectangle(const size & sz)
			:x(0), y(0), width(sz.width), height(sz.height)
		{}

		rectangle::rectangle(const point & pos, const size& sz)
			: x(pos.x), y(pos.y), width(sz.width), height(sz.height)
		{}

		bool rectangle::operator==(const rectangle& rhs) const
		{
			return (width == rhs.width) && (height == rhs.height) && (x == rhs.x) && (y == rhs.y);
		}

		bool rectangle::operator!=(const rectangle& rhs) const
		{
			return (width != rhs.width) || (height != rhs.height) || (x != rhs.x) || (y != rhs.y);
		}

		rectangle & rectangle::operator=(const point& pos)
		{
			x = pos.x;
			y = pos.y;
			return *this;
		}

		rectangle & rectangle::operator=(const size& sz)
		{
			width = sz.width;
			height = sz.height;
			return *this;
		}

		rectangle& rectangle::pare_off(int pixels)
		{
			x += pixels;
			y += pixels;
			width -= (pixels << 1);
			height -= (pixels << 1);
			return *this;
		}
	//end struct rectangle

	//struct arrange
		arrange::arrange()
			: value(horizontal)
		{}

		arrange::arrange(arrange::t x)
			: value(x)
		{}

		arrange::operator arrange::t() const
		{
			return value;
		}

		arrange& arrange::operator=(arrange::t x)
		{
			value = x;
			return *this;
		}

		bool arrange::operator==(arrange::t x) const
		{
			return (value == x);
		}

		bool arrange::operator!=(arrange::t x) const
		{
			return (value != x);
		}
	//end struct arrange
}
