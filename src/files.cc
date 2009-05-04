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

#include "peach.h"
#include <list>
#include <string.h>
#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fnmatch.h>
#include <time.h>



namespace Files {

    /* En esta estructura guardamos cada fichero que coincida con las
       palabras buscadas */

    namespace Filter {
        typedef char filename[PATH_MAX];

        struct Match {
            filename path;
            filename name;
        };

        struct AvailableFile {
            filename path;
            filename name;
            int score;
        };
    }

    struct Score {
        bool active;
        std::string file;

        struct HistoryEntry {
            char file[PATH_MAX];
            int timestamp;
        };

        class HistoryList : public std::vector<HistoryEntry> {} history;
    } score;


    class FileList : public std::list<Filter::AvailableFile>
    {
    public:

        void new_file(const std::string& abs_path, const std::string& new_name)
        {
            Filter::AvailableFile af;
            strncpy(af.path, abs_path.c_str(), PATH_MAX);
            strncpy(af.name, new_name.c_str(), PATH_MAX);
            af.score = 0;

            for(Score::HistoryList::iterator i = score.history.begin(); i != score.history.end(); i++) {
                if(strcmp(i->file, af.path) == 0)
                    af.score += i->timestamp;
            }

            push_back(af);
        }

        struct _cmp_files
        {
            inline bool operator()(const Filter::AvailableFile& a, const Filter::AvailableFile& b) const
            {
                return a.score > b.score;
            }
        };

        void sort() {
            std::list<Filter::AvailableFile>::sort(_cmp_files());
        }
    };

    FileList files;
    std::vector<Filter::Match> matched_files;

    vector_string exclude_list;
    vector_string filter_words;

    std::string command_action;
    std::string command_mapping;

    struct Selection {
        int index;
        int scroll;
    } selection;

    int window_height()
    {
        return getmaxy(files_window) - 1;
    }

    void update_window()
    {
        werase(files_window);

        /* Metadata */
        mvwprintw(files_window, 0, 30, "%d of %d files", matched_files.size(), files.size());

        if(filter_words.size() > 0) {
            wprintw(files_window, " with");
            vector_iterator(filter_words, i) {
                wprintw(files_window, " \"%s\"", i->c_str());
            }
        }

        unsigned int limit = window_height();
        for(unsigned int row = 0; row < limit; row++) {
            int file_index = row + selection.scroll;
            if(row >= matched_files.size())
                break;

            mvwaddnstr(files_window, row + 1, 0, matched_files[file_index].name, strlen(matched_files[file_index].name));

            if(selection.index == (int)file_index)
                mvwchgat(files_window, row + 1, 0, -1, A_REVERSE, 0, NULL);

        }

        wrefresh(files_window);
        Entry::grab_focus();
    }

    void select_index(int index)
    {
        int height = window_height();
        int window_start = selection.scroll;
        int window_end = selection.scroll + height;

        selection.index = index;
        if((int)matched_files.size() <= height)
            selection.scroll = 0;
        else if(index < window_start)
            selection.scroll = index;
        else if(index > window_end)
            selection.scroll = index - window_height() + 1;

        update_window();
    }

    void change_selection(int offset)
    {
        int s = selection.index + offset;
        if(s >= (int)matched_files.size())
            s = matched_files.size() - 1;
        if(s < 0)
            s = 0;

        select_index(s);
    }

    static void find_files()
    {
        matched_files.clear();

        bool has_every_word;
        for(FileList::iterator file = files.begin(); file != files.end(); file++) {
            has_every_word = true;
            vector_iterator(filter_words, word) {
                if(!strcasestr(file->name, word->c_str())) {
                    has_every_word = false;
                    break;
                }
            }

            if(has_every_word) {
                Filter::Match m;
                strcpy(m.path, file->path);
                strcpy(m.name, file->name);
                matched_files.push_back(m);
            }
        }
    }

    void set_map_command(const char* cmd)
    {
        command_mapping = cmd;
    }

    void set_filter(const char* filter)
    {
        /* Separamos en palabras */
        filter_words = split(filter, " \t\n");

        if(filter_words.size() > 0)
            find_files();
        else
            matched_files.clear();

        change_selection(0);
    }

