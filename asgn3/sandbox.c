//Sandbox

#include <sys/queue.h>
#include <stdio.h>

struct str {
    TAILQ_ENTRY(str) tailq;
    int n;
};

int main() {

    TAILQ_HEAD(intq, int);

    struct intq q;
    TAILQ_INIT(&q);

    struct str data[5] = {str(1), str(2), str(3), str(4), str(5)};

    for (int i = 0; i < 5; i++) {
        TAILQ_INSERT_HEAD(&q, data[i], tailq);
    }

    struct str *p;
    TAILQ_FOREACH(p, &q, tailq) {
        printf("%d\n", p->n);
    }

    return(0);
}