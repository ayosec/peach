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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include "peach.h"

int mbposition(const char* buffer, int limit)
{
    int p = 0, l = 0, r;
    while(limit > l) {
        r = mblen(buffer + l, limit);
        if(r < 1) return limit;
        l += r;
        p++;
    }

    return p;
}


void debug(const char *s, ...)
{
    va_list a;
    va_start(a, s);
    FILE *f = fopen("/tmp/peach-debug", "a");
    vfprintf(f, s, a);
    fclose(f);
}


vector_string split(const char* str, const char* separator)
{
    vector_string parts;

    const char *current = str;
    const char *start;

    while(*current) {

        /* Find the first non-blank */
        while(index(separator, *current) && *current)
            current++;

        /* The next blank */
        start = current;
        while(!index(separator, *current) && *current)
            current++;

        if(*start) {
            std::string n(start, current - start);
            parts.push_back(n);
        }
    }

    return parts;
}


int popen2(FILE** fd_read, FILE** fd_write, const char* cmd) {

    int from_parent[2], from_child[2];
    int pid;

    if(pipe(from_child) != 0 || pipe(from_parent) != 0) {
        perror("pipe");
        return -1;
    }

    pid = fork();
    if(pid < 0) {
        perror("fork");
        return -1;
    }

    if(pid == 0) {
        /* Child */
        close(from_child[0]);
        close(from_parent[1]);
        dup2(from_parent[0], 0);
        dup2(from_child[1], 1);

        execl("/bin/sh", "sh", "-c", cmd, NULL);
        perror("execl");
        exit(1);
    }

    /* Parent */
    close(from_child[1]);
    close(from_parent[0]);
    *fd_write = fdopen(from_parent[1], "w");
    *fd_read = fdopen(from_child[0], "r");
    return pid;
}
