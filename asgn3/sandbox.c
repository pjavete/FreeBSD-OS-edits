//Sandbox

#include <sys/queue.h>
#include <stdio.h>

struct str {
    TAILQ_ENTRY(str) tailq;
    int n;
};

int main() {

    TAILQ_HEAD(strq, str);

    struct strq q;
    TAILQ_INIT(&q);

    struct str data[5];

    for (int i = 0; i < 5; i++) {
        data[i].n = i;
        TAILQ_INSERT_HEAD(&q, data[i], tailq);
    }

    struct str *p;
    TAILQ_FOREACH(p, &q, tailq) {
        printf("%d\n", p->n);
    }

    return(0);
}