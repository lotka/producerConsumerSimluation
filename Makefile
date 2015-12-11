CC=g++ -Wall

all: producer consumer start reader

start: start.o helper.o
	$(CC) -o start start.o helper.o

start.o: start.cc helper.cc helper.h
	$(CC) -c start.cc helper.cc

producer: producer.o helper.o
	$(CC) -o producer producer.o helper.o

producer.o: producer.cc helper.cc helper.h
	$(CC) -c producer.cc helper.cc

consumer: consumer.o helper.o
	$(CC) -o consumer consumer.o helper.o

consumer.o: consumer.cc helper.cc helper.h
	$(CC) -c consumer.cc helper.cc

reader: reader.o helper.o
	$(CC) -o reader reader.o helper.o

reader.o: reader.cc helper.cc helper.h
	$(CC) -c reader.cc helper.cc

tidy:
	rm -f *.o core

clean:
	rm -f start producer consumer *.o core
