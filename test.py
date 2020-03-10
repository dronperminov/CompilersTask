import os

def parse_answer(path):
	f = open(path, 'r')
	blocks = dict()

	block_name = f.readline()

	while block_name:
		block_name = block_name.strip()
		variables = f.readline().strip().split()[2:]
		
		blocks[block_name] = []

		for variable in variables:
			blocks[block_name].append(variable)

		block_name = f.readline()

	f.close()
	return blocks

def check_test(test_file, correct_file):
	blocks_test = parse_answer(test_file)
	blocks_correct = parse_answer(correct_file)

	names1 = set(blocks_test.keys())
	names2 = set(blocks_correct.keys())

	if names1 != names2:
		print("Different blocks:")
		print("correct: ", names1)
		print("   test: ", names2)
		return False

	for block_name in names1:
		variables_test = blocks_test[block_name]
		variables_correct = blocks_correct[block_name]
		
		if len(variables_test) != len(variables_correct):
			print("Block '" + block_name + "'")
			print("correct: ", variables_correct)
			print("   test: ", variables_test)
			return False

	return True

for test_id in range(16):
	print('Test ' + str(test_id + 1))
	result = check_test('tests/test' + str(test_id + 1) + '.txt', 'answers/answer' + str(test_id + 1) + '.txt')

	if result:
		print('OK')
