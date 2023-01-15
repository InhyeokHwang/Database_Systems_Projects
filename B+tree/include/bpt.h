#ifndef __BPT_H__
#define __BPT_H__

// Uncomment the line below if you are compiling on Windows.
// #define WINDOWS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#define LEAF_MAX 31
#define INTERNAL_MAX 248

typedef struct record{
    int64_t key; //8byte
    char value[120]; //120byte
}record;

typedef struct inter_record {
    int64_t key; //8byte
    off_t p_offset; //8byte
}I_R;

typedef struct Page{
    off_t parent_page_offset; //parent page number 0~7, nex free page number if it is free page
    int is_leaf; //0 when it is internal, 1 when it is leaf 8~11
    int num_of_keys; //number of keys 12~15
    char reserved[104]; //16~127, if it is leaf then 16~119
	//1.case of leaf page 2. case of internal page
    off_t next_offset;
	//1.right sibling page number, if rightmost then 0, 120~127
	//2.one more page number, offset for left most child
	//page header


	//1.leaf page 2.internal page 
    union{
        I_R b_f[248]; //2. (4096 - 128)/(8 + 8) = 248
        record records[31]; //1. (4096 - 128)/(8 + 120) = 31
    };
}page;

typedef struct Header_Page{ //special, containing metadata
    off_t fpo; //first free page(0 if no free page left) 0~7
    off_t rpo; //root page within data file 8~15
    int64_t num_of_pages; //how many pages exist in data file, 16 ~ 23
    char reserved[4072]; //24 ~ 4095
}H_P;


extern int fd;

extern page * rt;

extern H_P * hp;
// FUNCTION PROTOTYPES.
int open_table(char * pathname);
H_P * load_header(off_t off);
page * load_page(off_t off);

void reset(off_t off);
off_t new_page();
off_t find_leaf(int64_t key);
char * db_find(int64_t key);
void freetouse(off_t fpo);
int cut(int length);
int parser();

void start_new_file(record rec);
int db_insert(int64_t key, char * value);
int key_rotation_for_insert(off_t leaf, record inst); //additional function
off_t insert_into_leaf(off_t leaf, record inst);
off_t insert_into_leaf_as(off_t leaf, record inst);
off_t insert_into_parent(off_t old, int64_t key, off_t newp);
int get_left_index(off_t left);
off_t insert_into_new_root(off_t old, int64_t key, off_t newp);
off_t insert_into_internal(off_t bumo, int left_index, int64_t key, off_t newp);
off_t insert_into_internal_as(off_t bumo, int left_index, int64_t key, off_t newp);

int db_delete(int64_t key);
void delete_entry(int64_t key, off_t deloff);
void redistribute_pages(off_t need_more, int nbor_index, off_t nbor_off, off_t par_off, int64_t k_prime, int k_prime_index);
void coalesce_pages(off_t will_be_coal, int nbor_index, off_t nbor_off, off_t par_off, int64_t k_prime);
void adjust_root(off_t deloff);
void remove_entry_from_page(int64_t key, off_t deloff);
void usetofree(off_t wbf);

int binary_search_for_internal(page * p, int first, int last, int64_t key);//additional function
void print_tree(); //additional function used for checking leaves 
#endif /* __BPT_H__*/


