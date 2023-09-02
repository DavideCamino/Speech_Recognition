all: dir bin/speech_recognition

bin/speech_recognition: obj/speech_recognition.o
	gcc -o bin/speech_recognition obj/speech_recognition.o

obj/speech_recognition.o: source/speech_recognition.c source/recognition.py
	gcc -o obj/speech_recognition.o -c source/speech_recognition.c

dir:
	source/create_dir.sh

clean:
	rm -rf bin
	rm -rf obj
