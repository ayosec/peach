
let s:peach_last_filter = ""

" Starts peach in the current directory
function! PeachFinder()

    let l:tmp_file = tempname()

    exe "silent :!peach-finder -s ~/.peach_score -f '" . s:peach_last_filter . "' -x .git,*.png,*.gif,*.jpeg,*.jpg,*.o,*.swp .  2> " . l:tmp_file
    redraw!

    for l:line in readfile(l:tmp_file, '', 100)
        if l:line =~ '^OPEN'
            call PeachOpenInTab(l:line[5:])
        elseif l:line =~ '^FILTER'
            let s:peach_last_filter = l:line[7:]
        endif
    endfor

    call delete(l:tmp_file)

endfunct

function! PeachOpenInTab(filename)

    let l:cwd = getcwd()
    let l:filename = a:filename

    if l:filename[0:strlen(l:cwd)-1] == l:cwd
        let l:filename = l:filename[strlen(l:cwd)+1:]
        echo l:filename
    end

    let l:bufnum = bufnr(l:filename)
    if l:bufnum > -1
        for l:i in range(tabpagenr('$'))
            if index(tabpagebuflist(l:i + 1), l:bufnum) >= 0
                exe ':' . (l:i + 1) . "tabnext"
                return
            end
        endfor
    end

    " Use current buffer if it is empty and has no changes
    if empty(bufname("%")) && !getbufvar("%", "&modified")
        exe ":e " . l:filename
    else
        exe ":tabedit " . l:filename
    end

endfunct

map <F3> :call PeachFinder()<CR>
