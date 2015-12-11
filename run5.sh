./start 3;
sleep 1;
./producer 1 10 &
./consumer 1    & 
./consumer 2    &
./consumer 3    &
./consumer 4    &
./consumer 5    &
./consumer 6    &
./consumer 7    &
./consumer 8    &
./consumer 9    &
./consumer 10   &
./consumer 11
echo "Test finished."
