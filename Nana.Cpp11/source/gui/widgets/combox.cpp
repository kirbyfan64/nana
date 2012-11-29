/*
 *	A Combox Implementation
 *	Copyright(C) 2003-2012 Jinhao(cnjinhao@hotmail.com)
 *
 *	Distributed under the Nana Software License, Version 1.0.
 *	(See accompanying file LICENSE_1_0.txt or copy at
 *	http://nanapro.sourceforge.net/LICENSE_1_0.txt)
 *
 *	@file: nana/gui/widgets/combox.cpp
 */

#include <nana/gui/wvl.hpp>
#include <nana/gui/widgets/combox.hpp>
#include <nana/paint/gadget.hpp>
#include <nana/system/dataexch.hpp>
#include <nana/gui/widgets/float_listbox.hpp>
#include <nana/gui/widgets/skeletons/text_editor.hpp>

namespace nana{ namespace gui{
	namespace drawerbase
	{
		namespace combox
		{
			class drawer_impl
			{
			public:
				typedef nana::paint::graphics & graph_reference;
				typedef nana::gui::widget	& widget_reference;

				enum{WhereUnknown, WhereText, WherePushButton};
				enum{StateNone, StateMouseOver, StatePress};

				mutable extra_events ext_event;

				typedef drawerbase::float_listbox::module_def::item_type item_type;

				drawer_impl()
					:	widget_(nullptr), graph_(nullptr),
						item_renderer_(nullptr), image_enabled_(false), image_pixels_(16), editor_(nullptr)
				{
					state_.focused = false;
					state_.state = StateNone;
					state_.pointer_where = WhereUnknown;
					state_.lister = nullptr;
				}

				~drawer_impl()
				{
					clear();
				}

				void renderer(drawerbase::float_listbox::item_renderer* ir)
				{
					item_renderer_ = ir;
				}

				void bind(widget_reference wd)
				{
					widget_ = &wd;
				}

				void attached(graph_reference graph)
				{
					editor_ = new widgets::skeletons::text_editor(widget_->handle(), graph);
					editor_->border_renderer(nana::make_fun(*this, &drawer_impl::draw_border));
					editor_->multi_lines(false);
					this->editable(false);

					graph_ = &graph;
				}

				void detached()
				{
					delete editor_;
					editor_ = nullptr;
					graph_ = nullptr;
				}

				void insert(const nana::string& text)
				{
					module_.items.push_back(text);
					anyobj_.push_back(nullptr);
				}

				int get_where() const
				{
					return state_.pointer_where;
				}

				nana::any * anyobj(std::size_t i, bool allocate_if_empty) const
				{
					if(i < anyobj_.size())
					{
						nana::any * p = anyobj_[i];
						if(allocate_if_empty && (nullptr == p))
							anyobj_[i] = p = new nana::any;
						return p;
					}
					return nullptr;
				}

				void text_area(unsigned width, unsigned height)
				{
					nana::rectangle r(2, 2, width > 19 ? width - 19 : 0, height > 4 ? height - 4 : 0);
					if(image_enabled_)
					{
						unsigned place = image_pixels_ + 2;
						r.x += place;
						if(r.width > place)	r.width -= place;
					}
					editor_->text_area(r);
				}

				widgets::skeletons::text_editor * editor() const
				{
					return editor_;
				}

				widget* widget_ptr() const
				{
					return widget_;
				}

				void clear()
				{
					for(auto p : anyobj_)
						delete p;
					anyobj_.clear();
					module_.items.clear();
				}

