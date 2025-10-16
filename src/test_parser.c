#include "../include/parser.h"
#include <stdio.h>
#include <string.h>
#define MAX 100
// make clean 
// make
// ./test

void print_test_result(char* func_name, int test_result) {
	printf("- Fonction %s : %s\n", func_name, (test_result==0)?"SUCCES":"ECHEC");
}

void print_expected_result(char* func_name, char* obtained, char* expected) {
	printf("Après %s: '%s'\nRésultat attendu: '%s'\n\n", func_name, obtained, expected);
}

int test_trim() {
	int result = 0;
	char s1[] = "        toto 123    !                  ";
	char s2[] = "titi1 2 3    ";
	char s3[] = "               tutu123.";
	char s4[] = "tata123";
	
	char t1[] = "toto 123    !";
	char t2[] = "titi1 2 3";
	char t3[] = "tutu123.";
	char t4[] = "tata123";
	
	trim(s1); trim(s2); trim(s3); trim(s4);
	
	int r1 = strcmp(s1, t1);
	int r2 = strcmp(s2, t2);
	int r3 = strcmp(s3, t3);
	int r4 = strcmp(s4, t4);
	
	if( r1 || r2 || r3 || r4) {
		result = -1;
		
		if(r1)
			print_expected_result("trim", s1, t1);
		if(r2)
			print_expected_result("trim", s2, t2);
		if(r3)
			print_expected_result("trim", s3, t3);	
		if(r4)
			print_expected_result("trim", s4, t4);	
	}
	
	return result;
}

int test_replace() {
	int result = 0;
	
	char s1[MAX] = "toto";
	char s2[MAX] = "abcd";
	
	char t1[] = "PRoto";
	char t2[] = "abcxy";
	
	int r1 = replace(s1, "t", "PR", MAX) || strcmp(s1,t1);
	int r2 = replace(s2, "d", "xyzzz", 6) || strcmp(s2,t2);
	int r3 = replace(s1, "blablabal", "", MAX);
	
	if( r1 || r2 || r3 != -1) {
		result = -1;
		
		if(r1)
			print_expected_result("replace", s1, t1);
		if(r2)
			print_expected_result("replace", s2, t2);
		if(r3)
			print_expected_result("replace", s1, "ERREUR");		
	}
	
	return result;
}

int test_clean() {
	int result = 0;
	
	char s1[] = "        to          t          o                   ";
	char s2[] = "abcd";
	
	char t1[] = " to t o ";
	char t2[] = "abcd";
	
	int r1 = clean(s1) || strcmp(s1,t1);

	int r2 = clean(s2) || strcmp(s2,t2);
	
	if( r1 || r2 ) {
		result = -1;
		
		if(r1)
			print_expected_result("clean", s1, t1);
		if(r2)
			print_expected_result("clean", s2, t2);	
	}	
	
	return result;

} 

int test_strcut() { 
	int result = 0;
	char s1[] = "chaine de caracteres a decouper en tokens";
	char* tokens1[8] = {NULL};
	char* expected1[8] = {"chaine", "de", "caracteres", "a", "decouper", "en", "tokens", NULL};
	
	int r1 = strcut(s1, ' ', tokens1, 8); // 7 tokens + NULL
	for(int i = 0; i < 8; i++) {
		if(tokens1[i] == NULL)
			continue;
		if(strcmp(tokens1[i], expected1[i]))
			printf("tokens[%d]: %s\nexpected[%d]: %s\n\n", i, tokens1[i], i, expected1[i]);
	}
	
	printf("%d\n", r1);
	
	return !(r1==7);
}

int main() {
	print_test_result("test_trim", test_trim());
	print_test_result("test_replace", test_replace());
	print_test_result("test_clean", test_clean());
	print_test_result("test_strcut", test_strcut());
	
	return 0;
}


