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

#include <string>
#include <vector>

/* Utils */
int mbposition(const char* buffer, int limit);
void debug(const char *s, ...);
int popen2(FILE**, FILE**, const char*);

typedef std::vector<std::string> vector_string;
vector_string split(const char* str, const char* separator);

#define vector_iterator(iter_object, iterator_name) for(vector_string::iterator iterator_name = iter_object.begin(); iterator_name != iter_object.end(); iterator_name++)


namespace Entry {
    void init(const char* initial_text);
    void update_window();
    void grab_focus();
    void loop();
    const char* get_current_text();
}

namespace UI {
    void init();
    void build_ui();
    void update_windows();
    void update_term_size();
}

namespace Files {
    void update_window();
    int window_height();
    void set_filter(const char*);
    void load_dir(const char*);
    void change_selection(int);
    bool ready();
    void do_action();
    void init();

    void set_score_file(const char*);
    void set_exclude_list(const char*);
    void set_action(const char*);
    void set_map_command(const char*);
}

#include <stdlib.h>
#include <ncursesw/ncurses.h>

extern WINDOW *entry_window;
extern WINDOW *files_window;

#define ACTION_MARK "%%"
