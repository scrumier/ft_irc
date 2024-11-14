NAME = ircserv
SRCDIR = srcs/
OBJDIR = .obj/

SRCS = $(addprefix $(SRCDIR), \
		main.cpp \
		Server.cpp \
		handle_commands.cpp \
		Client.cpp \
		utils.cpp \
		Channel.cpp \
		)
OBJS = $(SRCS:$(SRCDIR)%.cpp=$(OBJDIR)%.o)
DEPS = $(OBJS:.o=.d)

BOTNAME = bot
BOTDIR = $(SRCDIR)bot/
BOTOBJDIR = .obj/bot/

BOTSRCS = $(addprefix $(BOTDIR), \
		main.cpp \
		IRCBot.cpp \
		bot.cpp \
		)
BOTOBJS = $(BOTSRCS:$(BOTDIR)%.cpp=$(BOTOBJDIR)%.o)
BOTDEPS = $(BOTOBJS:.o=.d)

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
INC = -I includes/
BOTINC = -I includes/ -I includes/bot/  # Fix here: separate the two include dirs

all: $(NAME)
	@echo "\033[32mCompiled $(NAME)\033[0m"
	@echo "\033[32mUsage: ./$(NAME) <port> <password>\033[0m"

$(NAME): $(OBJDIR) $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

$(OBJDIR)%.o: $(SRCDIR)%.cpp
	@$(CXX) $(CXXFLAGS) $(INC) -MMD -c $< -o $@

$(OBJDIR): 
	@mkdir -p $(OBJDIR)

-include $(DEPS)

bot: $(BOTOBJDIR) $(BOTOBJS)
	@$(CXX) $(CXXFLAGS) $(BOTOBJS) -o $(BOTNAME)  # Linking step: no $(BOTINC) here
	@echo "\033[32mCompiled $(BOTNAME)\033[0m"
	@echo "\033[32mUsage: ./$(BOTNAME) <server> <port> <nickname> <password> <channel>\033[0m"

$(BOTOBJDIR)%.o: $(BOTDIR)%.cpp
	@$(CXX) $(CXXFLAGS) $(BOTINC) -MMD -c $< -o $@  # $(BOTINC) used only during compilation

$(BOTOBJDIR):
	@mkdir -p $(BOTOBJDIR)

-include $(BOTDEPS)

clean:
	@rm -rf $(OBJDIR)
	@rm -rf $(BOTOBJDIR)
	@echo "\033[31mDeleted $(OBJDIR) and $(BOTOBJDIR)\033[0m"

fclean: clean
	@rm -f $(NAME)
	@rm -f $(BOTNAME)
	@echo "\033[31mDeleted $(NAME) and $(BOTNAME)\033[0m"

re: fclean all

.PHONY: all clean fclean re
