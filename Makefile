PROGNAME = readexe
CC		 = clang 
CFLAGS	 = -march=native -ggdb3
LDFLAGS  = 
RM		 = rm -f
BINEXT	 =
OBJEXT	 = o
SRC		 = main.c err.c
OBJ		 = $(SRC:.c=.$(OBJEXT))

# You can remove err.c from SRC on most modern systems. 
# Add -I. to CFLAGS if you get linker/header errors.

$(PROGNAME)$(BINEXT): $(OBJ)
	$(CC) -o $(PROGNAME)$(BINEXT) $(LDFLAGS) $(OBJ)

clean:
	$(RM) $(PROGNAME)$(BINEXT) *.$(OBJEXT)