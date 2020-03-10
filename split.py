f = open('all_tests.txt', 'r')

tests = f.read().split('=======')[1:]

for test_id, test in enumerate(tests):
	print('TEST', test_id + 1)
	parts = test.split('--- ')[1:]

	test_part = "\n".join(parts[0].splitlines()[1:])
	answers_part = parts[2].splitlines()[1:][:-1]
	answers_part = "\n".join([line for line in answers_part if line.strip() != ''])

	test_f = open('test_data/test' + str(test_id + 1) + ".txt", 'w')
	test_f.write(test_part)
	test_f.close()

	ans_f = open('answers/answer' + str(test_id + 1) + ".txt", 'w')
	ans_f.write(answers_part)
	ans_f.close()
