NAME := woody_woodpacker

PATH_INCLUDES := includes
PATH_SRCS := srcs
PATH_OBJS := objs

INCLUDES := $(wildcard $(PATH_INCLUDES)/*.h)

SRCS_C := $(wildcard $(PATH_SRCS)/*.c)
OBJS_C := $(SRCS_C:$(PATH_SRCS)/%.c=$(PATH_OBJS)/%.o)

SRCS_ASM := $(wildcard $(PATH_SRCS)/*.s)
OBJS_ASM := $(SRCS_ASM:$(PATH_SRCS)/%.s=$(PATH_OBJS)/%.o)

CC := cc -std=gnu17 -Wall -Wextra -Werror -g3
CFLAGS := -I$(PATH_INCLUDES) -Ift_elf/inc -Ift_elf/libft/includes
LDFLAGS := -Lft_elf -lft_elf -Lft_elf/libft -lft

FT_ELF_PATH := ft_elf

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

all: ft_elf $(NAME)

ft_elf:
	@echo "$(PURPLE)Building ft_elf library...$(RESET)"
	@$(MAKE) --no-print-directory -C $(FT_ELF_PATH)

$(NAME): $(OBJS_C) $(OBJS_ASM)
	@$(CC) $(OBJS_C) $(OBJS_ASM) $(LDFLAGS) -o $@
	@echo "$(PURPLE)$@ is compiled.$(RESET)"

$(PATH_OBJS)/%.o: $(PATH_SRCS)/%.c $(INCLUDES)
	@mkdir -p $(PATH_OBJS)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "$(GREEN)+++ $@$(RESET)"

$(PATH_OBJS)/%.o: $(PATH_SRCS)/%.s
	@mkdir -p $(PATH_OBJS)
	@nasm -f elf64 $< -o $@
	@echo "$(GREEN)+++ $@$(RESET)"

clean:
	@echo "$(PURPLE)Cleaning ft_elf library...$(RESET)"
	@$(MAKE) --no-print-directory -C $(FT_ELF_PATH) clean
	$(call remove_target,$(PATH_OBJS))

fclean: clean
	@echo "$(PURPLE)Fully cleaning ft_elf library...$(RESET)"
	@$(MAKE) --no-print-directory -C $(FT_ELF_PATH) fclean
	$(call remove_target,$(NAME))

re: fclean all

.PHONY: all clean fclean re ft_elf
