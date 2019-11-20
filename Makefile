PROGNAME = readexe
CC		 = clang 
CFLAGS	 = -march=native -ggdb3
LDFLAGS  = 
RM		 = rm -f
BINEXT	 =
OBJEXT	 = o
SRC		 = main.c err.c
OBJ		 = $(SRC:.c=.$(OBJEXT))

$(PROGNAME)$(BINEXT): $(OBJ)
	$(CC) -o $(PROGNAME)$(BINEXT) $(LDFLAGS) $(OBJ)

clean:
	$(RM) $(PROGNAME)$(BINEXT) *.$(OBJEXT)