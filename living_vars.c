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
static void add_to_list(list_t *list, Blk *blk, int blk_index) {
	node_t *node = (node_t *) malloc(sizeof(node_t));
	node->info.blk = blk;
	node->info.index = blk_index;
	node->next = list->head;
	
	if (!list->head)
		list->tail = node;

	list->head = node;
}

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
static list_t init_worklist(Fn *fn) {
	list_t worklist;
	worklist.head = NULL;
	worklist.tail = NULL;

	int index = 0;
	for (Blk *blk = fn->start; blk; blk = blk->link) {
		add_to_list(&worklist, blk, index);
		index++;
	}

	return worklist;
}

static void make_def_use(Blk *blk, BSet *def, BSet *use, Fn *fn) {
	for (int i = 0; i < blk->nins; i++) {
		int arg1 = blk->ins[i].arg[0].val;
		int arg2 = blk->ins[i].arg[1].val;
		int to = blk->ins[i].to.val;

		// if ((Tmp0 <= arg1 && !strcmp(fn->tmp[arg1].name, "e")) ||
		// 	(Tmp0 <= arg2 && !strcmp(fn->tmp[arg2].name, "e")) ||
		// 	(Tmp0 <= to && !strcmp(fn->tmp[to].name, "e")))
		// 	printf("got 'e' at block '%s'", blk->name);

		if (Tmp0 <= arg1 && !bsget(def, arg1)) {
			// if (!strcmp(fn->tmp[arg1].name, "e"))
			// 	printf("got 'e' at block '%s' as use arg0", blk->name);
			bsset(use, arg1);
		}

		if (Tmp0 <= arg2 && !bsget(def, arg2)) {
			// if (!strcmp(fn->tmp[arg2].name, "e"))
			// 	printf("got 'e' at block '%s' as use arg1", blk->name);

			bsset(use, arg2);
		}
				
		if (Tmp0 <= to/* && !bsget(use, arg2)*/) {
			// if (!strcmp(fn->tmp[to].name, "e"))
			// 	printf("got 'e' at block '%s' as def to", blk->name);

			bsset(def, to);
		}
	}
}

static int blk2idx(Fn *fn, Blk *block) {
	int index = 0;

	for (Blk *blk = fn->start; blk; blk = blk->link) {
		if (blk == block)
			return index;

		index++;
	}

	printf("this unreally\n");
}

static void print_set(Fn *fn, BSet bset) {
	printf("size: %d\t", bset.nt);
	for (int i = 0; i < fn->ntmp; i++)
		if (bsget(&bset, i))
			printf(" %%%s", fn->tmp[i].name);
	printf("\n");
}

static BSet update_out(BSet *out, BSet *def, BSet *use, Blk *succ, Fn *fn) {
	BSet result = init_zeroset(fn->ntmp);

	if (!succ)
		return result;

	int succ_index = blk2idx(fn, succ);

	// if (!strcmp(succ->name, "l13") || !strcmp(succ->name, "l14")) {
	// 	printf("BLOCK: %s\n", succ->name);
	// 	printf("\tOUT: ");
	// 	print_set(fn, out[succ_index]);
	// 	printf("\tDEF: ");
	// 	print_set(fn, def[succ_index]);
	// 	printf("\tUSE: ");
	// 	print_set(fn, use[succ_index]);
	// }

	bscopy(&result, &out[succ_index]);
	bsdiff(&result, &def[succ_index]);
	bsunion(&result, &use[succ_index]);

	return result;
}

static void readfn (Fn *fn) {
	BSet out[fn->nblk];
	BSet def[fn->nblk];
	BSet use[fn->nblk];

	for (int i = 0; i < fn->nblk; i++) {
		out[i] = init_zeroset(fn->ntmp);
		def[i] = init_zeroset(fn->ntmp);
		use[i] = init_zeroset(fn->ntmp);		
	}
	
	list_t worklist = init_worklist(fn);
	node_t *node = worklist.head;

	while (node) {
		make_def_use(node->info.blk, &def[node->info.index], &use[node->info.index], fn);
		node = node->next;
	}

	while (worklist.head) {
		block_info_t info = pop_list(&worklist);
		// printf("Iterate block '%s':\n", info.blk->name);		
		
		BSet out_new;
		out_new = init_zeroset(fn->ntmp);
		BSet new_s1 = update_out(out, def, use, info.blk->s1, fn);
		BSet new_s2 = update_out(out, def, use, info.blk->s2, fn);

		bsunion(&out_new, &new_s1);
		bsunion(&out_new, &new_s2);

		// if (!strcmp(info.blk->name, "l11")) {
		// 	printf("out_new:");
		// 	print_set(fn, out_new);
		// }

		if (!bsequal(&out_new, &out[info.index])) {
			bscopy(&out[info.index], &out_new);
			add_back_list(&worklist, info.blk, info.index);
		}
	}

	int index = 0;
	int is_printed = 0;

	for (Blk *blk = fn->start; blk; blk = blk->link) {
		if (is_printed)
			printf("\n");

		printf("@%s\n", blk->name);
		printf("\tlv_out =");

		for (int i = 0; i < fn->ntmp; i++)
			if (bsget(&out[index], i))
				printf(" %%%s", fn->tmp[i].name);

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