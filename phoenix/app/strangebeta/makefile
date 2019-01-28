all: try human

try:
	# Execute spell-checking on original data
	ruby ../../../data/sb_ucd_anal.rb < ../../../data/strangebeta_user_climb_data_20180128.txt > input.txt
	# Parse the data, put in output.txt
	cat input.txt | cut -f 6 -d '|' | ./run_parse > output.txt
	# Bind a symbol to every move (for every symbol set, k1..k4), put in symbolic.txt
	ruby symbolize.rb < output.txt > symbolic.txt
	# Print counts
	grep "No parse" output.txt | wc -l
	wc -l symbolic.txt
	wc -l input.txt
	grep '^1 ' symbols.txt | wc -l
	grep '^2 ' symbols.txt | wc -l
	grep '^3 ' symbols.txt | wc -l
	grep '^4 ' symbols.txt | wc -l

human:
	# Print all four symbol-sets in human-readable form
	bash human.sh 1 > human.txt
	bash human.sh 2 >> human.txt
	bash human.sh 3 >> human.txt
	bash human.sh 4 >> human.txt
