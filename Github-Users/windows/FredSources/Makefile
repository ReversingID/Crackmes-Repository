
MODE?=Release
ARCH?=$(shell uname -m)

############################################
# Generic rules
############################################



all :   boost
	make -C LibTrace            MODE=$(MODE) ARCH=$(ARCH) all
	make -C LibOS               MODE=$(MODE) ARCH=$(ARCH) all
	make -C LibDisassemble      MODE=$(MODE) ARCH=$(ARCH) all
	make -C LibMemoryProtection MODE=$(MODE) ARCH=$(ARCH) all
	make -C CrackMe             MODE=$(MODE) ARCH=$(ARCH) all

boost : boost.update

boost.update : 
	git submodule init
	git submodule update
	cd boost; git submodule init
	cd boost ; git submodule update libs/core
	cd boost ; git submodule update libs/config
	cd boost ; git submodule update libs/msm
	cd boost ; git submodule update libs/mpl
	cd boost ; git submodule update libs/preprocessor
	cd boost ; git submodule update libs/type_traits
	cd boost ; git submodule update libs/fusion
	cd boost ; git submodule update libs/static_assert
	cd boost ; git submodule update libs/utility
	cd boost ; git submodule update libs/assert
	cd boost ; git submodule update libs/bind
	cd boost ; git submodule update libs/function
	cd boost ; git submodule update libs/throw_exception
	cd boost ; git submodule update libs/integer
	cd boost ; git submodule update libs/type_index
	cd boost ; git submodule update libs/container_hash
	cd boost ; git submodule update libs/detail
	cd boost ; git submodule update libs/move
	cd boost ; git submodule update libs/serialization
	cd boost ; git submodule update libs/parameter
	cd boost ; git submodule update libs/proto
	cd boost ; git submodule update libs/typeof
	cd boost ; git submodule update libs/tuple
	cd boost ; git submodule update libs/any
	cd boost ; git submodule update libs/predef
	touch $@

############################################
# Clean rules
############################################
clean:
	make -C LibTrace            MODE=$(MODE) ARCH=$(ARCH) clean
	make -C LibOS               MODE=$(MODE) ARCH=$(ARCH) clean
	make -C LibDisassemble      MODE=$(MODE) ARCH=$(ARCH) clean
	make -C LibMemoryProtection MODE=$(MODE) ARCH=$(ARCH) clean
	make -C CrackMe             MODE=$(MODE) ARCH=$(ARCH) clean

mrproper: clean 
	rm -fr Release Debug $(BOOST)
