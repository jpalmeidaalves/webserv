NAME = webserv

SRC = ./source/main.cpp\
		./source/utils.cpp\
		./source/Server.cpp\
		./source/HTTP.cpp\
		./source/Request.cpp\
		./source/Response.cpp\
		./source/MimeTypes.cpp\
		

OBJS = ${SRC:.cpp=.o}

INCLUDE = -I .

CC = c++
RM = rm -f
CPPFLAGS = -Wall -Wextra -Werror -std=c++98

.cpp.o:
		${CC} ${CPPFLAGS} ${INCLUDE} -c $< -o ${<:.cpp=.o}

$(NAME): ${OBJS}
		${CC} ${CPPFLAGS} ${INCLUDE} ${OBJS} -o ${NAME}

all:	${NAME}

clean:
		${RM} ${OBJS}

fclean:	clean
		${RM} ${NAME}

re: clean all

# ---------------------------------------------------------------------------- #
#                                     TESTS                                    #
# ---------------------------------------------------------------------------- #

test_multiple_requests:
	curl --parallel --parallel-immediate --parallel-max 3 --config urls.txt 

# ---------------------------------------------------------------------------- #
#                                 FORMAT RULES                                 #
# ---------------------------------------------------------------------------- #
format:
	clang-format -style=file -i ${SRCS} ${HEADERS}

# ---------------------------------------------------------------------------- #
#                                 LINTER RULES                                 #
# ---------------------------------------------------------------------------- #
lint:
	clang-tidy ${HEADERS} -checks=-*,cppcoreguidelines-*,readability-*
	clang-tidy ${SRC} -checks=-*,cppcoreguidelines-*,readability-*

# ------------------------------- END OF RULES ------------------------------- #


.PHONY: all clean flcean re format lint