				void editable(bool enb)
				{
					if(editor_)
					{
						editor_->editable(enb);

						if(enb)
							editor_->ext_renderer().background = nullptr;
						else
							editor_->ext_renderer().background = std::bind(&drawer_impl::_m_draw_background, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

						editor_->enable_background(enb);
						editor_->enable_background_counterpart(!enb);
						API::refresh_window(this->widget_->handle());
					}
				}

				bool editable() const
				{
					return (editor_ ? editor_->attr().editable : false);
				}

				bool calc_where(graph_reference graph, int x, int y)
				{
					int new_where = WhereUnknown;

					if(1 < x && x < static_cast<int>(graph.width()) - 2 && 1 < y && y < static_cast<int>(graph.height()) - 2)
					{
						if((editor_->attr().editable == false) || (static_cast<int>(graph.width()) - 22 <= x))
							new_where = WherePushButton;
						else
							new_where = WhereText;
					}

					if(new_where != state_.pointer_where)
					{
						state_.pointer_where = new_where;
						return true;
					}
					return false;
				}

				void set_mouse_over(bool mo)
				{
					state_.state = mo ? StateMouseOver : StateNone;
					state_.pointer_where = WhereUnknown;
				}

				void set_mouse_press(bool mp)
				{
					state_.state = (mp ? StatePress : StateMouseOver);
				}

				void set_focused(bool f)
				{
					if(editor_)
					{
						state_.focused = f;
						if(editor_->attr().editable)
							editor_->select(f);
					}
				}

				bool has_lister() const
				{
					return (state_.lister != nullptr);
				}

				void open_lister()
				{
					if(nullptr == state_.lister)
					{
						state_.lister = &form_loader<nana::gui::float_listbox>()(widget_->handle(), nana::rectangle(0, widget_->size().height, widget_->size().width, 10));
						state_.lister->renderer(item_renderer_);
						state_.lister->set_module(module_, image_pixels_);
						state_.item_index_before_selection = module_.index;
						//The lister window closes by itself. I just take care about the destroy event.
						//The event should be destroy rather than unload. Because the unload event is invoked while
						//the lister is not closed, if popuping a message box, the lister will cover the message box.
						state_.lister->make_event<events::destroy>(*this, &drawer_impl::_m_lister_close_sig);
					}
				}

				void scroll_items(bool upwards)
				{
					if(state_.lister)
						state_.lister->scroll_items(upwards);
				}

				void move_items(bool upwards, bool recycle)
				{
					if(0 == state_.lister)
					{
						std::size_t orig_i = module_.index;
						if(upwards)
						{
							if(module_.index)
								--(module_.index);
							else if(recycle)
								module_.index = static_cast<unsigned>(module_.items.size() - 1);
						}
						else
						{
							if(module_.index != module_.items.size() - 1)
								++(module_.index);
							else if(recycle)
								module_.index = 0;
						}

						if(orig_i != module_.index)
							option(module_.index, false);
					}
					else
						state_.lister->move_items(upwards, recycle);
				}

				void draw()
				{
					bool enb = widget_->enabled();
					if(editor_)
					{
						nana::size sz = widget_->size();
						text_area(sz.width, sz.height);
						editor_->redraw(state_.focused);
					}
					_m_draw_push_button(enb);
					_m_draw_image();
				}

				void draw_border(graph_reference graph)
				{
					graph.rectangle((state_.focused ? 0x0595E2 : 0x999A9E), false);
					graph.rectangle(nana::rectangle(graph.size()).pare_off(1), 0xFFFFFF, false);
				}

				std::size_t the_number_of_options() const
				{
					return module_.items.size();
				}

				std::size_t option() const
				{
					return module_.index;
				}

				void option(std::size_t index, bool ignore_condition)
				{
					if(index < module_.items.size())
					{
						std::size_t old_index = module_.index;
						module_.index = index;
						if(widget_)
						{
							//Test if the current item or text is different from selected.
							if(ignore_condition || (old_index != index) || (module_.items[index].text != widget_->caption()))
							{
								nana::point pos = API::cursor_position();
								API::calc_window_point(widget_->handle(), pos);
								if(this->calc_where(*graph_, pos.x, pos.y))
									state_.state = StateNone;

								editor_->text(module_.items[index].text);
								bool enb = widget_->enabled();
								_m_draw_push_button(enb);
								_m_draw_image();

								//Yes, it's safe to static_cast here!
								ext_event.selected(*static_cast<nana::gui::combox*>(widget_));
							}
						}
					}
				}

				const item_type & at(std::size_t i) const
				{
					return module_.items.at(i);
				}

				void text(const nana::string& str)
				{
					if(editor_)
						editor_->text(str);
				}

				void image(std::size_t i, const nana::paint::image& img)
				{
					if(i < module_.items.size())
					{
						module_.items[i].img = img;
						if((false == image_enabled_) && img)
						{
							image_enabled_ = true;
							draw();
						}
					}
				}

				bool image_pixels(unsigned px)
				{
					if(image_pixels_ != px)
					{
						image_pixels_ = px;
						return true;
					}
					return false;
				}
			private:
				void _m_lister_close_sig()
				{
					state_.lister = 0;	//The lister closes by itself.
					if(module_.index != module_.npos && module_.index != state_.item_index_before_selection)
					{
						option(module_.index, true);
						API::update_window(*widget_);
					}
				}

				void _m_draw_background(graph_reference graph, const nana::rectangle&, nana::color_t)
				{
					nana::rectangle r(graph.size());
					unsigned color_start = color::button_face_shadow_start, color_end = gui::color::button_face_shadow_end;
					if(state_.state == StatePress)
					{
						r.pare_off(2);
						color_start = gui::color::button_face_shadow_end;
						color_end = gui::color::button_face_shadow_start;
					}
					else
						r.pare_off(1);

					graph.shadow_rectangle(r, color_start, color_end, true);
				}

				void _m_draw_push_button(bool enabled)
				{
					if(0 == graph_) return;

					int left = graph_->width() - 17;
					int right = left + 16;
					int top = 1;
					int bottom = graph_->height() - 2;
					int mid = top + (bottom - top) * 5 / 18;

					nana::color_t topcol, topcol_ln, botcol, botcol_ln;
					if(this->has_lister() || (state_.state == StatePress && state_.pointer_where == WherePushButton))
					{
						topcol_ln = nana::paint::graphics::mix(0x3F476C, 0x0, 0.8);
						botcol_ln = nana::paint::graphics::mix(0x031141, 0x0, 0.8);
						topcol = nana::paint::graphics::mix(0x3F83B4, 0x0, 0.8);
						botcol = nana::paint::graphics::mix(0x0C4A95, 0x0, 0.8);					
					}
					else if(state_.state == this->StateMouseOver)
					{
						topcol_ln = nana::paint::graphics::mix(0x3F476C, 0xFFFFFF, 0.9);
						botcol_ln = nana::paint::graphics::mix(0x031141, 0xFFFFFF, 0.9);
						topcol = nana::paint::graphics::mix(0x3F83B4, 0xFFFFFF, 0.9);
						botcol = nana::paint::graphics::mix(0x0C4A95, 0xFFFFFF, 0.9);
					}
					else
					{
						topcol_ln = 0x3F476C;
						botcol_ln = 0x031141;
						topcol = 0x3F83B4;
						botcol = 0x0C4A95;
					}

					graph_->line(left, top, left, mid, topcol_ln);
					graph_->line(right - 1, top, right - 1, mid, topcol_ln);

					graph_->line(left, mid + 1, left, bottom, botcol_ln);
					graph_->line(right - 1, mid + 1, right - 1, bottom, botcol_ln);

					graph_->rectangle(left + 1, top, right - left - 2, mid - top + 1, topcol, true);
					graph_->rectangle(left + 1, mid + 1, right - left - 2, bottom - mid, botcol, true);

					nana::paint::gadget::arrow_16_pixels(*graph_, left, top + ((bottom - top) / 2) - 7, (enabled ? 0xFFFFFF : nana::gui::color::dark_border), 1, nana::paint::gadget::directions::to_south);
				}

				void _m_draw_image()
				{
					if(module_.index != module_.npos)
					{
						nana::paint::image img = module_.items[module_.index].img;
						if(img)
						{
							unsigned vpix = editor_->line_height();
							nana::size imgsz = img.size();
							if(imgsz.width > image_pixels_)
							{
								unsigned new_h = image_pixels_ * imgsz.height / imgsz.width;
								if(new_h > vpix)
								{
									imgsz.width = vpix * imgsz.width / imgsz.height;
									imgsz.height = vpix;
								}
								else
								{
									imgsz.width = image_pixels_;
									imgsz.height = new_h;
								}
							}
							else if(imgsz.height > vpix)
							{
								unsigned new_w = vpix * imgsz.width / imgsz.height;
								if(new_w > image_pixels_)
								{
									imgsz.height = image_pixels_ * imgsz.height / imgsz.width;
									imgsz.width = image_pixels_;
								}
								else
								{
									imgsz.height = vpix;
									imgsz.width = new_w;
								}
							}

							nana::point pos(2, 2);
							pos.x += (image_pixels_ - imgsz.width) / 2;
							pos.y += (vpix - imgsz.height) / 2;

							img.stretch(img.size(), *graph_, nana::rectangle(pos, imgsz));
						}
					}
				}
			private:
				nana::gui::float_listbox::module_type module_;
				mutable std::vector<nana::any*>	anyobj_;
				nana::gui::widget * widget_;
				nana::paint::graphics * graph_;

				drawerbase::float_listbox::item_renderer* item_renderer_;

				bool image_enabled_;
				unsigned image_pixels_;
				widgets::skeletons::text_editor * editor_;

				struct state_type
				{
					bool	focused;
					int		state;
					int		pointer_where;

					nana::gui::float_listbox * lister;
					std::size_t	item_index_before_selection;
				}state_;
			};


