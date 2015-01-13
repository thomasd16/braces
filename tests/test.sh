#!/bin/bash
cd `dirname $0`

for test in *.mustache;
do
	test=${test::-9}
	if cmp -s test-files/${test}.txt <(cat test-files/${test}.json | ../build/braces test-files/${test}.mustache)
	then
		echo "${test} test passed"
	else
		echo "${test} failed diff output betwean ${test}.txt and cat ${test}.json | ../build/braces ${test}.mustache Below"
		diff ${test}.txt <(cat test-files/${test}.json | ../build/braces test-files/${test}.mustache)
	fi
done
