SRC = ps:sprsrc/
OBJ = ps:sprobj/
RUN = ps:sprsrc/
RAD = ps:pw/

.c.o:
  @lc -D -o$(OBJ) $*

$(RUN)Sprite: $(OBJ)sprite.o
  @blink <with <
FROM lib:c.o $(OBJ)sprite.o
TO   $(RUN)Sprite
LIB  lib:lc.lib+lib:amiga.lib
<

$(OBJ)sprite.o: $(SRC)sprite.c

run: $(RUN)sprite
   sprite

backup:
   copy $(SRC)* Work:Backups/PacMan/SprSrc
