#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "conf.h"
#include "list.h"
#include "random.h"
#include "student.h"
#include "group.h"
#include "sort.h"
#include "shm.h"
#include "debug.h"

extern double TWO;
extern double THREE;
extern double FOUR;
extern int NOF_INVITES;
extern int MAX_REJECT;
extern int POP_SIZE;
extern int SIM_TIME;

void print_int(void* a) {
  printf("%d", *(int*)a);
}

void test_conf() {
  //error("Test error!\n");
  config_init();
  printf("\n\nCONF TEST\n\n");
  printf("two: %lf, three: %lf, four: %lf, nof_invites: %d, max_reject: %d, pop_size: %d, sim_time: %d\n", TWO, THREE, FOUR, NOF_INVITES, MAX_REJECT, POP_SIZE, SIM_TIME);
}

void test_list() {
  list* l = new_list();
  int a = 3;
  int b = 10;
  int c = 123;
  list_add(l, &a);
  list_add(l, &b);
  list_add(l, &c);
  assert(list_contains(l, &a));
  assert(list_contains(l, &b));
  assert(list_contains(l, &c));
  list_remove(l, &b);
  assert(!list_contains(l, &b));
  //print_list(l, print_int);
}

void test_random() {
  printf("\n\nRANDOM TEST\n\n");
  random_init();
  int tests = 1000;
  int two = 0, three = 0, four = 0;
  int total;
  for (int i = 0; i < tests; i++) {
    int r = random_int();
    //printf("%u, ",r);
    int from = -1000;
    int to = 1000;
    int range = random_int_range(from, to);
    //printf("%u, ",range);
    assert(range >= from && range <= to);
    double prob = random_zero_to_one();
    //printf("%lf\n", prob);
    assert(prob >= 0 && prob <= 1);
    int nof_elems = random_nof_elems(TWO, THREE, FOUR);
    if (nof_elems == 2) two++;
    else if (nof_elems == 3) three++;
    else four++;
  }
  total = two + three + four;
  printf("2 prob: %lf, test: %lf\n", TWO, (double)two / total);
  printf("3 prob: %lf, test: %lf\n", THREE, (double)three / total);
  printf("4 prob: %lf, test: %lf\n", FOUR, (double)four / total);
}

void test_student() {
  student* s = new_student();
  student* s1 = new_student();
  student* s2 = new_student();
  s->pid = 1;
  s1->pid = 2;
  s2->pid = 3;
  printf("\n\nSTUDENT TEST\n\n");
  print_student(s);
  printf("\n");
  print_student(s1);
  printf("\n");
  print_student(s2);
  printf("\n");
  group* g = new_group();
  group_add_student(g, s);
  group_add_student(g, s1);
  print_group(g);
  group_add_student(g, s2);
  g->leader = s2;
  print_group(g);
  printf("\nVotes: %u, %u, %u\n", s->vote, s1->vote, s2->vote);
  g->closed = TRUE;
  student_set_vote(s);
  student_set_vote(s1);
  student_set_vote(s2);
  printf("\nClosed group votes: %u, %u, %u\n", s->vote, s1->vote, s2->vote);
}

void test_shm() {
  shm_create();
  printf("\nSHARED MEMORY TEST\n");
  student* s = new_student();
  student* s1 = new_student();
  student* s2 = new_student();
  s->pid = 1;
  s1->pid = 2;
  s2->pid = 3;
  list* l = new_list();
  list_add(l, s);
  list_add(l, s1);
  list_add(l, s2);
  shm_write_students(l);
  printf("3 students have been written\n\n");
  shm_print();
  group* g = new_group();
  group_add_student(g, s);
  group_add_student(g, s1);
  group_add_student(g, s2);
  shm_close_group(g);
  printf("First and third students closed groups\n\n");
  shm_print();
  shm_delete();
}

void test_improvement_sort() {
  printf("\nIMPROVEMENT SORT TEST\n\n");
  student* self = new_student();
  int length = 5;
  student* arr[length];
  for (int i = 0; i < length;i++) {
    arr[i] = new_student();
  }
  for (int i = 0; i < length;i++) {
    printf("imp: %d, nof_elems uguale: %d, voto self: %d, voto: %d\n", student_imp(self, arr[i]), self->nof_elems == arr[i]->nof_elems, self->voto_AdE, arr[i]->voto_AdE);
  }
  printf("\n\n");
  qsort_s(arr, length, sizeof(student*), student_imp_comp, self);
  for (int i = 0; i < length;i++) {
    printf("imp: %d, nof_elems uguale: %d, voto self: %d, voto: %d\n", student_imp(self, arr[i]), self->nof_elems == arr[i]->nof_elems, self->voto_AdE, arr[i]->voto_AdE);
  }
  qsort_s(arr, length, sizeof(student*), student_vote_comp, NULL);
  printf("\nVOTE SORT\n\n");
  for (int i = 0; i < length;i++) {
    printf("%d\n", arr[i]->voto_AdE);
  }
}

int main() {
  //atexit(shm_delete);
  test_conf();
  test_list();
  test_random();
  test_student();
  test_shm();
  test_improvement_sort();
}
