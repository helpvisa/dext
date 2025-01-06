# dext - the distraction-free terminal text editor
## about
dext is a text editor whose goal is to help you write with the least amount of distraction possible. Its key distinction is its ability to wrap and centre text in the middle of your terminal, creating a much more pleasant writing and reading experience that is more akin to typing on a typewriter or reading from the pages of a book.

By default, dext aims to allow a maximum of no more than 72 characters per line in order to more accurately mimic a paged reading experience.

dext is currently under heavy development; it should be considered a proof of concept and has many bugs and incomplete features.

## todo
- accurate cursor movements up and down through lines of text (which remember their 'rightmost' position)
- open / save to plain text or markdown
- basic markdown style formatting for headers, bold, and italics
- vim-style gestures for moving forward / backward by word, sentence, or paragraph
- character, word, sentence, line, and paragraph counts

## building
dext aims to support the ANSI C89 standard. Why? Because it would be fun to get it to build on DOS or other similarly irrelevant operating systems (and fun is important)!

For the time being, you should be able to compile by installing your chosen platform's curses library (on linux this is usually ncurses) and running ```make```.
Compiling dext on *BSD systems requires gmake.

It is not yet possible to compile dext on Windows platforms with the provided Makefile.

dext is currently known to compile with:
- tcc (tiny C compiler)
- pcc
- gcc
- clang
- cproc

dext operates as expected on OpenBSD.
