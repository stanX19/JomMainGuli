NPROC		:= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
MAKEFLAGS	+= -j$(NPROC)

SRCDIR		= srcs
SRCS		:= $(shell find $(SRCDIR) -name '*.cpp')

OBJDIR		= objs
OBJS		= $(subst $(SRCDIR)/,$(OBJDIR)/,$(subst .cpp,.o,$(SRCS)))
OBJDIRS		= $(sort $(dir $(OBJS)))

MAINCPP		= main/main.cpp

TESTDIR		= tests
TESTBINDIR	= objs/test_bin
TESTOBJDIR	= objs/test
TEST_FRAMEWORK_DIR = includes/catch2
CATCH_SRC	= $(TEST_FRAMEWORK_DIR)/catch_amalgamated.cpp
CATCH_OBJ	= objs/catch2/catch_amalgamated.o
CATCH_DEP	= $(CATCH_OBJ:.o=.d)

UNIT_TEST_SRCS	:= $(shell find $(TESTDIR)/unit -name '*.cpp' 2>/dev/null)
INTEGRATION_TEST_SRCS := $(shell find $(TESTDIR)/integration -name '*.cpp' 2>/dev/null)
SMOKE_TEST_SRCS	:= $(shell find $(TESTDIR)/smoke -name '*.cpp' 2>/dev/null)
MANUAL_TEST_SRCS := $(shell find $(TESTDIR)/manual -name '*.cpp' 2>/dev/null)

UNIT_TEST_OBJS	:= $(patsubst $(TESTDIR)/unit/%.cpp,$(TESTOBJDIR)/unit/%.o,$(UNIT_TEST_SRCS))
INTEGRATION_TEST_OBJS := $(patsubst $(TESTDIR)/integration/%.cpp,$(TESTOBJDIR)/integration/%.o,$(INTEGRATION_TEST_SRCS))
SMOKE_TEST_OBJS	:= $(patsubst $(TESTDIR)/smoke/%.cpp,$(TESTOBJDIR)/smoke/%.o,$(SMOKE_TEST_SRCS))
MANUAL_TEST_BINS := $(patsubst $(TESTDIR)/manual/%.cpp,$(TESTBINDIR)/manual/%,$(MANUAL_TEST_SRCS))

UNIT_TEST_BIN	= $(TESTBINDIR)/unit_tests
INTEGRATION_TEST_BIN = $(TESTBINDIR)/integration_tests
SMOKE_TEST_BIN	= $(TESTBINDIR)/smoke_tests
TEST_ARCHIVE	= $(OBJDIR)/libcodesfaires.a
TEST_FLAGS	?= -r compact
INTEGRATION_BIN_TARGET := $(if $(strip $(INTEGRATION_TEST_SRCS)),$(INTEGRATION_TEST_BIN),)
TEST_OBJDIRS	:= $(sort $(dir $(CATCH_OBJ) $(UNIT_TEST_OBJS) $(INTEGRATION_TEST_OBJS) $(SMOKE_TEST_OBJS) $(MANUAL_TEST_BINS) $(TEST_ARCHIVE)))

WSL_D3D12_AVAILABLE := $(shell if [ -e /dev/dxg ] && [ -f /usr/lib/x86_64-linux-gnu/dri/d3d12_dri.so ]; then echo 1; fi)
RUN_DRIVER_ENV :=
ifeq ($(origin GALLIUM_DRIVER),undefined)
ifneq ($(WSL_D3D12_AVAILABLE),)
RUN_DRIVER_ENV := GALLIUM_DRIVER=d3d12
endif
endif

RAYLIB_DIR      = includes/raylib
RAYLIB_REPO_DIR = $(RAYLIB_DIR)/repo
RAYLIB_SRC_DIR  = $(RAYLIB_REPO_DIR)/src
RAYLIB_LIB      = $(RAYLIB_DIR)/libraylib.a

INCLUDE_DIR	= includes/raylib includes/entt includes
HEADER_DIR	= headers
HEADERS		:= $(shell find $(HEADER_DIR) -name '*.hpp')
HEADERS_INC	= $(addprefix -I,$(sort $(dir $(HEADERS))) $(INCLUDE_DIR))

IFLAGS		:= -I. $(HEADERS_INC)

ifneq ($(shell command -v mold 2>/dev/null),)
    LINKER := -fuse-ld=mold
