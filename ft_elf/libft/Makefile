NAME = libft.a
CC = gcc
CFLAGS = -Iincludes -Wall -Werror -Wextra

SRCS = $(wildcard */*.c) 
SRCS += $(wildcard */*/*.c)

OBJS = $(SRCS:.c=.o)

all: $(NAME)
	
%.o: %.c
	@$(CC) $(CFLAGS) -c -o $@ $<

$(NAME): $(OBJS)
	@ar rc $(NAME) $(OBJS)
	@ranlib $(NAME)

clean:
	@rm -f $(OBJS)
	@rm -f .*.swp
	@rm -f */.*.swp
	@rm -f */*/.*.swp
	@rm -f a.out

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re bonus
