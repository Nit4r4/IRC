# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vferraro <vferraror@student.42lausanne.    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/05/23 15:18:37 by vferraro          #+#    #+#              #
#    Updated: 2023/08/03 14:04:50 by vferraro         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

PURP	= \e[35m
GREEN	= \e[32m
GREEN2	= \033[32m
RED		= \e[31m
WHITE	= \e[39m
YELLOW	= \033[33m
HIDDEN	= \e[8m
RESET	= \e[0m
BLUE	= \033[34m
CYAN	= \033[36m
MAGENTA = \033[35m
RUN_C	= \033[0;32m

NAME	= ircserv

SRCS	= main.cpp $(addprefix src/, Server.cpp Client.cpp notInUse.cpp ImpCommands.cpp Messages.cpp Channel.cpp)
MAIN	= main.cpp

OBJS	= ${SRCS:.cpp=.o}
#OBJ	= $(addprefix objs/, $(OBJS))

LOGO_IRC	= 	 @echo "$(CYAN) \tWelcome to : \n\
 								\tIIIIIIIIIIRRRRRRRRRRRRRRRRR           CCCCCCCCCCCCC\n\
 								\tI::::::::IR::::::::::::::::R       CCC::::::::::::C\n\
								\tI::::::::IR::::::RRRRRR:::::R    CC:::::::::::::::C\n\
								\tII::::::IIRR:::::R     R:::::R  C:::::CCCCCCCC::::C\n\
								\t  I::::I    R::::R     R:::::R C:::::C       CCCCCC\n\
								\t  I::::I    R::::R     R:::::RC:::::C              \n\
								\t  I::::I    R::::RRRRRR:::::R C:::::C              \n\
								\t  I::::I    R:::::::::::::RR  C:::::C              \n\
								\t  I::::I    R::::RRRRRR:::::R C:::::C              \n\
								\t  I::::I    R::::R     R:::::RC:::::C              \n\
								\t  I::::I    R::::R     R:::::RC:::::C              \n\
								\t  I::::I    R::::R     R:::::R C:::::C       CCCCCC\n\
								\tII::::::IIRR:::::R     R:::::R  C:::::CCCCCCCC::::C\n\
								\tI::::::::IR::::::R     R:::::R   CC:::::::::::::::C\n\
								\tI::::::::IR::::::R     R:::::R     CCC::::::::::::C\n\
								\tIIIIIIIIIIRRRRRRRR     RRRRRRR        CCCCCCCCCCCCC$(\RESET)\n\
								\t                                      By The Moires\n\
								\t  \n"

CC			= c++
CPPFLAGS	= -Wall -Wextra -Werror -g3 -std=c++98 -I./inc -fsanitize=address #-pedantic -lpoll

AR 		= ar rc
RM 		= rm -f

.c.o:		%.o : %.cpp
					$(CC)${CPPFLAGS}


logo:
			$(LOGO_IRC)

all: 		 logo ${NAME}

${NAME}:			${OBJS}
					@echo "⛳️ Flags and O->CPP ✅"
					@$(CC) $(CPPFLAGS) ${OBJS} -o $(NAME)
					@printf "$(BLUE)🍵 Creating $(NAME)$(RESET) ✅\n"
					@echo "$(CYAN)$(NAME) Compiled !  ✅ \033[39m(\033[31m๑\033[39m╹◡╹\033[31m๑\033[39m)"
					@echo ""
					@echo "$(RUN_C)****************************************************************"
					@echo "$(RUN_C)* Now you can run the program with ./ircserv <port> <password> *"
					@echo "$(RUN_C)****************************************************************\033[0m"


clean:
					@${RM} ${OBJS}
					@echo ""
					@echo "$(YELLOW)---- Cleaning library ----"
					@printf "$(BLUE)🧽 Cleaning $(NAME)$(RESET)\n"
					@echo "$(YELLOW)$(NAME) is all clean ! $(ORANGE)(ﾉ◕ヮ◕)ﾉ$(YELLOW)*:･ﾟ✧"
					@echo ""

fclean: 	clean
					@${RM} $(NAME) $(textGen.replace)
					@printf "\r$(PURP)----- Deleting library ----\n"
					@echo 🗑 "\033[31mEverything is deleting now !"
					@printf "\r$(PURP)$(NAME) have been removed $(WHITE)¯\_$(PURP)(ツ)$(WHITE)_/¯$(RESET)\n"
					@echo ""

re:			fclean all

leak: 		all
					leaks -atExit -- ./$(NAME)

.PHONY: all clean fclean re
