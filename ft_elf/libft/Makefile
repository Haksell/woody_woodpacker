NAME := libft.a

PATH_INCLUDES := includes
PATH_SRCS := .
PATH_OBJS := obj

SRCS := $(wildcard */*.c */*/*.c)

OBJS := $(SRCS:%.c=$(PATH_OBJS)/%.o)

CC := gcc -std=gnu17 -Wall -Wextra -Werror
CFLAGS := -I$(PATH_INCLUDES)

AR := ar rc
RANLIB := ranlib

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

all: $(NAME)

$(NAME): $(OBJS)
	@$(AR) $(NAME) $(OBJS)
	@$(RANLIB) $(NAME)
	@echo "$(PURPLE)$@ is created.$(RESET)"

$(PATH_OBJS)/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "$(GREEN)+++ $@$(RESET)"

clean:
	@echo "$(PURPLE)Cleaning libft library...$(RESET)"
	$(call remove_target,$(PATH_OBJS))

fclean: clean
	$(call remove_target,$(NAME))

re: fclean all

.PHONY: all clean fclean re
