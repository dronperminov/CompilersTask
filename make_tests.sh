mkdir -p tests test_data answers &&
python3 split.py && cd .. &&
gcc CompilersTask/living_vars.c -L /usr/bin/qbe/lib -lqbe -o living_vars &&
./living_vars < CompilersTask/test_data/test1.txt > CompilersTask/tests/test1.txt &&
./living_vars < CompilersTask/test_data/test2.txt > CompilersTask/tests/test2.txt &&
./living_vars < CompilersTask/test_data/test3.txt > CompilersTask/tests/test3.txt &&
./living_vars < CompilersTask/test_data/test4.txt > CompilersTask/tests/test4.txt &&
./living_vars < CompilersTask/test_data/test5.txt > CompilersTask/tests/test5.txt &&
./living_vars < CompilersTask/test_data/test6.txt > CompilersTask/tests/test6.txt &&
./living_vars < CompilersTask/test_data/test7.txt > CompilersTask/tests/test7.txt &&
./living_vars < CompilersTask/test_data/test8.txt > CompilersTask/tests/test8.txt &&
./living_vars < CompilersTask/test_data/test9.txt > CompilersTask/tests/test9.txt &&
./living_vars < CompilersTask/test_data/test10.txt > CompilersTask/tests/test10.txt &&
./living_vars < CompilersTask/test_data/test11.txt > CompilersTask/tests/test11.txt &&
./living_vars < CompilersTask/test_data/test12.txt > CompilersTask/tests/test12.txt &&
./living_vars < CompilersTask/test_data/test12.txt > CompilersTask/tests/test12.txt &&
./living_vars < CompilersTask/test_data/test13.txt > CompilersTask/tests/test13.txt &&
./living_vars < CompilersTask/test_data/test14.txt > CompilersTask/tests/test14.txt &&
./living_vars < CompilersTask/test_data/test15.txt > CompilersTask/tests/test15.txt &&
./living_vars < CompilersTask/test_data/test16.txt > CompilersTask/tests/test16.txt &&
cd CompilersTask && python3 test.py