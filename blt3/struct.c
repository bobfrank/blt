#include <stdio.h>

typedef struct {
    const char* fname;
    const char* lname;
} C;

int main()
{
    C c;
    c.fname = "hello";
    c.lname = "test";

    C* c2 = (C*)malloc(sizeof(C));
    c2->fname = "test1";
    c2->lname = "ahh";
    free(c2);
}