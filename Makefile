# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: cgoh <cgoh@student.42singapore.sg>         +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/06/09 12:48:34 by athonda           #+#    #+#              #
#    Updated: 2025/08/26 18:08:05 by cgoh             ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME_S	=	webserv

SRC_F_S	=	main.cpp Server.cpp Location.cpp Network.cpp Config.cpp\
            cgi_handler.cpp cgi_utils.cpp Epoll.cpp ClientConnection.cpp\
            HttpRequest.cpp HttpResponse.cpp TmpDirCleaner.cpp BaseFile.cpp\
            Timer.cpp ConnectionExpiration.cpp CGI.cpp Pipe.cpp

SRC_DIR	=	.
SRC_S	=	$(patsubst %.cpp,$(SRC_DIR)/%.cpp,$(SRC_F_S))

OBJ_DIR	=	obj
OBJ_S	=	$(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC_F_S))

INC_DIR	=	.
S_DEP	=	$(SRC_S:.cpp=.d)

IFLAGS	=	-I$(INC_DIR)
CXXFLAGS	=	-std=c++98 -Wall -Wextra -Werror -Wpedantic -MMD -MP -ggdb3 \
    -Wcast-align=strict \
    -Wcast-qual \
    -Wconversion \
    -Wdate-time \
    -Wdouble-promotion \
    -Wduplicated-cond \
    -Wduplicated-branches \
    -Wfloat-equal \
    -Wformat=2 \
    -Wimplicit-fallthrough=5 \
    -Wlogical-op \
    -Wmissing-declarations \
    -Wmissing-include-dirs \
    -Wnon-virtual-dtor \
    -Wnull-dereference \
    -Wold-style-cast \
    -Woverloaded-virtual \
    -Wshadow \
    -Wsign-conversion \
    -Wsuggest-override \
    -Wswitch-default \
    -Wswitch-enum \
    -Wundef \
    -Wunused-macros \
    -Wuseless-cast \

CXX		=	c++

$(NAME_S): $(OBJ_S)
	$(CXX) $(OBJ_S) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) $(IFLAGS) -c $< -o $@

.PHONY: all clean fclean re

all: $(NAME_S)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME_S)

re: fclean all

-include $(S_DEP)