else ifneq ($(shell command -v lld 2>/dev/null),)
    LINKER := -fuse-ld=lld
else
    LINKER := 
endif

CC_BASE		= g++
ifneq ($(shell command -v ccache 2>/dev/null),)
CC			= ccache $(CC_BASE)
else
CC			= $(CC_BASE)
endif

LFLAGS		= $(LINKER) -Lincludes/raylib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
CFLAGS		= -std=c++20 -Wall -Wextra -Werror -std=c++17 -MMD -MP -fmax-errors=3
AR			= ar -rcs
RM			= rm -rf
UP			= \033[1A
FLUSH		= \033[2K

PCH_HEADER	= headers/includes.hpp
PCH			= headers/includes.hpp.gch
PCH_FLAG	= -include $(PCH_HEADER)
PCH_DEPS	= $(PCH:.gch=.d)

NAME		= codesfaires
ARGV		= 

run: all
	$(RUN_DRIVER_ENV) ./$(NAME) $(ARGV)

$(NAME): $(OBJDIRS) $(PCH) $(OBJS) $(MAINCPP) $(RAYLIB_LIB)
	$(CC) $(CFLAGS) $(PCH_FLAG) $(OBJS) $(MAINCPP) $(IFLAGS) $(LFLAGS) -o $(NAME)

all: $(NAME)

test: test-unit test-integration test-smoke

test-unit: $(UNIT_TEST_BIN)
	@echo "Running $<..."
	@./$< $(TEST_FLAGS)

test-integration:
	@if [ -z "$(strip $(INTEGRATION_TEST_SRCS))" ]; then \
		echo "No integration tests found."; \
	else \
		$(MAKE) --no-print-directory $(INTEGRATION_TEST_BIN); \
		echo "Running $(INTEGRATION_TEST_BIN)..."; \
		./$(INTEGRATION_TEST_BIN) $(TEST_FLAGS); \
	fi

test-smoke:
	@if [ -z "$(strip $(SMOKE_TEST_SRCS))" ]; then \
		echo "No smoke tests found."; \
	else \
		$(MAKE) --no-print-directory $(SMOKE_TEST_BIN); \
		echo "Running $(SMOKE_TEST_BIN)..."; \
		./$(SMOKE_TEST_BIN) $(TEST_FLAGS); \
	fi

test-manual-bin: $(MANUAL_TEST_BINS)

test-manual:
	@if [ -z "$(TEST)" ]; then \
		echo "Usage: make test-manual TEST=<basename>"; \
		echo "Available manual tests:"; \
		for test_src in $(MANUAL_TEST_SRCS); do basename "$$test_src" .cpp; done; \
		exit 2; \
	fi
	@if [ ! -f "$(TESTDIR)/manual/$(TEST).cpp" ]; then \
		echo "Unknown manual test: $(TEST)"; \
		echo "Run 'make test-manual' to list available tests."; \
		exit 2; \
	fi
	@$(MAKE) --no-print-directory $(TESTBINDIR)/manual/$(TEST)
	@echo "Running $(TESTBINDIR)/manual/$(TEST)..."
	@./$(TESTBINDIR)/manual/$(TEST)

all_test: test

testbin: $(UNIT_TEST_BIN) $(INTEGRATION_BIN_TARGET) $(SMOKE_TEST_BIN)

$(OBJDIRS) $(TEST_OBJDIRS) $(TESTBINDIR) $(TESTBINDIR)/manual:
	mkdir -p $@
	@echo "$(UP)$(FLUSH)$(UP)$(FLUSH)$(UP)"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(PCH) $(RAYLIB_LIB)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(IFLAGS) $(PCH_FLAG) -c $< -o $@

$(PCH): $(PCH_HEADER) $(RAYLIB_LIB)
	@mkdir -p $(dir $@)
	$(CC) $(filter-out -Werror,$(CFLAGS)) $(IFLAGS) -x c++-header $< -o $@

$(CATCH_OBJ): $(CATCH_SRC) | $(TEST_OBJDIRS)
	$(CC) $(CFLAGS) -MMD -MP -I$(TEST_FRAMEWORK_DIR) -c $< -o $@

$(TEST_ARCHIVE): $(OBJS) | $(TEST_OBJDIRS)
	$(AR) $@ $(OBJS)

$(TESTOBJDIR)/unit/%.o: $(TESTDIR)/unit/%.cpp $(PCH) $(RAYLIB_LIB)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(IFLAGS) $(PCH_FLAG) -c $< -o $@

$(TESTOBJDIR)/integration/%.o: $(TESTDIR)/integration/%.cpp $(PCH) $(RAYLIB_LIB)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(IFLAGS) $(PCH_FLAG) -c $< -o $@

$(TESTOBJDIR)/smoke/%.o: $(TESTDIR)/smoke/%.cpp $(PCH) $(RAYLIB_LIB)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(IFLAGS) $(PCH_FLAG) -c $< -o $@

$(UNIT_TEST_BIN): $(UNIT_TEST_OBJS) $(CATCH_OBJ) $(TEST_ARCHIVE) | $(TESTBINDIR)
	$(CC) $(CFLAGS) $(UNIT_TEST_OBJS) $(CATCH_OBJ) $(TEST_ARCHIVE) $(IFLAGS) $(LFLAGS) -o $@

$(INTEGRATION_TEST_BIN): $(INTEGRATION_TEST_OBJS) $(CATCH_OBJ) $(TEST_ARCHIVE) | $(TESTBINDIR)
	$(CC) $(CFLAGS) $(INTEGRATION_TEST_OBJS) $(CATCH_OBJ) $(TEST_ARCHIVE) $(IFLAGS) $(LFLAGS) -o $@

$(SMOKE_TEST_BIN): $(SMOKE_TEST_OBJS) $(CATCH_OBJ) | $(TESTBINDIR)
	$(CC) $(CFLAGS) $(SMOKE_TEST_OBJS) $(CATCH_OBJ) -o $@

$(TESTBINDIR)/manual/%: $(TESTDIR)/manual/%.cpp $(OBJS) $(PCH) | $(TESTBINDIR)/manual
	$(CC) $(CFLAGS) $(IFLAGS) $(PCH_FLAG) $< $(OBJS) $(LFLAGS) -o $@

$(RAYLIB_REPO_DIR):
	@echo "==> Cloning Raylib repository..."
	git clone --depth 1 https://github.com/raysan5/raylib.git $(RAYLIB_REPO_DIR)

$(RAYLIB_SRC_DIR)/libraylib.a: | $(RAYLIB_REPO_DIR)
	@echo "==> Compiling Raylib static library..."
	$(MAKE) -C $(RAYLIB_SRC_DIR) PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=STATIC

$(RAYLIB_LIB): $(RAYLIB_SRC_DIR)/libraylib.a
	@echo "==> Updating Raylib headers and static archive..."
	@cp -f $(RAYLIB_SRC_DIR)/libraylib.a $(RAYLIB_LIB)
	@cp -f $(RAYLIB_SRC_DIR)/raylib.h $(RAYLIB_SRC_DIR)/raymath.h $(RAYLIB_SRC_DIR)/rcamera.h $(RAYLIB_SRC_DIR)/rlgl.h $(RAYLIB_DIR)/

setup: $(RAYLIB_LIB)

check:
	valgrind --track-origins=yes --leak-check=full ./$(NAME) $(ARGV) 2>&1 | tail -n 200

clean:
	@$(RM) $(OBJS) $(OBJS:.o=.d) $(PCH) $(PCH_DEPS) $(CATCH_OBJ) $(CATCH_DEP) $(TEST_ARCHIVE) \
		$(UNIT_TEST_OBJS) $(INTEGRATION_TEST_OBJS) $(SMOKE_TEST_OBJS) \
		$(UNIT_TEST_OBJS:.o=.d) $(INTEGRATION_TEST_OBJS:.o=.d) $(SMOKE_TEST_OBJS:.o=.d) \
		$(UNIT_TEST_BIN) $(INTEGRATION_TEST_BIN) $(SMOKE_TEST_BIN) $(MANUAL_TEST_BINS)

fclean: clean
	@$(RM) $(NAME) codesfaires.d
	@$(RM) $(OBJDIRS) $(TESTOBJDIR) $(TESTBINDIR)
	@$(RM) ./a.out

re:
	@$(MAKE) fclean
	@$(MAKE) all

.PHONY: all clean fclean re run test test-unit test-integration test-smoke test-manual test-manual-bin testbin all_test setup
-include $(OBJS:.o=.d) $(PCH_DEPS) $(CATCH_DEP)