    bool ready()
    {
        if(files.size() == 0)
            return false;

        /* Sort found files */
        files.sort();

        /* Map files */
        if(command_mapping.size() > 0) {
            FILE *fd_write, *fd_read;
            int pid;

            if((pid = popen2(&fd_read, &fd_write, command_mapping.c_str())) > 0) {
                for(FileList::iterator file = files.begin(); file != files.end(); file++)
                    fprintf(fd_write, "%s\n", file->name);
                fclose(fd_write);

                int count = 0;
                for(FileList::iterator file = files.begin(); file != files.end(); file++, count++) {
                    if(!fgets(file->name, sizeof(file->name), fd_read)) {
                        fprintf(stderr, "Error in map command: %d lines read. Should be %d\n", count, (int)files.size());
                        exit(2);
                    }
                }
                fclose(fd_read);

                int status;
                waitpid(pid, &status, 0);
                if(status != 0) {
                    fprintf(stderr, "Error in map command: Exit status: %d\n", status);
                    exit(2);
                }
            }
        }

        return true;
    }

    static void load_files(std::string& path, std::string &name)
    {
        struct stat stat_file;
        dirent *ent;
        DIR *dir;

        dir = opendir(path.c_str());
        if(!dir) {
            perror(path.c_str());
            return;
        }

        while((ent = readdir(dir))) {

            /* Ignore . and .. */
            if(strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue;

            /* Check if the file has to be excluded */
            bool ignore_file = false;
            for(unsigned int i = 0; i < exclude_list.size(); i++) {
                if(fnmatch(exclude_list[i].c_str(), ent->d_name, 0) == 0) {
                    ignore_file = true;
                    break;
                }
            }

            if(ignore_file)
                continue;

            std::string abs_path = path + "/" + ent->d_name;
            if(lstat(abs_path.c_str(), &stat_file) != 0)
                continue;

            std::string new_name = name + "/" + ent->d_name;

            if(S_ISDIR(stat_file.st_mode)) {
                load_files(abs_path, new_name);
            } else if(S_ISREG(stat_file.st_mode)) {
                files.new_file(abs_path, new_name);
            }

        }
        closedir(dir);
    }

    void init()
    {
        selection.index = 0;
        selection.scroll = 0;

        score.active = false;
    }

    void load_dir(const char *base_path)
    {
        std::string bp;
        if(base_path[0] != '/') {
            char *wd = get_current_dir_name();
            bp = wd;
            free(wd);

            if(strcmp(base_path, ".") != 0) {
                bp += "/";
                bp += base_path;
            }

        } else {
            bp = base_path;
        }

        std::string name = "";
        load_files(bp, name);
    }

    void set_score_file(const char* filename)
    {
        score.active = true;
        score.file = filename;

        /* Load history data */
        FILE *f = fopen(filename, "r");
        if(f) {
            char line[PATH_MAX + 20];
            int min_value = (int)time(NULL);
            while(!feof(f)) {
                char *separator;
                Score::HistoryEntry he;

                if(fgets(line, sizeof(line), f) && (separator = index(line, ':'))) {
                    *separator = '\0';
                    he.timestamp = atoi(line);

                    if(he.timestamp < min_value)
                        min_value = he.timestamp;

                    separator++;
                    if(*separator && separator[strlen(separator) - 1] == '\n')
                        separator[strlen(separator) - 1] = '\0';

                    strncpy(he.file, separator, PATH_MAX);
                    score.history.push_back(he);
                }
            }
            fclose(f);

            /* Adjust timestamp */
            for(unsigned int i = 0; i < score.history.size(); i++)
                score.history[i].timestamp -= min_value;

        } else {
            perror(filename);
        }
    }

    void set_exclude_list(const char* list)
    {
        exclude_list = split(list, ", \t:");
    }

    void set_action(const char* action)
    {
        command_action = action;
    }

    void do_action()
    {
        if(selection.index < 0 || selection.index >= (int)matched_files.size())
            return;

        const char *selected_path = matched_files[selection.index].path;

        if(score.active) {
            FILE* f = fopen(score.file.c_str(), "a");
            if(f) {
                fprintf(f, "%d:%s\n", (int)time(NULL), selected_path);
                fclose(f);
            } else {
                perror(score.file.c_str());
            }
        }

        if(command_action.size() > 0) {
            std::string file = "'";
            file += selected_path;
            file += "'";

            std::string::size_type pos = command_action.find(ACTION_MARK);
            if(pos == std::string::npos)
                command_action += " " + file;
            else
                command_action.replace(pos, strlen(ACTION_MARK), file);

            endwin();
            execl("/bin/sh", "sh", "-c", command_action.c_str(), NULL);
            perror("/bin/sh");
            exit(1);
        } else {
            endwin();
            fprintf(stderr, "OPEN %s\n", selected_path);
            fprintf(stderr, "FILTER %s\n", Entry::get_current_text());
            exit(0);
        }
    }

}