			//class trigger
				trigger::trigger()
					: drawer_(new drawer_impl)
				{}

				trigger::~trigger()
				{
					delete drawer_;
					drawer_ = 0;
				}

				drawer_impl& trigger::get_drawer_impl()
				{
					return *drawer_;
				}

				const drawer_impl& trigger::get_drawer_impl() const
				{
					return *drawer_;
				}

				void trigger::bind_window(widget_reference wd)
				{
					drawer_->bind(wd);
					wd.background(0xFFFFFF);
				}

				void trigger::attached(graph_reference graph)
				{
					drawer_->attached(graph);

					window wd = drawer_->widget_ptr()->handle();
					using namespace API::dev;
					make_drawer_event<events::mouse_down>(wd);
					make_drawer_event<events::mouse_up>(wd);
					make_drawer_event<events::mouse_move>(wd);
					make_drawer_event<events::mouse_enter>(wd);
					make_drawer_event<events::mouse_leave>(wd);
					make_drawer_event<events::focus>(wd);
					make_drawer_event<events::mouse_wheel>(wd);
					make_drawer_event<events::key_down>(wd);
					make_drawer_event<events::key_char>(wd);

					API::effects_edge_nimbus(wd, effects::edge_nimbus::active);
					API::effects_edge_nimbus(wd, effects::edge_nimbus::over);
				}

