ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

USEFILE=
NAME=MoaiSample

# Extra include path for libfreetype and for target overrides and patches
EXTRA_INCVPATH+=$(QNX_TARGET)/../target-override/usr/include \
	$(PRODUCT_ROOT)/moai/src
	
EXTRA_INCVPATH+=$(PRODUCT_ROOT)/moai/3rdparty/box2d-2.2.1 \
  $(PRODUCT_ROOT)/moai/3rdparty/chipmunk-5.3.4/include $(PRODUCT_ROOT)/moai/3rdparty/tinyxml \
  $(PRODUCT_ROOT)/moai/3rdparty $(PRODUCT_ROOT)/moai/3rdparty/freetype-2.4.4/include \
  $(PRODUCT_ROOT)/moai/3rdparty/jansson-2.1/src $(PRODUCT_ROOT)/moai/3rdparty/jpeg-8c \
  $(PRODUCT_ROOT)/moai/3rdparty/ooid-0.99 $(PRODUCT_ROOT)/moai/src \
  $(PRODUCT_ROOT)/moai/3rdparty/untz/include $(PRODUCT_ROOT)/moai/3rdparty/tlsf-2.0 \
  $(PRODUCT_ROOT)/moai/3rdparty/lua-5.1.3/src

# Extra library search path for target overrides and patches
EXTRA_LIBVPATH+=$(QNX_TARGET)/../target-override/$(CPUVARDIR)/lib \
	$(QNX_TARGET)/../target-override/$(CPUVARDIR)/usr/lib \
	$(PRODUCT_ROOT)/moai/arm/so.le-v7-g $(PRODUCT_ROOT)/moai/arm/so.le-v7 \
	$(PRODUCT_ROOT)/moai/arm/a.le-v7-g $(PRODUCT_ROOT)/moai/arm/a.le-v7 

# Compiler options for enhanced security and recording the compiler options in release builds
CCFLAGS+=-fstack-protector-all -D_FORTIFY_SOURCE=2 \
	$(if $(filter g so shared,$(VARIANTS)),,-fPIE) \
	$(if $(filter g,$(VARIANTS)),,-frecord-gcc-switches)
	
CCFLAGS+=-DUSING_GL20 -DMOAI_OS_QNX

# Linker options for enhanced security
LDFLAGS+=-Wl,-z,relro -Wl,-z,now $(if $(filter g so shared,$(VARIANTS)),,-pie)

# Add your required library names, here
LIBS+=GLESv2 EGL screen png bps moai

include $(MKFILES_ROOT)/qmacros.mk

# Suppress the _g suffix from the debug variant
BUILDNAME=$(IMAGE_PREF_$(BUILD_TYPE))$(NAME)$(IMAGE_SUFF_$(BUILD_TYPE))

include $(MKFILES_ROOT)/qtargets.mk

OPTIMIZE_TYPE_g=none
OPTIMIZE_TYPE=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))
