/*
 *	Platform Specification Implementation
 *	Copyright(C) 2003-2012 Jinhao(cnjinhao@hotmail.com)
 *
 *	Distributed under the Nana Software License, Version 1.0.
 *	(See accompanying file LICENSE_1_0.txt or copy at
 *	http://stdex.sourceforge.net/LICENSE_1_0.txt)
 *
 *	@file: nana/detail/platform_spec.hpp
 *
 *	This file provides basis class and data structrue that required by nana
 *	This file should not be included by any header files.
 */

#ifndef NANA_DETAIL_PLATFORM_SPEC_HPP
#define NANA_DETAIL_PLATFORM_SPEC_HPP

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <nana/config.hpp>
#include <nana/macros.hpp>
#include <nana/gui/basis.hpp>
#include <nana/refer.hpp>
#include <nana/threads/thread.hpp>
#include <nana/threads/locks.hpp>
#include <nana/paint/image.hpp>
#include <nana/paint/graphics.hpp>
#include <vector>
#include <nana/detail/msg_packet.hpp>
#if defined(NANA_UNICODE)
	#include <X11/Xft/Xft.h>
	#include <iconv.h>
	#include <fstream>
#endif

namespace nana
{
namespace detail
{
	class msg_dispatcher;
#if defined(NANA_UNICODE)
	class conf
	{
	public:
		conf(const char * file);
		bool open(const char* file);
		std::string value(const char* key);
	private:
		std::ifstream ifs_;
	};

	class charset_conv
	{
	public:
		charset_conv(const char* tocode, const char* fromcode);
		~charset_conv();
		std::string charset(const std::string& str) const;
		std::string charset(const char * buf, size_t len) const;
	private:
		iconv_t handle_;
	};
#endif

	struct font_tag
	{
		nana::string name;
		unsigned height;
		unsigned weight;
		bool italic;
		bool underline;
		bool strikeout;
#if defined(NANA_UNICODE)
		XftFont * handle;
#else
		XFontSet handle;
#endif

		struct deleter
		{
			void operator()(const font_tag*) const;
		};
	};

	struct drawable_impl_type
	{
		typedef nana::refer<font_tag*, font_tag::deleter> font_refer_t;

		drawable_impl_type();
		~drawable_impl_type();

		void fgcolor(nana::color_t);
		void bgcolor(nana::color_t);

		Pixmap	pixmap;
		GC	context;
		font_refer_t font;

		struct string_spec
		{
			unsigned tab_length;
			unsigned tab_pixels;
			unsigned whitespace_pixels;
		}string;
#if defined(NANA_UNICODE)
		XftDraw * xftdraw;
		XftColor	xft_fgcolor;
		XftColor	xft_bgcolor;
		const std::string charset(const nana::string& str, const std::string& strcode);
#endif
	private:
		unsigned bgcolor_;
		unsigned fgcolor_;
#if defined(NANA_UNICODE)
		struct conv_tag
		{
			iconv_t handle;
			std::string code;
		}conv_;
#endif
	};

	struct atombase_tag
	{
		Atom wm_protocols;
		//window manager support
		Atom wm_delete_window;
		//ext
		Atom net_wm_state;
		Atom net_wm_state_skip_taskbar;
		Atom net_wm_state_fullscreen;
		Atom net_wm_state_maximized_horz;
		Atom net_wm_state_maximized_vert;
		Atom net_wm_window_type;
		Atom net_wm_window_type_normal;
		Atom net_wm_window_type_utility;
		Atom net_wm_window_type_dialog;
		Atom motif_wm_hints;

		Atom clipboard;
		Atom text;
		Atom text_uri_list;
		Atom utf8_string;
		Atom targets;

		Atom xdnd_aware;
		Atom xdnd_enter;
		Atom xdnd_position;
		Atom xdnd_status;
		Atom xdnd_action_copy;
		Atom xdnd_drop;
		Atom xdnd_selection;
		Atom xdnd_typelist;
		Atom xdnd_finished;
	};

	struct caret_tag;

	class timer_runner;

	class platform_scope_guard
	{
	public:
		platform_scope_guard();
		~platform_scope_guard();
	};

	class platform_spec
	{
		typedef platform_spec self_type;