				void trigger::detached()
				{
					drawer_->detached();
					API::dev::umake_drawer_event(drawer_->widget_ptr()->handle());
				}

				void trigger::refresh(graph_reference)
				{
					drawer_->draw();
				}

				void trigger::focus(graph_reference, const eventinfo& ei)
				{
					drawer_->set_focused(ei.focus.getting);
					if(drawer_->widget_ptr()->enabled())
					{
						drawer_->draw();
						drawer_->editor()->reset_caret();
						API::lazy_refresh();
					}
				}

				void trigger::mouse_enter(graph_reference, const eventinfo&)
				{
					drawer_->set_mouse_over(true);
					if(drawer_->widget_ptr()->enabled())
					{
						drawer_->draw();
						API::lazy_refresh();
					}
				}

				void trigger::mouse_leave(graph_reference, const eventinfo&)
				{
					drawer_->set_mouse_over(false);
					drawer_->editor()->mouse_enter(false);
					if(drawer_->widget_ptr()->enabled())
					{
						drawer_->draw();
						API::lazy_refresh();
					}
				}

				void trigger::mouse_down(graph_reference graph, const eventinfo& ei)
				{
					drawer_->set_mouse_press(true);
					if(drawer_->widget_ptr()->enabled())
					{
						widgets::skeletons::text_editor * editor = drawer_->editor();

						if(false == editor->mouse_down(ei.mouse.left_button, ei.mouse.x, ei.mouse.y))
							if(drawer_->WherePushButton == drawer_->get_where())
								drawer_->open_lister();

						drawer_->draw();
						if(editor->attr().editable)
							editor->reset_caret();

						API::lazy_refresh();
					}
				}

				void trigger::mouse_up(graph_reference graph, const eventinfo& ei)
				{
					if(drawer_->widget_ptr()->enabled())
					{
						if(false == drawer_->has_lister())
						{
							drawer_->editor()->mouse_up(ei.mouse.left_button, ei.mouse.x, ei.mouse.y);
							drawer_->set_mouse_press(false);
							drawer_->draw();
							API::lazy_refresh();
						}
					}
				}

				void trigger::mouse_move(graph_reference graph, const eventinfo& ei)
				{
					if(drawer_->widget_ptr()->enabled())
					{
						bool redraw = drawer_->calc_where(graph, ei.mouse.x, ei.mouse.y);
						redraw |= drawer_->editor()->mouse_move(ei.mouse.left_button, ei.mouse.x, ei.mouse.y);

						if(redraw)
						{
							drawer_->draw();
							drawer_->editor()->reset_caret();
							API::lazy_refresh();
						}
					}
				}

				void trigger::mouse_wheel(graph_reference graph, const eventinfo& ei)
				{
					if(drawer_->widget_ptr()->enabled())
					{
						if(drawer_->has_lister())
							drawer_->scroll_items(ei.wheel.upwards);
						else
							drawer_->move_items(ei.wheel.upwards, false);
					}
				}

