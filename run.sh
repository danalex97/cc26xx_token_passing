CONTIKI=../..
make
java -mx512m -jar $CONTIKI/tools/cooja/dist/cooja.jar -nogui=./star.csc -contiki=$CONTIKI
