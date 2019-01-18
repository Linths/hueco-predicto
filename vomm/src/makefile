all: clean build test

build:
	find . -name \*.java -print > file.list
	javac -cp . @file.list	

clean:
	find vmm -name \*.class -exec rm -f {} \;
	rm -f *.class
	rm -f *.ser

test:
	bash test.sh
