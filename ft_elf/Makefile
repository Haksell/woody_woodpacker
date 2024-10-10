NAME = libft_elf.a
CC = gcc
CFLAGS = -Wall -Werror -Wextra -Ilibft/includes -Iinc
SRC_PATH=src/
SRCS = $(SRC_PATH)elf.c
LIBFT_PATH = libft/
LIBFT_NAME = libft.a
OBJ/OBJECTS		=	$(patsubst %.c, obj/%.o, $(SRCS))

all: $(NAME)

obj/%.o: %.c Makefile | obj
	@$(CC) $(CFLAGS) -c -o $@ $<

$(NAME): $(LIBFT_NAME) $(OBJ/OBJECTS)
	@ar rc $(NAME) $(OBJ/OBJECTS)
	@ranlib $(NAME)


$(LIBFT_NAME):
	$(MAKE) -C $(LIBFT_PATH) && cp -f $(LIBFT_PATH)/$(LIBFT_NAME) $(shell pwd)

obj:
	mkdir -p obj/$(SRC_PATH)

clean:
	$(MAKE) clean -C $(LIBFT_PATH)
	rm -rf $(LIBFT_NAME)
	rm -rf obj
	rm -f .*.swp
	rm -rf $(NAME).dSYM

fclean: clean
	$(MAKE) fclean -C $(LIBFT_PATH)
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re bonus