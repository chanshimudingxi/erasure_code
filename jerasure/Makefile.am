AM_CPPFLAGS = -I gf-complete/include -I jerasure/include/
AM_CFLAGS = -O3 $(SIMD_FLAGS) -fPIC -Wsign-compare


noinst_LTLIBRARIES = libJerasure.a
libJerasure_a_SOURCES = \
					gf-complete/src/gf.c gf-complete/src/gf_method.c gf-complete/src/gf_wgen.c \
					gf-complete/src/gf_w4.c gf-complete/src/gf_w8.c gf-complete/src/gf_w16.c gf-complete/src/gf_w32.c \
          gf-complete/src/gf_w64.c gf-complete/src/gf_w128.c gf-complete/src/gf_rand.c gf-complete/src/gf_general.c

if HAVE_NEON
libJerasure_a_SOURCES += gf-complete/src/neon/neon/gf_w4_neon.c  \
                             gf-complete/src/neon/neon/gf_w8_neon.c  \
                             gf-complete/src/neon/neon/gf_w16_neon.c \
                             gf-complete/src/neon/neon/gf_w32_neon.c \
                             gf-complete/src/neon/neon/gf_w64_neon.c
endif

libJerasure_a_SOURCES +=  \
				jerasure/src/galois.c jerasure/src/jerasure.c jerasure/src/reed_sol.c jerasure/src/cauchy.c jerasure/src/liberation.c
libJerasure_a_LDFLAGS = -version-info 2:0:0