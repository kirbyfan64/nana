/*
 *	An Implementation of Place for Layout
 *	Copyright(C) 2003-2013 Jinhao(cnjinhao@hotmail.com)
 *
 *	Distributed under the Nana Software License, Version 1.0.
 *	(See accompanying file LICENSE_1_0.txt or copy at
 *	http://nanapro.sourceforge.net/LICENSE_1_0.txt)
 *
 *	@file: nana/gui/place.cpp
 */

#ifndef NANA_GUI_PLACE_HPP
#define NANA_GUI_PLACE_HPP
#include <utility>
#include <nana/gui/basis.hpp>

namespace nana
{
namespace gui
{
	class place
	{
		typedef std::pair<window, unsigned>	fixed_t;
		typedef std::pair<window, int>	percent_t;
		typedef std::pair<window, std::pair<unsigned, unsigned> > room_t;

		struct implement;

		class field_t
		{
		public:
			virtual ~field_t() = 0;
			virtual field_t& operator<<(window wd)		= 0;
			virtual field_t& operator<<(unsigned gap)	= 0;
			virtual field_t& operator<<(const fixed_t& f)	= 0;
			virtual field_t& operator<<(const percent_t& p)	= 0;
			virtual field_t& operator<<(const room_t& r)	= 0;
			virtual field_t& fasten(window wd)	= 0;
		};
	public:
		place();
		place(window wd);
		~place();

		void div(const char* s);
		field_t& field(const char* name);
		void collocate();

		static fixed_t fixed(window wd, unsigned size);
		static percent_t percent(window wd, int per);
		static room_t room(window wd, unsigned w, unsigned h);
	private:
		implement * impl_;
	};
}//end namespace gui
}//end namespace nana

#endif //#ifndef NANA_GUI_PLACE_HPP