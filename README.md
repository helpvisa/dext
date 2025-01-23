# dext - the distraction-free terminal text editor
## about
dext is a text editor whose goal is to help you write with the least amount of
distraction possible. Its key distinction is its ability to wrap and centre
text in the middle of your terminal, creating a much more pleasant writing and
reading experience that is more akin to typing on a typewriter or reading from
the pages of a book.

By default, dext aims to allow a maximum of no more than 72 characters per
line in order to more accurately mimic a paged reading experience.

dext is currently under heavy development; it should be considered a proof of
concept and has many bugs and incomplete features.

## todo
- accurate cursor movements up and down through lines of text
- basic markdown style formatting for headers, bold, and italics
- gestures for moving forward / backward by word, sentence, or paragraph
- character, word, sentence, line, and paragraph counts

## how to use, tips, and ethos
dext operates on a few basic commands, some familiar. Pressing ```i``` or
```a``` will bring you into ```insert``` or ```append``` mode
respectively, at which point you will be able to add text to a given line.
Lines will automatically wrap as you write, but internally this is
still considered one 'line' of text and is merely a rendering trick. Perhaps
in the future, this line-wrapping technique will be made to more closely mimic
Vim's "wrapmargin" feature; the current technique was chosen as it prevents
newline characters from getting intermingled with your writing, which can be
helpful for rendering "unformatted" text files.

Once you are done inserting text, you can press the ```ESC``` key to
return to ```command``` mode. In ```command mode```, pressing ```q``` twice
will allow you to exit dext. You can additionally use the ```h```, ```j```,
```k```, and ```l``` keys to navigate through the text.

Pressing ```ESC``` an additional time while in command mode will put you
into ```extended command``` mode; this mode allows you to change the
current filepath (```r```), open the file at the current filepath (```o```),
or save your text to the current filepath (```w```). Please keep in mind
that simply changing the filepath is not enough to "load" a new file; you
must first change the current filepath with ```r``` and then use ```o```
to read the file from that location!

## building
dext aims to support the ANSI C89 standard. Why? Because it would be fun to get
it to build on DOS or other similarly irrelevant operating systems (and fun is
important)!

For the time being, you should be able to compile by installing your chosen
platform's curses library (on linux this is usually ncurses) and running
```make```.

Compiling dext on *BSD systems requires gmake.

dext is untested on Windows, but maybe you can try compiling it with MSYS2 and
let me know how it goes?

dext is currently known to compile with:
- tcc (tiny C compiler)
- pcc
- gcc
- clang
- cproc
