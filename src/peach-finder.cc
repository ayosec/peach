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


#include <locale.h>
#include "peach.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void bad_command_line(int argc, char *argv[])
{
    fprintf(stderr, "Usage: %s [-x exclude_list] [-c command] [-s score_file] [-m mapping command] [-f initial filter] paths\n",
            argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "");

    const char* initial_words = NULL;

    Files::init();

    /* Parse options */
    int opt;
    while ((opt = getopt(argc, argv, "x:c:s:m:f:")) != -1) {
        switch(opt) {
        case 'x':
            Files::set_exclude_list(optarg);
            break;

        case 'c':
            Files::set_action(optarg);
            break;

        case 's':
            Files::set_score_file(optarg);
            break;

        case 'm':
            Files::set_map_command(optarg);
            break;

        case 'f':
            initial_words = strdupa(optarg);
            break;

        default:
            bad_command_line(argc, argv);
        }
    }

    if(optind >= argc)
        bad_command_line(argc, argv);

    while(optind < argc) {
        Files::load_dir(argv[optind]);
        optind++;
    }

    if(!Files::ready())
        return 0;

    UI::init();
    Entry::init(initial_words);

    UI::update_windows();
    Entry::loop();

    return 0;
}
