CC ?= gcc
INCLUDE_DIR := ./include
CFLAGS ?= -Wall -Wextra -I${INCLUDE_DIR} -g
LDFLAGS ?= 
SRC_DIR ?= src
OBJ_DIR ?= build
DOC_DIR ?= doc
SRCS = ${SRC_DIR}/main.c ${SRC_DIR}/parser.c ${SRC_DIR}/processus.c ${SRC_DIR}/builtins.c
HEADERS = ${INCLUDE_DIR}/parser.h ${INCLUDE_DIR}/processus.h ${INCLUDE_DIR}/builtins.h
DOXYGEN ?= $(strip $(shell which doxygen))
DOXYGEN_CONFIG ?= ${DOC_DIR}/Doxyfile

EXEC ?= minishell

.PHONY: clean deepclean doc

${EXEC}: ${OBJ_DIR}/main.o ${OBJ_DIR}/parser.o ${OBJ_DIR}/processus.o ${OBJ_DIR}/builtins.o
	${CC} $^ -o $@ ${LDFLAGS}

${OBJ_DIR}/main.o: ${SRC_DIR}/main.c include/parser.h include/processus.h include/builtins.h
	${CC} ${CFLAGS} -c $< -o $@

${OBJ_DIR}/parser.o: ${SRC_DIR}/parser.c include/parser.h include/processus.h
	${CC} ${CFLAGS} -c $< -o $@

${OBJ_DIR}/processus.o: ${SRC_DIR}/processus.c include/processus.h include/builtins.h
	${CC} ${CFLAGS} -c $< -o $@

${OBJ_DIR}/builtins.o: ${SRC_DIR}/builtins.c include/builtins.h
	${CC} ${CFLAGS} -c $< -o $@

test_parser: ${OBJ_DIR}/parser.o ${OBJ_DIR}/processus.o ${OBJ_DIR}/builtins.o src/test_parser.c
	${CC} $^ -o $@ ${LDFLAGS}

test_builtins: ${OBJ_DIR}/parser.o ${OBJ_DIR}/processus.o ${OBJ_DIR}/builtins.o src/test_builtins.c
	${CC} $^ -o $@ ${LDFLAGS}

clean:
	rm -f ${OBJ_DIR}/*.o

deepclean: clean
	rm -f ${EXEC}
	rm -rf ${DOC_DIR}/html ${DOC_DIR}/latex

doc: ${DOXYGEN_CONFIG} ${HEADERS} ${SRCS}
ifeq (${DOXYGEN},)
	@echo "Doxygen not found, please install it to generate documentation."
else
	${DOXYGEN} $<
endif

