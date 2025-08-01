# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/06/09 12:48:34 by athonda           #+#    #+#              #
#    Updated: 2025/07/13 22:18:00 by cgoh             ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME_S	=	webserv
NAME_C	=	Client

SRC_F_S	=	main.cpp Server.cpp Location.cpp Network.cpp Config.cpp parsing.cpp cgi_handler.cpp cgi_utils.cpp my_cgi.cpp
SRC_F_C	=	client.cpp

SRC_DIR	=	.
SRC_S	=	$(patsubst %.cpp,$(SRC_DIR)/%.cpp,$(SRC_F_S))
SRC_C	=	$(patsubst %.cpp,$(SRC_DIR)/%.cpp,$(SRC_F_C))

OBJ_DIR	=	obj
OBJ_S	=	$(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC_F_S))
OBJ_C	=	$(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC_F_C))

INC_DIR	=	.
S_DEP	=	$(SRC_S:.cpp=.d)
C_DEP	=	$(SRC_C:.cpp=.d)

IFLAGS	=	-I$(INC_DIR)
CXXFLAGS	=	-ggdb3 -Wall -Werror -Wextra -std=c++98 -MMD -MP -Weverything -Wno-padded -fstack-protector-strong -fno-delete-null-pointer-checks -fno-strict-overflow -fno-strict-aliasing
CXX		=	c++

$(NAME_S): $(OBJ_S)
	$(CXX) $(OBJ_S) -o $@

$(NAME_C): $(OBJ_C)
	$(CXX) $(OBJ_C) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(IFLAGS) -c $< -o $@

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

-include $(S_DEP) $(C_DEP)
