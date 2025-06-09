# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: athonda <athonda@student.42singapore.sg    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/06/09 12:48:34 by athonda           #+#    #+#              #
#    Updated: 2025/06/09 13:03:23 by athonda          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	=	Webserv

SRC_F	=	main.cpp \

SRC_DIR	=	.
SRC		=	$(patsubst %.cpp,$(SRC_DIR)/%.cpp,$(SRC_F))

OBJ_DIR	=	obj
OBJ		=	$(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC_F))

INC_DIR	=	.
DEP		=	$(wildcard $(patsubst %.cpp,$(INC_DIR)/%.hpp,$(SRC_F)))

IFLAGS	=	-I$(INC_DIR)
CFLAGS	=	-g -Wall -Werror -Wextra -std=c++98
CC		=	c++

$(NAME): $(OBJ) $(DEP)
	$(CC) $(CFLAGS) $(OBJ) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(DEP)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(IFLAGS) -c $< -o $@

.PHONY: all clean fclean re

all: $(NAME)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm $(NAME)

re: fclean all

norm:
	clear
	nm -u $(NAME)
	grep -n -e "main" $(SRC) $(DEP)