SRCDIR = srcs/
OBJDIR = .obj/

SRCS = $(addprefix $(SRCDIR), \
		main.cpp \
		)
OBJS = $(SRCS:$(SRCDIR)%.cpp=$(OBJDIR)%.o)
DEPS = $(OBJS:.o=.d)

NAME = ircserv
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98
INC = -I includes/

all: $(NAME)
	@echo "\033[32mCompiled $(NAME)\033[0m"

$(NAME): $(OBJDIR) $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJDIR)%.o: $(SRCDIR)%.cpp
	@$(CC) $(CFLAGS) $(INC) -MMD -c $< -o $@

$(OBJDIR): 
	@mkdir -p $(OBJDIR)

-include $(DEPS)

clean:
	@rm -rf $(OBJDIR)
	@echo "\033[31mDeleted $(OBJDIR)\033[0m"

fclean: clean
	@rm -f $(NAME)
	@echo "\033[31mDeleted $(NAME)\033[0m"

re: fclean all

.PHONY: all clean fclean re