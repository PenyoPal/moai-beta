ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

USEFILE=
NAME=moai

EXCLUDE_OBJS+=AKU-fmod.o AKU-debugger.o AKU-untz.o
EXCLUDE_OBJS+=ftbase.o autofit.o bdf.o sfnt.o cff.o type42.o type1.o psaux.o \
  pcf.o ftcache.o ftzopen.o truetype.o ttsbit0.o pshinter.o pfr.o smooth.o \
  psnames.o raster.o type1cid.o ls_sqlite3.o luasql.o
EXTRA_OBJS+=inftrees.o

	
EXTRA_INCVPATH+=${QNX_TARGET}/usr/include/openssl $(PROJECT_ROOT)/3rdparty $(PROJECT_ROOT)
EXTRA_INCVPATH+=$(PROJECT_ROOT)/3rdparty/box2d-2.2.1 \
  $(PROJECT_ROOT)/3rdparty/chipmunk-5.3.4/include $(PROJECT_ROOT)/3rdparty/tinyxml \
  $(PROJECT_ROOT)/3rdparty/jansson-2.1/src $(PROJECT_ROOT)/3rdparty/jpeg-8c \
  $(PROJECT_ROOT)/3rdparty/ooid-0.99 $(PROJECT_ROOT)/src \
  $(PROJECT_ROOT)/3rdparty/untz/include $(PROJECT_ROOT)/3rdparty/tlsf-2.0 \
  $(PROJECT_ROOT)/3rdparty/libogg-1.2.2/include \
  $(PROJECT_ROOT)/3rdparty/libvorbis-1.3.2/include \
  $(PROJECT_ROOT)/3rdparty/chipmunk-5.3.4/include/chipmunk \
  $(PROJECT_ROOT)/3rdparty/lua-5.1.3/src $(PROJECT_ROOT)/src/lua-headers \
  $(PROJECT_ROOT)/3rdparty/luasocket-2.0.2/src $(PROJECT_ROOT)/luacurl-1.2.1 \
  $(PROJECT_ROOT)/3rdparty/luafilesystem-1.5.0/src \
  $(PROJECT_ROOT)/3rdparty/luacrypto-0.2.0/src \
  $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/include  $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/src/gzip
EXTRA_INCVPATH+= ${QNX_TARGET}/usr/include $(QNX_TARGET)/../target-override/usr/include
  
# Extra library search path for target overrides and patches
EXTRA_LIBVPATH+=$(QNX_TARGET)/../target-override/$(CPUVARDIR)/lib \
	$(QNX_TARGET)/../target-override/$(CPUVARDIR)/usr/lib                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         
    
EXTRA_SRCVPATH+=$(PROJECT_ROOT)/src/moaicore $(PROJECT_ROOT)/src/uslscore \
    $(PROJECT_ROOT)/src/zipfs $(PROJECT_ROOT)/src/aku $(PROJECT_ROOT)/src/lua-headers \
    $(PROJECT_ROOT)/3rdparty/box2d-2.2.1/Box2D/Collision \
    $(PROJECT_ROOT)/3rdparty/box2d-2.2.1/Box2D/Collision/Shapes \
    $(PROJECT_ROOT)/3rdparty/box2d-2.2.1/Box2D/Common \
    $(PROJECT_ROOT)/3rdparty/box2d-2.2.1/Box2D/Dynamics \
    $(PROJECT_ROOT)/3rdparty/box2d-2.2.1/Box2D/Dynamics/Joints \
    $(PROJECT_ROOT)/3rdparty/box2d-2.2.1/Box2D/Dynamics/Contacts \
    $(PROJECT_ROOT)/3rdparty/box2d-2.2.1/Box2D/Rope \
    $(PROJECT_ROOT)/3rdparty/tinyxml $(PROJECT_ROOT)/3rdparty/contrib \
    $(PROJECT_ROOT)/3rdparty/jansson-2.1/src $(PROJECT_ROOT)/3rdparty/tlsf-2.0 \
    $(PROJECT_ROOT)/3rdparty/chipmunk-5.3.4/src $(PROJECT_ROOT)/3rdparty/chipmunk-5.3.4/src/constraints \
    $(PROJECT_ROOT)/3rdparty/luacurl-1.2.1 $(PROJECT_ROOT)/3rdparty/lua-5.1.3/src  \
    $(PROJECT_ROOT)/3rdparty/luasocket-2.0.2/src $(PROJECT_ROOT)/3rdparty/luafilesystem-1.5.0/src \
    $(PROJECT_ROOT)/3rdparty/luacrypto-0.2.0/src \
    $(PROJECT_ROOT)/3rdparty/zlib-1.2.3 
# $(PROJECT_ROOT)/3rdparty/luasql-2.2.0/src 
EXTRA_SRCVPATH+=$(PROJECT_ROOT)/3rdparty/freetype-2.4.4/src/base $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/src/autofit/ \
    $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/src/bdf $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/src/cff \
    $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/src/cache $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/src/gzip \
    $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/src/pcf $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/src/truetype \
    $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/src/smooth $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/src/pfr \
    $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/src/psaux $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/src/pshinter \
    $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/src/psnames $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/src/raster \
    $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/src/sfnt $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/src/type1 \
    $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/src/cid $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/src/type42 \
    $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/src/lzw $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/src/winfonts
SRCS+=$(PROJECT_ROOT)/3rdparty/luasql-2.2.0/src/ls_sqlite3.c $(PROJECT_ROOT)/3rdparty/luasql-2.2.0/src/luasql.c 
# Compiler options for enhanced security and recording the compiler options in release builds
CCFLAGS+=-fstack-protector-all -D_FORTIFY_SOURCE=2 \
	$(if $(filter g so shared,$(VARIANTS)),,-fPIE) \
	$(if $(filter g,$(VARIANTS)),,-frecord-gcc-switches)
	
CCFLAGS+=-DOPENSSL_NO_GMP -DOPENSSL_NO_JPAKE -DOPENSSL_NO_MD2 -DOPENSSL_NO_RC5 -DOPENSSL_NO_RFC3779 \
 -DOPENSSL_NO_STORE -DMOAI_OS_QNX -DPOSIX -DHAVE_MEMMOVE -DLUA_USE_POSIX
CCFLAGS+=-fno-rtti -O0 -Wmissing-field-initializers -Wreturn-type -Wnon-virtual-dtor \
 -Woverloaded-virtual -Wformat -Wno-missing-braces -Wparentheses -Wswitch -Wunused-function -Wunused-label \
 -Wunused-parameter -Wunused-variable -Wunused-value -Wno-uninitialized -Wunknown-pragmas -Wshadow \
 -Wsign-compare -Wno-bad-function-cast 
CCFLAGS+=-MMD -MP -D_GLOBAL_USING -DFT2_BUILD_LIBRARY -UFT_CONFIG_OPTION_PIC -UNO_INFLATE_MASK

# Linker options for enhanced security
LDFLAGS+=-Wl,-z,relro -Wl,-z,now $(if $(filter g so shared,$(VARIANTS)),,-pie)

# Add your required library names, here
LIBS+=m expat socket EGL GLESv2 ssl curl jpeg sqlite3

include $(MKFILES_ROOT)/qmacros.mk

# Suppress the _g suffix from the debug variant
BUILDNAME=$(IMAGE_PREF_$(BUILD_TYPE))$(NAME)$(IMAGE_SUFF_$(BUILD_TYPE))
BUILDNAME_SAR=$(IMAGE_PREF_AR)$(NAME)S$(IMAGE_SUFF_AR)

include $(MKFILES_ROOT)/qtargets.mk

OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))
