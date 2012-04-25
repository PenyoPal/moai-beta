ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

USEFILE=
NAME=moai
# Extra include path for libfreetype and for target overrides and patches
EXTRA_INCVPATH+=$(QNX_TARGET)/usr/include/freetype2 \
	$(QNX_TARGET)/../target-override/usr/include
	
EXTRA_INCVPATH+=${QNX_TARGET}/usr/include/openssl \
    ${QNX_TARGET}/usr/include /Users/james/src/lua-qnx/Lua/src
EXTRA_INCVPATH+=$(PROJECT_ROOT)/3rdparty/box2d-2.2.1 \
  $(PROJECT_ROOT)/3rdparty/chipmunk-5.3.4/include $(PROJECT_ROOT)/3rdparty/tinyxml \
  $(PROJECT_ROOT)/3rdparty $(PROJECT_ROOT)/3rdparty/freetype-2.4.4/include \
  $(PROJECT_ROOT)/3rdparty/jansson-2.1/src $(PROJECT_ROOT)/3rdparty/jpeg-8c \
  $(PROJECT_ROOT)/3rdparty/ooid-0.99 $(PROJECT_ROOT)/src \
  $(PROJECT_ROOT)/3rdparty/untz/include $(PROJECT_ROOT)/3rdparty/tlsf-2.0
    
# Extra library search path for target overrides and patches
EXTRA_LIBVPATH+=$(QNX_TARGET)/../target-override/$(CPUVARDIR)/lib \
	$(QNX_TARGET)/../target-override/$(CPUVARDIR)/usr/lib \
	/Users/james/src/lua-qnx/Lua/Device-Debug /Users/james/src/lua-qnx/Lua/Device-Release \
	/Users/james/src/lua-qnx/Lua/Default
    
EXTRA_SRCVPATH+=$(PROJECT_ROOT)/src/moaicore $(PROJECT_ROOT)/src/uslscore \
    $(PROJECT_ROOT)/src/zipfs $(PROJECT_ROOT)/src/aku $(PROJECT_ROOT)/src/moaiext-untz

# Compiler options for enhanced security and recording the compiler options in release builds
CCFLAGS+=-fstack-protector-all -D_FORTIFY_SOURCE=2 \
	$(if $(filter g so shared,$(VARIANTS)),,-fPIE) \
	$(if $(filter g,$(VARIANTS)),,-frecord-gcc-switches)
	
CCFLAGS+=-DOPENSSL_NO_GMP -DOPENSSL_NO_JPAKE -DOPENSSL_NO_MD2 -DOPENSSL_NO_RC5 -DOPENSSL_NO_RFC3779 \
 -DOPENSSL_NO_STORE -DMOAI_OS_QNX
CCFLAGS+=-fno-rtti -O0 -Wmissing-field-initializers -Wreturn-type -Wnon-virtual-dtor \
 -Woverloaded-virtual -Wformat -Wno-missing-braces -Wparentheses -Wswitch -Wunused-function -Wunused-label \
 -Wunused-parameter -Wunused-variable -Wunused-value -Wno-uninitialized -Wunknown-pragmas -Wshadow \
 -Wsign-compare
CCFLAGS+=-lang-c++ 

# Linker options for enhanced security
LDFLAGS+=-Wl,-z,relro -Wl,-z,now $(if $(filter g so shared,$(VARIANTS)),,-pie)

# Add your required library names, here
LIBS+=lua GLESv2 ssl curl

include $(MKFILES_ROOT)/qmacros.mk

# Suppress the _g suffix from the debug variant
BUILDNAME=$(IMAGE_PREF_$(BUILD_TYPE))$(NAME)$(IMAGE_SUFF_$(BUILD_TYPE))
BUILDNAME_SAR=$(IMAGE_PREF_AR)$(NAME)S$(IMAGE_SUFF_AR)

include $(MKFILES_ROOT)/qtargets.mk

OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))
