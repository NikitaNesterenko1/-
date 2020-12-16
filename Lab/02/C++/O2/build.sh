#! /usr/bin/bash

CLANG=/usr/bin/clang++
GPP=/usr/bin/g++

OUT_FILE=server.out
FLAGS="--std=c++17 -pipe -lpthread"

if test -f "$CLANG"
then
	echo "Clang ��������..."
	$CLANG $FLAGS *.cpp -o $OUT_FILE

	echo "��������� ����������� ������������ ����� $OUT_FILE"
	echo ""
elif test -f "$GPP"
then
	echo "g++ ��������..."
	$GPP $FLAGS *.cpp -o $OUT_FILE

	echo "��������� ����������� ������������ ����� $OUT_FILE"
	echo ""
else
	if [[ -z "$1" ]] # ���� $1 �� ����� ��� ����
	then
		echo ""
		echo "�� ������� �����������! ������� ���� � �����������:"
		echo "$ ./build.sh /����/�/�����������"
		echo ""
	else
		$1 $FLAGS *.cpp -o $OUT_FILE
	fi
fi