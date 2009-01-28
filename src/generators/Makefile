
.PHONY: all clean

all: ../inc_case_folding.hh ../inc_diacritic_folding.hh

generate: generate.cc
	c++ -W -Wall $< -o $@

../inc_case_folding.hh ../inc_diacritic_folding.hh: CaseFolding.txt DiacriticFolding.txt generate
	./generate

clean:
	rm -f generate
