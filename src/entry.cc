/*
 * Peach Finder: Peach Finder
 * Copyright (c) 2009 Ayose Cazorla
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */


#include <string.h>
#include <sys/ioctl.h>
#include "peach.h"

namespace Entry {

    static std::string line_buffer;
    int cursor_position;
    bool all_text_selected;
    bool text_changed;
    bool window_changed;


    void grab_focus()
    {
        wmove(entry_window, 0, mbposition(line_buffer.c_str(), cursor_position));
        wrefresh(entry_window);
    }

    void update_window()
    {
        werase(entry_window);
        mvwaddnstr(entry_window, 0, 0, line_buffer.c_str(), line_buffer.size());

        if(all_text_selected)
            mvwchgat(entry_window, 0, 0, mbposition(line_buffer.c_str(), cursor_position), A_REVERSE, 0, NULL);

        grab_focus();
    }

    void insert_text(const char* new_text)
    {
        if(all_text_selected) {
            line_buffer = new_text;
            cursor_position = strlen(new_text);
            all_text_selected = false;
        } else {
            line_buffer.insert(cursor_position, new_text);
            cursor_position += strlen(new_text);
        }

        text_changed = true;
    }

    void move_cursor(int offset)
    {
        const char *lb = line_buffer.c_str();
        int current_char = mbposition(lb, cursor_position);
        int next_char = current_char + offset;

        all_text_selected = false;
        window_changed = true;

        if(next_char < 1) {
            cursor_position = 0;
            return;
        }

        cursor_position = 0;
        int lb_size = strlen(lb);
        while(next_char-- > 0) {
            cursor_position += mblen(lb + cursor_position, strlen(lb) - cursor_position);
            if(cursor_position > lb_size) {
                cursor_position = lb_size;
                return;
            }
        }
    }

    void delete_character()
    {
        if(cursor_position < 0 || cursor_position > (int)line_buffer.size())
            return;

        const char *to_delete = line_buffer.c_str() + cursor_position;
        line_buffer.erase(cursor_position, mblen(to_delete, strlen(to_delete)));
        text_changed = true;
    }

    void kill_to_begin()
    {
        text_changed = true;
        line_buffer.erase(0, cursor_position);
        cursor_position = 0;
    }

#define prepare_move_cursor_until() \
    int limit = line_buffer.size(); \
    do { \
        if(cursor_position >= limit && limit > 1) \
            cursor_position = limit - 1; \
    } while(0)

#define move_cursor_until(steps, condition) \
    do { \
        while(cursor_position >= 0 && cursor_position < limit && (condition)) \
            cursor_position += steps; \
    } while(0)

#define cursor_on_blank() (isblank(line_buffer[cursor_position]))

    void jump_to_prev_word()
    {
        prepare_move_cursor_until();

        cursor_position--;
        move_cursor_until(-1, cursor_on_blank());
        move_cursor_until(-1, !cursor_on_blank());
        cursor_position++;

        /* Fix ranges */
        move_cursor(0);
    }

    void jump_to_next_word()
    {
        prepare_move_cursor_until();
        cursor_position++;

        move_cursor_until(1, !cursor_on_blank());
        move_cursor_until(1, cursor_on_blank());

        /* Fix ranges */
        move_cursor(0);
    }

    void kill_word()
    {
        int end = cursor_position;
        jump_to_prev_word();
        line_buffer.erase(cursor_position, end - cursor_position);

        text_changed = true;
    }

    void loop()
    {
        while(true) {

            if(text_changed || window_changed) {
                ssize_t n;
                if (ioctl (fileno(stdin), FIONREAD, (char *)&n) != 0)
                    n = 0;

                if(n == 0) {
                    if(text_changed)
                        Files::set_filter(line_buffer.c_str());

                    update_window();

                    text_changed = false;
                    window_changed = false;
                }
            }

            int c = wgetch(entry_window);

            switch(c) {
                case KEY_RESIZE:
                    UI::update_term_size();
                    break;

                case KEY_UP:
                    Files::change_selection(-1);
                    break;

                case KEY_DOWN:
                    Files::change_selection(1);
                    break;

                case KEY_PPAGE:
                    Files::change_selection(Files::window_height() / -3);
                    break;

                case KEY_NPAGE:
                    Files::change_selection(Files::window_height() / 3);
                    break;

                case '':
                case KEY_HOME:
                    cursor_position = 0;
                    all_text_selected = false;
                    window_changed = true;
                    break;

                case '':
                case KEY_END:
                    cursor_position = line_buffer.size();
                    all_text_selected = false;
                    window_changed = true;
                    break;

                case '':
                case KEY_RIGHT:
                    move_cursor(1);
                    break;

                case '':
                case KEY_LEFT:
                    move_cursor(-1);
                    break;

                case '':
                case KEY_DC:
                    if(all_text_selected) {
                        insert_text("");
                    } else {
                        delete_character();
                    }
                    break;

                case '':
                case KEY_BACKSPACE:
                    if(all_text_selected) {
                        insert_text("");
                    } else {
                        move_cursor(-1);
                        delete_character();
                    }
                    break;

                case '':
                case KEY_ENTER:
                case '\n':
                case '\r':
                    Files::do_action();
                    break;

                case '':
                    kill_to_begin();
                    break;

                case '':
                    kill_word();
                    break;

                case '':
                    switch(wgetch(entry_window)) {
                        case 'b':
                            jump_to_prev_word();
                            break;

                        case 'f':
                            jump_to_next_word();
                            break;
                    }

                default:
                    if(c >= 32) {
                        char new_text[2] = { c, 0 };
                        insert_text(new_text);
                    }
            }
        }
    }

    void init(const char* initial_text)
    {
        text_changed = true;
        window_changed = true;

        if(initial_text) {
            line_buffer = initial_text;
            cursor_position = strlen(initial_text);
            all_text_selected = true;
        } else {
            line_buffer = "";
            cursor_position = 0;
            all_text_selected = false;
        }
    }


    const char* get_current_text() {
        return line_buffer.c_str();
    }

}