		struct window_context_t
		{
			nana::gui::native_window_type owner;
			std::vector<nana::gui::native_window_type> * owned;
		};
    public:
        int error_code;
	public:
		typedef drawable_impl_type::font_refer_t font_refer_t;
		typedef void (*timer_proc_type)(unsigned tid);
		typedef void (*event_proc_type)(Display*, msg_packet_tag&);


		platform_spec();
		~platform_spec();

		const font_refer_t& default_native_font() const;
		unsigned font_size_to_height(unsigned) const;
		unsigned font_height_to_size(unsigned) const;
		font_refer_t make_native_font(const nana::char_t* name, unsigned height, unsigned weight, bool italic, bool underline, bool strick_out);

		Display* open_display();
		void close_display();

		void lock_xlib();
		void unlock_xlib();

		Window root_window();
		int screen_depth();
		Visual* screen_visual();

		Colormap& colormap();

		static self_type& instance();
		const atombase_tag & atombase() const;

		void make_owner(nana::gui::native_window_type owner, nana::gui::native_window_type wd);
		nana::gui::native_window_type get_owner(nana::gui::native_window_type) const;
		void remove(nana::gui::native_window_type);

		void write_keystate(const XKeyEvent&);
		void read_keystate(XKeyEvent&);

		XIC	caret_input_context(nana::gui::native_window_type) const;
		void caret_open(nana::gui::native_window_type, unsigned width, unsigned height);
		void caret_close(nana::gui::native_window_type);
		void caret_pos(nana::gui::native_window_type, int x, int y);
		void caret_visible(nana::gui::native_window_type, bool);
		void caret_flash(nana::gui::native_window_type);
		void caret_reinstate(nana::gui::native_window_type);
		void set_error_handler();
		int rev_error_handler();
		void event_register_filter(nana::gui::native_window_type, unsigned eventid);
		//grab
		//register a grab window while capturing it if it is unviewable.
		//when native_interface::show a window that is registered as a grab
		//window, the native_interface grabs the window.
		Window grab(Window);
		void set_timer(int id, size_t interval, void (*timer_proc)(int id));
		void kill_timer(int id);
		void timer_proc(unsigned tid);

		//Message dispatcher
		void msg_insert(nana::gui::native_window_type);
		void msg_set(timer_proc_type, event_proc_type);
		void msg_dispatch(nana::gui::native_window_type modal);

		//X Selections
		void* request_selection(nana::gui::native_window_type requester, Atom type, size_t & bufsize);
		void write_selection(nana::gui::native_window_type owner, Atom type, const void* buf, size_t bufsize);

		//Icon storage
		//@biref: The image object should be kept for a long time till the window is closed,
		//			the image object is release in remove() method.
		const nana::paint::graphics& keep_window_icon(nana::gui::native_window_type, const nana::paint::image&);
	private:
		static int _m_msg_filter(XEvent&, msg_packet_tag&);
		void _m_caret_routine();
	private:
		Display*	display_;
		Colormap	colormap_;
		atombase_tag atombase_;
		font_refer_t def_font_ref_;
		XKeyEvent	key_state_;
		int (*def_X11_error_handler_)(Display*, XErrorEvent*);
		Window grab_;
		nana::threads::token xlib_locker_;
		struct caret_holder_tag
		{
			nana::threads::thread thr;
			std::map<nana::gui::native_window_type, caret_tag*> carets;
		}caret_holder_;

		std::map<nana::gui::native_window_type, window_context_t> wincontext_;
		std::map<nana::gui::native_window_type, nana::paint::graphics> iconbase_;

		struct timer_runner_tag
		{
			timer_runner * runner;
			nana::threads::token token;
			bool delete_declared;
			timer_runner_tag();
		}timer_;

		struct selection_tag
		{
			struct item_t
			{
				Atom	type;
				Window	requestor;
				void*	buffer;
				size_t	bufsize;
				nana::threads::condition cond;
			};

			std::vector<item_t*> items;

			struct content_tag
			{
				std::string * utf8_string;
			}content;
		}selection_;

		struct xdnd_tag
		{
			Atom good_type;
			int timestamp;
			Window wd_src;
			nana::point pos;
		}xdnd_;

		msg_dispatcher * msg_dispatcher_;
	};//end class platform_X11

}//end namespace detail

}//end namespace nana

#endif