				void trigger::key_down(graph_reference, const eventinfo& ei)
				{
					if(drawer_->widget_ptr()->enabled())
					{
						using namespace nana::gui;
						switch(ei.keyboard.key)
						{
						case keyboard::up:
						case keyboard::left:
							drawer_->move_items(true, true);
							break;
						case keyboard::down:
						case keyboard::right:
							drawer_->move_items(false, true);
							break;
						}
					}
				}

				void trigger::key_char(graph_reference graph, const eventinfo& ei)
				{
					widgets::skeletons::text_editor * editor = drawer_->editor();
					if(drawer_->widget_ptr()->enabled() && editor->attr().editable)
					{
						switch(ei.keyboard.key)
						{
						case '\b':
							editor->backspace();	break;
						case '\n': case '\r':
							editor->enter();	break;
						case keyboard::cancel:
							editor->copy();	break;
						case keyboard::sync:
							editor->paste();	break;
						case keyboard::tab:
							editor->put(keyboard::tab); break;
						default:
							if(ei.keyboard.key >= 0xFF || (32 <= ei.keyboard.key && ei.keyboard.key <= 126))
								editor->put(ei.keyboard.key);
							else if(sizeof(nana::char_t) == sizeof(char))
							{	//Non-Unicode Version for Non-English characters
								if(ei.keyboard.key & (1<<(sizeof(nana::char_t)*8 - 1)))
									editor->put(ei.keyboard.key);
							}
						}
						editor->reset_caret();
						API::lazy_refresh();
					}
					else
					{
						switch(ei.keyboard.key)
						{
						case keyboard::cancel:
							editor->copy();	break;
						case keyboard::sync:
							editor->paste();	break;
						}
					}
				}
			//end class trigger
		}
	}//end namespace drawerbase

	//class combox
		combox::combox(){}

		combox::combox(window wd, bool visible)
		{
			this->create(wd, rectangle(), visible);
		}

		combox::combox(window wd, const nana::rectangle& r, bool visible)
		{
			this->create(wd, r, visible);
		}

		void combox::clear()
		{
			nana::gui::internal_scope_guard sg;
			get_drawer_trigger().get_drawer_impl().clear();
			API::refresh_window(this->handle());
		}

		void combox::editable(bool eb)
		{
			get_drawer_trigger().get_drawer_impl().editable(eb);
		}

		bool combox::editable() const
		{
			return get_drawer_trigger().get_drawer_impl().editable();
		}

		combox& combox::push_back(const nana::string& text)
		{
			get_drawer_trigger().get_drawer_impl().insert(text);
			return *this;
		}

		std::size_t combox::the_number_of_options() const
		{
			return get_drawer_trigger().get_drawer_impl().the_number_of_options();
		}

		std::size_t combox::option() const
		{
			return get_drawer_trigger().get_drawer_impl().option();
		}

		void combox::option(std::size_t i)
		{
			get_drawer_trigger().get_drawer_impl().option(i, false);
		}

		nana::string combox::text(std::size_t i) const
		{
			return get_drawer_trigger().get_drawer_impl().at(i).text;
		}

		combox::ext_event_type& combox::ext_event() const
		{
			return get_drawer_trigger().get_drawer_impl().ext_event;
		}

		void combox::renderer(combox::item_renderer* ir)
		{
			get_drawer_trigger().get_drawer_impl().renderer(ir);
		}

		void combox::image(std::size_t i, const nana::paint::image& img)
		{
			if(empty()) return;

			auto & impl = get_drawer_trigger().get_drawer_impl();
			impl.image(i, img);
			if(i == impl.option())
				API::refresh_window(*this);
		}

		nana::paint::image combox::image(std::size_t i) const
		{
			return get_drawer_trigger().get_drawer_impl().at(i).img;
		}

		void combox::image_pixels(unsigned px)
		{
			if(get_drawer_trigger().get_drawer_impl().image_pixels(px))
				API::refresh_window(*this);
		}

		void combox::_m_caption(const nana::string& str)
		{
			internal_scope_guard isg;
			get_drawer_trigger().get_drawer_impl().text(str);
			API::refresh_window(this->handle());
		}

		nana::any * combox::_m_anyobj(std::size_t i, bool allocate_if_empty) const
		{
			return get_drawer_trigger().get_drawer_impl().anyobj(i, allocate_if_empty);
		}
	//end class combox
}//end namespace gui
}
