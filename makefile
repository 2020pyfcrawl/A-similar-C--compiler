NAME = final_ori12
# connected to COURCE_tmp
NAME1 = final_optwithC12
# connected to SOURCE_tmp1
NAME2 = final_optwithC2

#SOURCE = praser.cpp codegen1.cpp ParExp.cpp main.cpp token.cpp

#this is non-optimization one without C funtions embedded
SOURCE_tmp = praser.cpp codegen1.cpp ParExp1.cpp token.cpp

#this one is optimization one with 5 optimization optional in the main file \
 to run and the embeded output proto printi(int16 x) \
 we now make 2 optimization work which are the safest, \
 remember: optimization can bring bugs, so be careful.
SOURCE_tmp1 = praser.cpp codegen_opt.cpp ParExp1.cpp token.cpp

#SOURCE_tmp2 = praser.cpp codegen_C.cpp ParExp.cpp token.cpp

HPPS = util.hpp codegen1.hpp ParExp1.hpp
#OBJS = praser.o codegen1.o ParExp.o main.o token.o
#OBJS_tmp = praser.o codegen1.o ParExp.o token.o
#OBJS1 = praser.o  main.o token.o

CXXFLAGS = `llvm-config --cxxflags`
Arguments = `--ldflags --system-libs --libs all`

# use these two, use native if it has InitializeNativeTarget in main (in codegen_opt.cpp)
ALL_ARGU = `llvm-config --cxxflags --ldflags --system-libs --libs core`
OPT_ARGU = `llvm-config --cxxflags --ldflags --system-libs --libs native`

all : $(NAME1) 
	mv a.out $(NAME1)

#%.o: %.cpp util.hpp codegen1.hpp\
	g++ -c $(CXXFLAGS) -g -o $@ $< 

$(NAME): $(SOURCE_tmp) $(HPPS)
	clang++ -g -O3 $(SOURCE_tmp) $(ALL_ARGU)

$(NAME1): $(SOURCE_tmp1) $(HPPS)
	clang++ -g -O3 $(SOURCE_tmp1) $(OPT_ARGU)	

$(NAME2): $(SOURCE_tmp2) $(HPPS)
	clang++ -g -O3 $(SOURCE_tmp2) $(ALL_ARGU)	
#$(Arguments)


.PHONY: clean
clean:
	-rm -f *.o


#praser.o: praser.cpp util.hpp \
	clang++ -c praser.cpp \
\
codegen1.o: codegen1.cpp codegen1.hpp util.hpp\
	clang++ -c -O3 codegen1.cpp codegen1.h $(Arguments)\
\
ParExp.o: ParExp.cpp codegen1.hpp\
	clang++ -c ParExp.cpp $(Arguments)\
\
token.o: token.cpp util.hpp\
	clang++ -c token.cpp\
\
main.o: main.cpp util.hpp\
	clang++ -c main.cpp \