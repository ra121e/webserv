# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: athonda <athonda@student.42singapore.sg    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/06/09 12:48:34 by athonda           #+#    #+#              #
#    Updated: 2025/06/09 19:07:52 by athonda          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME_S	=	Server
NAME_C	=	Client

SRC_F_S	=	server.cpp
SRC_F_C	=	client.cpp

SRC_DIR	=	.
SRC_S	=	$(patsubst %.cpp,$(SRC_DIR)/%.cpp,$(SRC_F_S))
SRC_C	=	$(patsubst %.cpp,$(SRC_DIR)/%.cpp,$(SRC_F_C))

OBJ_DIR	=	obj
OBJ_S	=	$(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC_F_S))
OBJ_C	=	$(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC_F_C))

INC_DIR	=	.
DEP		=	$(wildcard $(patsubst %.cpp,$(INC_DIR)/%.hpp,$(SRC_F)))

IFLAGS	=	-I$(INC_DIR)
CFLAGS	=	-g -Wall -Werror -Wextra -std=c++98
CC		=	c++

$(NAME_S): $(OBJ_S) $(DEP)
	$(CC) $(CFLAGS) $(OBJ_S) -o $@

$(NAME_C): $(OBJ_C) $(DEP)
	$(CC) $(CFLAGS) $(OBJ_C) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(IFLAGS) -c $< -o $@

.PHONY: all clean fclean re server client

all: $(NAME_S) $(NAME_C)

server: $(NAME_S)

client: $(NAME_C)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME_S) $(NAME_C)

re: fclean all

norm:
	clear
	nm -u $(NAME_S) $(NAME_C)
	grep -n -e "main" $(SRC) $(DEP)
