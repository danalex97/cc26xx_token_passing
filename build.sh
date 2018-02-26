scp -r . ad5915@shell1.doc.ic.ac.uk:~/contiki/examples/pervasive_cw2
ssh ad5915@sheel1.doc.ic.ac.uk <<-'ENDSSH'
  ssh ad5915@matrix20.doc.ic.ac.uk <<-'END2'
    cd contiki/examples/pervasive_cw2
    make TARGET=srf06-cc26xx BOARD=sensortag/cc2650 CPU_FAMILY=cc26xx > log.txt
  END2
ENDSSH
cp ad5915@shell1.doc.ic.ac.uk:~/contiki/examples/pervasive_cw2/log.txt ./log.txt
scp ad5915@shell1.doc.ic.ac.uk:~/contiki/examples/pervasive_cw2/sender.bin ./sender.bin
scp ad5915@shell1.doc.ic.ac.uk:~/contiki/examples/pervasive_cw2/base_station.bin ./base_station.bin
