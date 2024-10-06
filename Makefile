NAME := woody_woodpacker

PATH_SRCS := srcs
PATH_OBJS := objs
PATH_INCLUDES := includes

INCLUDES := $(wildcard $(PATH_INCLUDES)/*.h)
SRCS := $(wildcard $(PATH_SRCS)/*.c)
OBJS := $(SRCS:$(PATH_SRCS)/%.c=$(PATH_OBJS)/%.o)

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

$(PATH_OBJS):
	@mkdir -p $(sort $(dir $(OBJS)))

$(OBJS): $(PATH_OBJS)/%.o: $(PATH_SRCS)/%.c $(INCLUDES)
	@mkdir -p $(PATH_OBJS)
	@$(CC) -c $< -o $@
	@echo "$(GREEN)+++ $@$(RESET)"

$(NAME): $(OBJS)
	@$(CC) $(OBJS) -o $@
	@echo "$(PURPLE)$@ is compiled.$(RESET)"

clean:
	$(call remove_target,$(PATH_OBJS))

fclean: clean
	$(call remove_target,$(NAME))

re: fclean
	@$(MAKE) -s $(NAME)

run: all
	@./$(NAME)

rerun: re
	@./$(NAME)

resources:
	rm -rf resources
	wget https://cdn.intra.42.fr/document/document/15758/resources.tgz
	tar xvf resources.tgz
	rm resources.tgz

.PHONY: all clean fclean re run rerun resources