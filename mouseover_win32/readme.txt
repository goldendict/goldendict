These files were taken straight from StarDict 3.0.1. They didn't seem to
bear any copyright notices, and as such, no new initial notices were added.
Some global ids were changed to make sure that StarDict and GoldenDict
wouldn't interfere with each other when running simultaneously. Also,
Firefox 3 was made to work by implementing ETO_GLYPH_INDEX in ExtTextOutW()
function.

The Makefile is made for mingw32 cross-compilation, and is completely separate
from the main program build.
