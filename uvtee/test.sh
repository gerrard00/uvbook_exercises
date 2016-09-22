nodemon --ext 'c' --ext '' --exec "make && cat main.c | ./uvtee.o ../junk/1.txt ../junk/2.txt || true" main.c  Makefile
