# decaf_compiler

A decaf compiler based on the CS143 starter code. In large part, it is a rewrite of https://github.com/manuzhang/decaf_compiler/tree/master/pp4/samples , however in pp3 a handful of minor errors have been fixed which were there in the referred source. Also, in pp4, the lexer I wrote did not give correct line numbers for mac-style files due to extra '\r''s added by it, thus I have directly used scanner.l and scanner.h from the specified source. Furthermore, the files which were supposed to be a part of the starter files of pp4 (which I didn't need to modify) according to the specification document were not downloading from the coursepage, hence I have used them from the source as well.
