.DEFAULT_GOAL := all

CXX      = c++
CC       = cc
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -MMD -MP
CFLAGS   = -O2 -MMD -MP

BINDIR   = bin
BUILDDIR = build

# Shared sources — compiled into objects linked by every test
LIB_SRCS = tx/01-whitening.cpp tx/02-header.cpp tx/03-crc.cpp tx/04-hamming_enc.cpp \
           tx/05-interleaver.cpp tx/06-gray_demap.cpp tx/07-modulate.cpp \
           rx/01-dewhitening.cpp rx/02-header_decoder.cpp rx/03-crc_verif.cpp rx/04-hamming_dec.cpp \
           rx/05-deinterleaver.cpp rx/06-gray_mapping.cpp \
           rx/07-frame_sync.cpp rx/08-fft_demod.cpp
LIB_OBJS = $(patsubst %.cpp,$(BUILDDIR)/%.o,$(LIB_SRCS))

COMMON_SRCS = common/kiss_fft.c
COMMON_OBJS = $(patsubst %.c,$(BUILDDIR)/%.o,$(COMMON_SRCS))

# Each tests/*.cpp becomes its own binary in bin/
TEST_SRCS = $(wildcard tests/*.cpp)
TEST_BINS = $(patsubst tests/%.cpp,$(BINDIR)/%,$(TEST_SRCS))
TEST_OBJS = $(patsubst tests/%.cpp,$(BUILDDIR)/tests/%.o,$(TEST_SRCS))

DEPS = $(LIB_OBJS:.o=.d) $(TEST_OBJS:.o=.d) $(COMMON_OBJS:.o=.d)

-include $(DEPS)

all: $(TEST_BINS)

$(BINDIR)/%: $(BUILDDIR)/tests/%.o $(LIB_OBJS) $(COMMON_OBJS) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILDDIR)/%.o: %.cpp | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BINDIR):
	mkdir -p $(BINDIR)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

run: all
	@for t in $(TEST_BINS); do echo "--- $$t ---"; ./$$t; done

clean:
	rm -rf $(BINDIR) $(BUILDDIR)

.PRECIOUS: $(BUILDDIR)/%.o
.PHONY: all run clean
