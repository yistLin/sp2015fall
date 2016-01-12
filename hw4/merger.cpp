#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <algorithm>
using namespace std;

pthread_mutex_t lock;

struct multi_arr {
    vector<int> *v1, *v2;
};

void divider(vector<int>& allElem, vector< vector<int> >& partElem, int n, int seg_len) {
    vector<int> row;
    int i;
    for (i=0; i<(n-1); i++) {
        row.clear();
        row.insert(row.begin(), allElem.begin()+(i*seg_len), allElem.begin()+((i+1)*seg_len));
        partElem.push_back(row);
    }
    row.clear();
    row.insert(row.begin(), allElem.begin()+(i*seg_len), allElem.end());
    partElem.push_back(row);
}

void* lsorter(void *arg) {
    vector<int>* v = (vector<int>*)arg;

    pthread_mutex_lock(&lock);
    printf("Handling elements:\n%d", v->at(0));
    for (int i=1; i<v->size(); i++)
        printf(" %d", v->at(i));
    printf("\n");
    pthread_mutex_unlock(&lock);

    sort((*v).begin(), (*v).end());
    printf("sorted %d elements.\n", v->size());
    return NULL;
}

void sorter(vector< vector<int> >& v, int n) {
    int i = 0;
    pthread_t tid[10000];
    while (i < n) {
        if (pthread_create( &(tid[i]), NULL, &lsorter, (void *)&v[i] ) != 0) {
            fputs("cannot create thread", stderr);
            exit(0);
        }
        i++;
    }
    for (i = 0; i < n; i++)
        pthread_join(tid[i], NULL);
}

multi_arr* wrapper(vector<int>* v1, vector<int>* v2) {
    multi_arr* twoArr = new multi_arr();
    twoArr->v1 = v1;
    twoArr->v2 = v2;
    return twoArr;
}

void* lmerger(void* arg) {
    multi_arr* twoArr = (multi_arr*)arg;
    vector<int>* v1 = twoArr->v1;
    vector<int>* v2 = twoArr->v2;

    pthread_mutex_lock(&lock);
    printf("Handling elements:\n%d", v1->at(0));
    for (int i=1; i<v1->size(); i++)
        printf(" %d", v1->at(i));
    for (int i=0; i<v2->size(); i++)
        printf(" %d", v2->at(i));
    printf("\n");
    pthread_mutex_unlock(&lock);

    vector<int> tmp;
    int lpr = 0, rpr = 0, dup = 0;
    int lsize = v1->size(), rsize = v2->size();
    while (lpr < lsize && rpr < rsize) {
        if ((*v1)[lpr] < (*v2)[rpr])
            tmp.push_back((*v1)[lpr++]);
        else if ((*v1)[lpr] == (*v2)[rpr]) {
            dup++;
            tmp.push_back((*v1)[lpr++]);
        }
        else
            tmp.push_back((*v2)[rpr++]);
    }
    if (lpr == lsize)
        tmp.insert(tmp.end(), (v2->begin())+rpr, (v2->end()));
    else if (rpr == rsize)
        tmp.insert(tmp.end(), (v1->begin())+lpr, (v1->end()));
    (*v1) = tmp;
    v2->clear();
    printf("Merged %d and %d elements with %d duplicates.\n", lsize, rsize, dup);
}

vector<int>* merger(vector< vector<int> >& v) {
    int nMerger = 0;
    pthread_t tid[10000];
    vector< vector<int> > tmp;
    while (v.size() > 1) {
        nMerger = v.size() / 2;
        for (int i=0; i<nMerger; i++) {
            if (pthread_create(&(tid[i]), NULL, &lmerger, (void *)wrapper(&v[2*i],&v[(2*i)+1]) ) != 0) {
                fputs("cannot create thread", stderr);
                exit(0);
            }
        }
        for (int i=0; i<nMerger; i++)
            pthread_join(tid[i], NULL);

        tmp.clear();
        for (int i=0; i<v.size(); i+=2)
            tmp.push_back(v[i]);

        v = tmp;
    }

    vector<int>* ret = new vector<int>;
    ret->insert(ret->end(), v[0].begin(), v[0].end());
    return ret;
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "usage: ./merger [segment_len]\n");
        exit(0);
    }
    int seg_len = atoi(argv[1]);

    vector<int> allElem;

    int item, nItem;
    scanf("%d", &nItem);
    for (int i=0; i<nItem; i++) {
        scanf("%d", &item);
        allElem.push_back(item);
    }

    // count the amount of sorter
    int nSorter = (allElem.size() / seg_len) + ((allElem.size() % seg_len > 0) ? 1 : 0);

    // divide array
    vector< vector<int> > partElem;
    divider(allElem, partElem, nSorter, seg_len);

    pthread_mutex_init(&lock, NULL);

    // send to sorter for sorting
    sorter(partElem, nSorter);

    // send to merger for merging
    vector<int> sorted = *(merger(partElem));

    // print out for checking
    printf("%d", sorted[0]);
    for (int i = 1; i < sorted.size(); i++) {
        printf(" %d", sorted[i]);
    }
    printf("\n");

    pthread_mutex_destroy(&lock);

    return 0;
}
