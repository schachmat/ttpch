include config.mk

SRC = ttpch.c

OBJ = ${SRC:.c=.o}

all: options ttpch

options:
	@echo ttpch build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $< -o $@

${OBJ}: config.mk

ttpch: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f ttpch ${OBJ}

.PHONY: all options ttpch clean
