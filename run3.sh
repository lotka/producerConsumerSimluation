./start 1
sleep 1
./producer 1 1 	&
./consumer 1	&
./producer 2 1	&
./consumer 1 	
echo "Test finished."
