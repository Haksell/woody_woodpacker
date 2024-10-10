NAME := libft_elf.a

PATH_INCLUDES := inc
PATH_SRCS := src
PATH_OBJS := obj

INCLUDES := $(wildcard $(PATH_INCLUDES)/*.h)

SRCS := $(wildcard $(PATH_SRCS)/*.c)
OBJS := $(SRCS:$(PATH_SRCS)/%.c=$(PATH_OBJS)/%.o)

CC := gcc -std=gnu17 -Wall -Wextra -Werror -g3
CFLAGS := -I$(PATH_INCLUDES) -Ilibft/includes
AR := ar rc

LIBFT_PATH := libft
LIBFT_NAME := libft.a

RESET := \033[0m
RED := \033[1m\033[31m
GREEN := \033[1m\033[32m
PURPLE := \033[1m\033[35m

define remove_target
@if [ -e "$(1)" ]; then \
	rm -rf "$(1)"; \
	echo "$(RED)[X] $(1) removed.$(RESET)"; \
fi
endef

all: libft $(NAME)

libft:
	@echo "$(PURPLE)Building libft library...$(RESET)"
	@$(MAKE) --no-print-directory -C $(LIBFT_PATH)

$(NAME): $(OBJS)
	@$(AR) $(NAME) $(OBJS)
	@ranlib $(NAME)
	@echo "$(PURPLE)$@ is created.$(RESET)"

$(PATH_OBJS)/%.o: $(PATH_SRCS)/%.c $(INCLUDES)
	@mkdir -p $(PATH_OBJS)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "$(GREEN)+++ $@$(RESET)"

clean:
	@echo "$(PURPLE)Cleaning libft library...$(RESET)"
	@$(MAKE) --no-print-directory -C $(LIBFT_PATH) clean
	$(call remove_target,$(PATH_OBJS))

fclean: clean
	@echo "$(PURPLE)Fully cleaning libft library...$(RESET)"
	@$(MAKE) --no-print-directory -C $(LIBFT_PATH) fclean
	$(call remove_target,$(NAME))

re: fclean all

.PHONY: all clean fclean re libft
