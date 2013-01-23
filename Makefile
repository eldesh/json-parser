###
###

NAME=   json_parser

### include debug information: 1=yes, 0=no
DBG?= 0

DEPEND= dependencies

LIBDIR= ./lib
OBJDIR= ./obj

CC=      $(shell which gcc) 
CXX=     $(shell which g++)

CFLAGS=  -std=gnu99 -pedantic -ffloat-store -fno-strict-aliasing -fsigned-char

FLAGS=   -Wall -Wextra -pedantic-errors -Wformat=2 -Wcast-align -Wwrite-strings -Wfloat-equal -Wpointer-arith \
		 -Wno-uninitialized -Wno-unused-parameter

ifeq ($(DBG),1)
SUFFIX= .dbg
FLAGS+= -g
else
SUFFIX=
FLAGS+= -O3
endif

SRC= json.c

OBJ= $(SRC:%.c=$(OBJDIR)/%.o$(SUFFIX))

LIB= $(LIBDIR)/lib$(NAME)$(SUFFIX).a

.PHONY: default distclean clean depend

default: messages objdir_mk depend $(LIB)

messages:
ifeq ($(DBG),1)
	@echo 'Compiling with Debug support...'
endif

clean:
	@echo remove all objects
	@rm -rf $(OBJDIR)
	@rm -f test
 
distclean: clean
	@rm -f $(DEPEND)
	@rm -rf $(LIBDIR)


$(LIB): $(OBJ)
	@echo
	@echo 'creating library "$(LIB)"'
	@mkdir -p $(LIBDIR)
	@ar rv $@ $?
	@ranlib $@
	@echo '... done'
	@echo

depend:
	@echo
	@echo 'checking dependencies'
	@$(SHELL) -ec '$(CC) -MM $(CFLAGS)                              \
	     $(SRC)                                                     \
         | sed '\''s@\(.*\)\.o[ :]@$(OBJDIR)/\1.o$(SUFFIX):@g'\''   \
         >$(DEPEND)'
	@echo

$(OBJDIR)/%.o$(SUFFIX): %.c
	@echo 'compiling object file "$@" ...'
	@$(CC) -c -o $@ $(FLAGS) $(CFLAGS) $<

objdir_mk:
	@echo 'Creating $(OBJDIR) ...'
	@mkdir -p $(OBJDIR)

test: test.c $(LIB)
	@$(CC) -o $@ $(FLAGS) $(CFLAGS) -Llib $< -l$(NAME)$(SUFFIX)

-include $(DEPEND)

