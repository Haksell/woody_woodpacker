NAME := woody_woodpacker

PATH_INCLUDES := includes
PATH_SRCS := srcs
PATH_OBJS := objs

INCLUDES := $(wildcard $(PATH_INCLUDES)/*.h)

SRCS_C := $(wildcard $(PATH_SRCS)/*.c)
OBJS_C := $(SRCS_C:$(PATH_SRCS)/%.c=$(PATH_OBJS)/%.o)

SRCS_ASM := $(wildcard $(PATH_SRCS)/*.s)
OBJS_ASM := $(SRCS_ASM:$(PATH_SRCS)/%.s=$(PATH_OBJS)/%.o)

CC := cc -std=gnu17 -Wall -Wextra -Werror -g3 -I$(PATH_INCLUDES)

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

$(NAME): $(OBJS_C) $(OBJS_ASM)
	@$(CC) $(OBJS_C) $(OBJS_ASM) -o $@
	@echo "$(PURPLE)$@ is compiled.$(RESET)"

$(OBJS_C): $(PATH_OBJS)/%.o: $(PATH_SRCS)/%.c $(INCLUDES)
	@mkdir -p $(PATH_OBJS)
	@$(CC) -c $< -o $@
	@echo "$(GREEN)+++ $@$(RESET)"

$(OBJS_ASM): $(PATH_OBJS)/%.o: $(PATH_SRCS)/%.s
	@mkdir -p $(PATH_OBJS)
	@nasm -f elf64 $< -o $@
	@echo "$(GREEN)+++ $@$(RESET)"

clean:
	$(call remove_target,$(PATH_OBJS))

fclean: clean
	$(call remove_target,$(NAME))

re: fclean
	@$(MAKE) -s $(NAME)

.PHONY: all clean fclean re