all: try human

try:
	ruby ../../../data/sb_ucd_anal.rb < ../../../data/strangebeta_user_climb_data_20100208.txt > input.txt
	cat input.txt | cut -f 6 -d '|' | ./run_parse > output.txt
	ruby symbolize.rb < output.txt > symbolic.txt
	grep "No parse" output.txt | wc -l
	wc -l symbolic.txt
	wc -l input.txt
	grep '^1 ' symbols.txt | wc -l
	grep '^2 ' symbols.txt | wc -l
	grep '^3 ' symbols.txt | wc -l
	grep '^4 ' symbols.txt | wc -l

human:
	bash human.sh 1 > human.txt
	bash human.sh 2 >> human.txt
	bash human.sh 3 >> human.txt
	bash human.sh 4 >> human.txt
