     Prefix-1                      ESC
     The next character typed will be interpreted  on  the  basis
     that  it  was preceded by the command Prefix-1.  This is one
     of two such commands whose purpose in life  is  to  increase
     the  number  of  commands that can be bound to keys and thus
     easily invoked.

     Prefix-2                      ^X
     The next character typed will be interpreted  on  the  basis
     that  it  was preceded by the command Prefix-2.  This is one
     of two such commands whose purpose in life  is  to  increase
     the  number  of  commands that can be bound to keys and thus
     easily invoked.

     append-region                 [unbound]
     Appends the region bounded by the  point  and  mark  to  the
     named file.  If the file does not exist it is created.

     backward-character            ^B
     Moves the point backward one character in  the  buffer.   If
     the  point  is  at the beginning of the line, it moves it to
     the end of the previous line.

     backward-paren                ESC-^B
     This moves  the  point  to  the  matching  open  parenthesis
     (brace)  of  the  close  parenthesis (brace) just before the
     point.  If there is no match, an error is reported  and  the
     point remains unchanged.

     backward-word                 ESC-B
     If in the middle of a word, the point moves to the beginning
     of  the word.  Otherwise it moves the point to the beginning
     of the previous word.

     beginning-of-file             ESC-<
     Moves the point to the beginning of the  current  buffer  so
     that the character after the point is the first character of
     the buffer.

     beginning-of-line             ^A
     Moves the point to the beginning of the current line.

     beginning-of-sentence         ESC-A
     Moves the point to the beginning of the current sentence.

     beginning-of-window           ESC-,
     Moves the point to the first character in the window.

     bind-macro-to-key             [unbound]
     Binds a macro to a key so that future hits on that key  will
     run the macro.

     bind-to-key                   [unbound]
     Binds a command to a key so that future  hits  on  that  key
     will run the command.  For example, the command next-line is
     bound to the key ^N.

     buffer-position               [unbound]
     Prints the line and column  of  the  point  in  the  current
     buffer.

     case-region-lower             ^X-^L
     Changes all the letters between the point and mark to  lower
     case.

     case-region-upper             ^X-^U
     Changes all the letters between the point and mark to  upper
     case.

     case-word-capitalize          ESC-C
     If the point is in the middle of a word, it capitalizes  the
     letter  after  the point and changes the rest of the word to
     lower case.  Otherwise it capitalizes the  first  letter  of
     the  next  word  and  changes the rest of this word to lower
     case.  In any case the point is left at the end of the word.

     case-word-lower               ESC-L
     If the point is in the middle of a word, it changes the rest
     of  the  word  to lower case.  Otherwise it changes the next
     word to lower case.  In either case the point is left at the
     end of the effected word.

     case-word-upper               ESC-U
     If the point is in the middle of a word, it changes the rest
     of  the  word  to upper case.  Otherwise it changes the next
     word to upper case.  In either case the point is left at the
     end of the effected word.

     char-to-octal-insert          [unbound]
     Reads a character from the terminal and  inserts  its  octal
     value  preceded  by  a  back-slash '\', into the text at the
     point.  For example if  the  character  ESC  is  typed,  the
     string \033 is inserted.

     clear-and-redraw              ESC-^L
     Clears and redraws the screen without  changing  the  screen
     orientation.   This is useful if the screen gets garbaged by
     output from another program or by someone writing you.

     compile-it                    ^X-^E
     Writes all the modified buffers and runs  the  UNIX  program
     make  into  a  buffer  called  Errors.   After  the  make is
     completed, the buffer is parsed for C type error messages to
     be used by the next-error command.

     copy-region                   ESC-W
     Does a pretend kill of the region bounded by the  point  and
     mark.   The  next  call  to  yank  will yank the region just
     copied.

     c-tab                         [unbound]
     When in c-mode, the command c-tab is automatically bound  to
     the  tab  key,  and when invoked, the point is tabbed to the
     "right" place for C programs.

     delete-next-char              ^D
     The character after the point is deleted moving the rest  of
     the  line  to the left one, leaving the point unchanged.  If
     at the end of the line, the line boundary is deleted joining
     the current line with the next line.

     delete-next-word              ESC-D
     If the point is in the middle of a word, it deletes  to  the
     end of the word.  Otherwise it deletes the entire next word.
     In either case, the point doesn't move, and the deleted text
     is saved in the kill buffer.

     delete-other-windows          ^X-1
     Makes the current window the only window by deleting all the
     other windows.  The window grows to fill the screen as it is
     when the editor starts up.

     delete-previous-char          DELETE
     Deletes the character before the point moving  the  rest  of
     the  line to the left one.  If the point is at the beginning
     of a line, the previous line is joined with the current line
     and  the  point  is  moved to what used to be the end of the
     previous line.

     delete-previous-word          ESC-DELETE
     If the point is in the middle of a word, it deletes  to  the
     beginning  of  that  word.   Otherwise it deletes the entire
     previous word.  In either case, the deleted text is saved in
     the kill buffer.

     delete-to-killbuffer          ^W
     Deletes all the text between the point and mark,  saving  it
     in  the  kill-buffer.   This  is one of the the ways to move
     text from place in the buffer to another.  The yank  command
     will  insert  the  most  recently deleted text at the point.
     Jove remembers 10 separate kills in a kill ring.  The  yank-
     pop  command  can  be  used  to go through the ring one at a
     time.

     delete-white-space            ESC-\f1
     Deletes all tabs and space around the point on  the  current
     line.  This does not work over line boundaries.

     describe-command              [unbound]
     Prompts for the name of a command  and  describes  what  the
     command does into a buffer.

     describe-key                  [unbound]
     Prompts for a key and prints the command that  is  bound  to
     that key.

     delete-current-window         ^X-D
     If there is more than one window on the screen, the  current
     window  is  deleted and is moved to neighboring window.  The
     space from the deleted window is given to  the  new  current
     window.

     end-of-file                   ESC->
     Moves the  point  to  the  end  of  the  buffer  making  the
     character before the point the last character in the buffer.

     end-of-line                   ^E
     Moves the point to the end of the current line.

     end-of-sentence               ESC-E
     Moves the point to the end of the sentence.

     end-of-window                 ESC-.
     Moves the point to the last character in the window.

     erase-buffer                  [unbound]
     Prompts for a buffer name, and deletes all the text in  that
     buffer.   If  the  buffer is modified, the user is asked for
     confirmation.

     exchange-point-and-mark       ^X-^X
     Sets the specified mark to the point and set  the  point  to
     the  mark.  A mark is referenced by a numerical argument; if
     none is given the zero'th mark is assumed.

     execute-extended-command      ESC-X
     Prompts for a command to run using command completion.  This
     is  how all commands would be run if not for the bind-to-key
     command.

     execute-keyboard-macro        [unbound]
     All the characters that  were  remembered  with  the  start-
     remembering  command  are  executed as if they were typed at
     the keyboard.

     execute-macro                 [unbound]
     This is the same as execute-keyboard-macro  except  that  it
     prompts for the name of a macro to execute.

     exit-jove                     ^X-^C
     Exits jove.  It asks  for  confirmation  if  there  are  any
     modified buffers which have not been saved.

     filter-region                 [unbound]
     Prompts for a UNIX command to send a region to.  The  region
     will  be  replaced  with  the  output from the command.  For
     example to  format  a  C  procedure,  a  region  around  the
     procedure  should  be  defined and the filter-region command
     used to send the region through the UNIX C beautifier.

     find-file                     ^X-^F
     Prompts for a file to find.  If that file is not already  in
     a  buffer,  it  creates a new buffer and reads the file into
     that buffer.  If the file is already in another buffer, that
     buffer  will  be selected instead.  In any event the current
     buffer becomes buffer with that file in it.

     find-file-into-other-window   ^X-4
     Do a find file into another window.  If there  is  only  one
     window,  is split.  If there is more than one window, one of
     the windows that is not the current window is used instead.

     find-tag                      ^X-^T
     Finds the file with the C tag which is  prompted  for.   The
     UNIX  program  ctags(1)  should be used to create a database
     with the  necessary  information  to  find  the  file  which
     contains the tag.  If the tag cannot be found, the point and
     buffer are unchanged.

     first-non-blank               ESC-M
     Moves the point to the first  non  blank  character  in  the
     current line.

     forward-char                  ^F
     Moves the point forward one character in the buffer.  If  at
     the  end  of  the  current  line  the  point is moved to the
     beginning of the next line.

     forward-paren                 ESC-^B
     Moves the point to the matching close parenthesis (brace) of
     the  open  parenthesis  (brace)  right  after the point.  If
     there is no match,  an  error  is  reported  and  the  point
     remains unchanged.

     forward-word                  ESC-F
     If the point is in the middle of a word, it moves to the end
     of  the  current word.  Otherwise it moves to the end of the
     next word.

     four-times                    ^U
     Multiple the numeric argument by four.  The sequence  ^U-^U-
     ^N runs the command next-line 16 times.

     goto-line                     ESC-G
     Moves the point to the numeric argument line in the  buffer.
     If no argument is supplied, the the point moves to the first
     line in the buffer.

     grow-window                   ^X-^
     Makes the current window one line larger if it  can.   There
     has  to  be more than one window, and the window which would
     get smaller has to be big enough to get smaller.

     i-search-forward              [unbound]
     Incremental search forward.  See the EMACS Manual for TWENEX
     Users if you care about this command.  I do not have time to
     explain it right now.

     i-search-reverse              [unbound]
     Incremental search reverse.  See the EMACS Manual for TWENEX
     Users if you care about this command.  I do not have time to
     explain it right now.

     insert-file                   ^X-^I
     If the named file exists and can be read, its  contents  are
     inserted into the current buffer at the point.  Otherwise an
     error is reported and the point is unchanged.

     init-bindings                 [unbound]
     Initialize  the  bindings  to  the  way  they  were  at  the
     beginning of the edit.

     justify-paragraph             ESC-J
     Justifys a paragraph.  Paragraphs are bounded by blank lines
     or  lines  that  begin  with  a  period.   The margin-length
     variable  determines  the  length  of  each  line   in   the
     paragraph.

     kill-buffer                   ^X-K
     Deletes a named buffer.  This  command  is  not  reversible;
     once  the  buffer  is  deleted,  it  is gone forever.  It is
     useful when the editor runs out of  space  and  there  is  a
     buffer which you do not care about.

     kill-to-end-of-line           ^K
     Kill all the text from the point to the end of the line.  If
     the  point  is  at  the end of the line, the current line is
     joined with the next line.  If a numeric count is given,  an
     internal  mark  is  set at the point, the point moves to the
     beginning of a line count below the current  line,  and  the
     region is deleted to the kill-buffer.  The saved test can be
     yanked  back  with  the   yank   command   (see   delete-to-
     killbuffer).   If this command is run several times in a row
     all the kills are merged into the same kill ring entry, thus
     running  the yank command will yank all of them back instead
     of just the last one.

     list-buffers                  ^X-^B
     Creates a buffer called Buffer list that contains a list  of
     all the buffers jove currently knows about.  Each line tells
     each buffer's number, type, file  associated  with  it,  its
     name, and a * if the buffer is modified.

     make-buffer-unmodified        ESC-~
     Makes the editor forget that the buffer has  been  modified.
     The * on the mode line will disappear if it is there.

     name-keyboard-macro           [unbound]
     A new macro is defined with the name supplied by  the  user.
     The  current definition of the keyboard macro is copied into
     this new macro, making it possible change the keyboard macro
     without  losing  the old version.  Now it is possible to run
     the new macro by binding the macro to a  key,  or  with  the
     execute-macro  command.   Note  that  only  named macros are
     saved with the write-macros-to-file command, so to  remember
     macros  from  one jove to another, one must give the macro a
     name, execute the  write-macros-to-file  command,  and  upon
     invokation of a new jove, the read-macros-from-file" command
     should be executed.

     newline                       [unbound]
     Divides the current line at the point moving  the  point  to
     the  beginning  of  the  newly  created line.  It is usually
     bound to the return key.

     newline-and-backup            ^O
     Divides the current line  at  the  point  like  the  newline
     command, but leaves the point unchanged.

     newline-and-indent            [unbound]
     Same as newline except that it inserts white  space  at  the
     beginning  of the newline copying the indent of the previous
     line.

     next-error                    ^X-^N
     Takes the next error  message  (as  returned  by  the  parse
     commands),  finds  the file in which the error occurred, and
     sets the the point to the line on which the error  occurred.
     The error message will be displayed at the top of the errors
     buffer.  (The  compile-it  command  automatically  runs  the
     parse-C-errors and the next-error commands).

     next-line                     ^N
     Moves the point to the next line keeping the column as close
     to the current column as possible.

     next-page                     ^V
     Puts the bottom line of the window at the top of the window,
     moving  the  point  to  the  top  of  the  window and to the
     beginning of the line.

     next-window                   ^X-N
     Moves to the next window in  the  screen.   If  the  current
     window is the last window, it moves to the first window.  It
     is an error to move to the next window when  there  is  only
     one window on the screen.

     number-lines-in-window        [unbound]
     Each line in the current window is displayed with  its  line
     number to the left.  The number isn't part of the buffer and
     will disappear when  this  command  is  executed  again  (it
     toggles).

     page-next-window              ESC-^V
     This command does a next-page on the next window.  If  there
     is only one window the editor complains.

     paren-flash                   [unbound]
     When  the  variable  show-match  is  non-zero,   the   close
     parenthesis/brace  keys  are  bound  to  this command.  When
     invoked,  this  command  inserts  the  character  typed  and
     temporarily  flashes  to the matching open parenthesis/brace
     for about one second.

     parse-C-errors                [unbound]
     This command takes C compiler (or similar in format)  errors
     and  sets  the  editor  up for subsequent invocations of the
     next-error command.  Giving the UNIX command grep(1) the  -n
     option  prints  the  output  in  the  same  format  as the C
     compiler, thus running this command into a buffer  makes  it
     possible  to  parse its output. This is a very useful way to
     look at all the occurrences of certain  strings  in  several
     files.

     parse-LINT-errors             [unbound]
     This is the  same  as  parse-C-errors  excepts  parses  lint
     errors.

     pause-jove                    [unbound]
     If the system has the Berkeley job control features, control
     is  returned  to  the superior shell.  Otherwise an inferior
     shell is spawned.

     previous-line                 ^P
     Moves the point to the previous  line  trying  to  keep  the
     column the same.

     previous-page                 ESC-V
     Moves the top line in the window to the bottom line leaving,
     the  point  at the top of the window and at the beginning of
     the line.

     previous-window               ^X-P
     Move to the previous window in the screen.  If currently  in
     the  first window, moves to the last window.  It is an error
     to move to the previous window when there is only one window
     on the screen.

     print                         [unbound]
     Prompts for a variable name and prints its value.

     query-replace-search          ESC-Q
     Upon receipt of a search and replacement string, the  editor
     replaces  all  occurrences  of  the  search  string with the
     replacement string.  For each occurrence the editor asks the
     user what to do.  The choices are:

             ' '     to replace this occurrence.
             DELETE  to skip this occurrence.
             'r'     to recursive edit.
             'p'     to proceed to replace all occurrences.
             RETURN  to stop

     When there are no more occurrences, the point is moves  back
     to  its  initial position.  Recursive edit makes it possible
     to temporarily suspend  the  query-replace-search,  let  the
     user  go  off  and  do  some editing, and then return to the
     search after the editing is finished.  Executing the command
     exit-jove returns from the recursive edit.

     quote-char                    ^Q
     Quotes the next character typed for insertion.  This is used
     to   insert  special  characters  which  normally  would  be
     interpreted as commands.

     read-file                     ^X-^R
     Prompts for a file to read into the current buffer.  It will
     erase  the  old contents so if the buffer has been modified,
     but not saved, jove complains and ask for confirmation.

     read-macros-from-file         [unbound]
     Prompts for a file that  was  previously  written  with  the
     write-macros-to-file  command,  and reads them back into the
     editor.

     redraw-display                ^L
     Redraws the window with the current line in the middle.   If
     a  numeric argument is provided the current line is moved to
     the argument line. If the current line is in the same  place
     as before, the window is cleared and redrawn.

     reinitialize-terminal
     If the value of the  variable  allow-^S-and-^Q  is  changed,
     this  command should be called to make the necessary changes
     to the terminal.

     replace-search                ESC-R
     This is the same as query-replace-search except  the  editor
     does not ask whether to replace it; it always does.

     ring-the-bell                 ^G
     Exactly what is says.  Its useful when it  is  not  apparent
     what the editor is currently doing.

     scroll-one-line-down          ESC-Z
     Scrolls the current window down one line.   If  the  current
     line  moves off the bottom of the window, it is moved to the
     middle of the window.

     scroll-one-line-up            ^Z
     Scrolls the current window up one line.  If the current line
     moves  off  the  bottom  of  the  window, it is moved to the
     middle of the window.

     search-forward                ^S
     Prompts for a string to search for and searches for the next
     instance of that string in the buffer (see searching above).

     search-reverse                ^R
     Prompts for a string to search  for  and  searches  for  the
     previous   instance  of  that  string  in  the  buffer  (see
     searching above).

     select-buffer                 ^X-B
     Prompts for a buffer name and makes that buffer the  current
     buffer.   If  the  buffer  does  not  exist  a new buffer is
     created with nothing in it.  If a number is supplied instead
     of  a name and a buffer exists with that number, that buffer
     is selected.  Otherwise a new buffer is  created  with  that
     number as a name.

     self-insert                   [very-bound]
     This is bound to all the keys that should be  inserted  when
     typed.   This  commands does not work unless bound to a key.
     Running this command manually will probably cause  a  random
     character to be inserted.

     set                           [unbound]
     Prompts for a variable  name  and  a  value,  and  sets  the
     variable to that value.

     set-mark                      ^@
     Sets a mark at the current  buffer  location.   If  given  a
     numeric  argument, it sets a mark whose name is that number.
     Otherwise it sets the zero'th mark.

     shell-command                 ^X-!
     Prompts for a UNIX shell  command  to  be  run  placing  the
     output  from  the  command  into  a  buffer  called  Command
     execution.  If the  a  numeric  argument  is  provided,  the
     buffer  is  left  alone  before the UNIX command is started.
     Otherwise the buffer is emptied.

     shell-command-to-buffer       [unbound]
     The same as shell-command except that it asks for a specific
     buffer to place the output in instead of Command execution.

     source                        [unbound]
     This prompts for a file  name  which  contains  a  bunch  of
     editor  commands.  These commands typically set variables or
     bind commands to keys.  Running source on a file which  does
     not  have editor commands in it will likely cause the editor
     to crash.  This is a serious bug.

     shrink-window                 [unbound]
     Shrink the current window  by  one  line  if  the  resulting
     window  would  not be too small.  It is an error to run this
     command when there is only one window.

     spell-buffer                  [unbound]
     Sends the entire buffer to the  UNIX  spell  program.   Jove
     will go through the list of spelling errors and asks whether
     or not a word is spelled correctly.   If  it  is  not,  jove
     remembers  where  each occurrence of the misspelled word is.
     The point in the buffer being spelled is positioned  at  the
     end  of  the current misspelled word. The next-error command
     moves to the next occurrence of the current word, or to  the
     first occurrence of the next word.

     split-current-window          ^X-2
     Splits the current window into two smaller windows,  if  the
     resulting  windows  would not be too small.  The two windows
     have the same buffer associated with them,  namely  the  one
     that the original window had.

     start-remembering             [unbound]
     This tells jove  to  start  remembering  all  the  following
     keystrokes  until  the stop-remembering command is executed.
     The saved commands are saved in the keyboard macro, and  can
     be  re-executed  with  the  execute-keyboard-macro  command.
     This is useful when it is necessary to run the same  command
     lots of times.

     stop-remembering              [unbound]
     This terminates the  definition  of  a  macro.   See  start-
     remembering for more details.

     string-length                 ^X-C
     Prints, on  the  message  line,  the  number  of  characters
     between  two quotes. The point must be between two quotes or
     the editor complains.  This  is  useful  for  C  programmers
     especially.

     suspend-jove                  [unbound]
     Same as pause-jove.

     text-insert                   [unbound]
     When the variable text-fill  is  non-zero,  the  self-insert
     keys  are  bound  to  this command.  This inserts characters
     like self-insert does but when the line gets  to  a  certain
     length,  a newline is automatically inserted.  This makes it
     possible to type in a paper without having  to  remember  to
     hit return, i.e the editor does it automatically.

     transpose-char                ^T
     Switches the characters on  opposite  sides  of  the  point,
     namely  the  character  before  the cursor and the character
     under the cursor.

     unbound                       [unbound]
     The unbound function is bound to all the keys that don't run
     commands.  It is essentially a no-op.

     vt100-arrow-keys              [unbound]
     This makes the arrow keys work on the vt100 terminal.

     write-current-file            ^X-^S
     This writes the current buffer to the file  associated  with
     the  buffer,  without asking.  If there is currently no file
     associated with the buffer, the editor complains.  (See  the
     write-named-file command below).

     write-macros-to-file          [unbound]
     Prompts for a file and writes all the current defined macros
     to  that  file.  The macros can be read back into the editor
     with the read-macros-from-file command.

     write-modified-files          ^X-^M
     Writes all the buffers that have been modified but not  been
     saved.

     write-named-file              ^X-^W
     Prompts for a file name and writes  the  current  buffer  to
     that file. If the file already exists and is not the current
     file name, the user is informed and asked if he really wants
     to  do  it.   Writing  a file erases the old contents of the
     file.

     write-region                  [unbound]
     Takes the region between the point and mark and writes it to
     a named file.

     yank                          ^Y
     Inserts, at the point, all the text that was  most  recently
     deleted with a delete command that saves the text it deleted
     to the kill ring.   The  point  moves  to  the  end  of  the
     inserted region.

     yank-pop                      ESC-Y
     Goes through the kill ring inserting each  entry  one  at  a
     time.   The  previous  command  has  to  have  been the yank
     command or the yank-pop command.

