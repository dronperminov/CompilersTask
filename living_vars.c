#ifdef __cplusplus
#define export exports
extern "C" {
#include <qbe/all.h>
}
#undef export
#else
#include <qbe/all.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#define DEBUG_DEF_USE

typedef struct block_info_t {
	Blk *blk;
	int index;
} block_info_t;

typedef struct node_t {
	block_info_t info;
	struct node_t *next;
} node_t;

typedef struct list_t {
	node_t *head;
	node_t *tail;
} list_t;

/******************************* WORK WITH LIST *******************************/
static void add_back_list(list_t *list, Blk *blk, int blk_index) {
	node_t *node = (node_t *) malloc(sizeof(node_t));
	node->info.blk = blk;
	node->info.index = blk_index;
	node->next = NULL;
	
	if (!list->tail)
		list->head = node;
	else
		list->tail->next = node;

	list->tail = node;
}

static block_info_t pop_list(list_t *list) {
	block_info_t info = list->head->info;
	node_t *tmp = list->head;
	list->head = list->head->next;
	free(tmp);
	return info;
}

/******************************* WORK WITH BSET *******************************/
BSet init_zeroset(uint n) {
	BSet set;
	bsinit(&set, n);
	bszero(&set);
	return set;
}

static uint bsget(BSet *set, int index) {
	return (set->t[index / NBit] >> (index % NBit)) & 1;
}

/******************************* ALGORITHM PART *******************************/
static int blk2idx(Fn *fn, Blk *block) {
	int index = 0;

	for (Blk *blk = fn->start; blk; blk = blk->link) {
		if (blk == block)
			return index;

		index++;
	}

	printf("this unreally\n");
}

static void add_all(list_t *list, Blk *blk, Fn *fn) {
	node_t *node = list->head;
	while (node && node->info.blk != blk)
		node = node->next;

	if (node)
		return;

	add_back_list(list, blk, blk2idx(fn, blk));
	
	for (int i = 0; i < blk->npred; i++) {
		add_all(list, blk->pred[i], fn);
	}	
}

static list_t init_worklist(Fn *fn) {
	list_t worklist;
	worklist.head = NULL;
	worklist.tail = NULL;

	for (Blk *blk = fn->start; blk; blk = blk->link)
		if (!blk->s1 && !blk->s2)
			add_all(&worklist, blk, fn);

	return worklist;
}

static void make_def_use(Blk *blk, BSet *def, BSet *use, Fn *fn) {
	for (int i = 0; i < blk->nins; i++) {
		int arg1 = blk->ins[i].arg[0].val;
		int arg2 = blk->ins[i].arg[1].val;
		int to = blk->ins[i].to.val;

		if (Tmp0 <= arg1 && !bsget(def, arg1))
			bsset(use, arg1);

		if (Tmp0 <= arg2 && !bsget(def, arg2))
			bsset(use, arg2);
				
		if (Tmp0 <= to)
			bsset(def, to);
	}

	int arg = blk->jmp.arg.val;
	if (Tmp0 <= arg && !bsget(def, arg))
		bsset(use, arg);
}

static void print_set(Fn *fn, BSet bset) {
	for (int i = 0; i < fn->ntmp; i++)
		if (bsget(&bset, i))
			printf(" %%%s", fn->tmp[i].name);
}

static void update_out(BSet *out_new, BSet *out, BSet *def, BSet *use, Blk *succ, Fn *fn) {
	if (!succ)
		return;

	BSet result = init_zeroset(fn->ntmp);

	int succ_index = blk2idx(fn, succ);
	bscopy(&result, &out[succ_index]);
	bsdiff(&result, &def[succ_index]);
	bsunion(&result, &use[succ_index]);
	bsunion(out_new, &result);
}

static void readfn (Fn *fn) {
	BSet out[fn->nblk];
	BSet def[fn->nblk];
	BSet use[fn->nblk];

	Blk *blk = fn->start;
	for (int i = 0; i < fn->nblk; i++) {
		out[i] = init_zeroset(fn->ntmp);
		def[i] = init_zeroset(fn->ntmp);
		use[i] = init_zeroset(fn->ntmp);
		make_def_use(blk, &def[i], &use[i], fn);
		blk = blk->link;
	}

	int changed = 1;
	
	while (changed) {
		changed = 0;
		list_t worklist = init_worklist(fn);

		while (worklist.head) {
			block_info_t info = pop_list(&worklist);

			BSet out_new = init_zeroset(fn->ntmp);
			update_out(&out_new, out, def, use, info.blk->s1, fn);
			update_out(&out_new, out, def, use, info.blk->s2, fn);

			if (!bsequal(&out_new, &out[info.index])) {
				bscopy(&out[info.index], &out_new);
				add_back_list(&worklist, info.blk, info.index);
				changed = 1;
			}
		}
	}

	int index = 0;
	int is_printed = 0;

	for (Blk *blk = fn->start; blk; blk = blk->link) {
		if (is_printed)
			printf("\n");

		printf("@%s\n", blk->name);
		printf("\tlv_out =");
		print_set(fn, out[index]);

		is_printed = 1;
		index++;
	}
}

static void readdat (Dat *dat) {
	(void) dat;
}

int main () {
	parse(stdin, "<stdin>", readdat, readfn);
	freeall();
}