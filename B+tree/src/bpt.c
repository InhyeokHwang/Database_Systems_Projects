#include "bpt.h"

H_P * hp;

page * rt = NULL; //root is declared as global

int fd = -1; //fd is declared as global

int binary_search_for_internal(page * p, int first, int last, int64_t key){
	int mid = (first + last)/2;
	while(first <= last){
		if(p->b_f[mid].key < key){
			first = mid + 1;
		}else if(p->b_f[mid].key == key){
			return mid;
			//key found at mid
		}else{
			last = mid - 1;
		}
		mid = (first + last)/2;
	}
	if(first > last){
		return last; //not found
	}
}

void print_tree(){ //function used for checking tree leaves
	H_P * hp = load_header(0);
	page * curr = load_page(hp->rpo);

	while(!curr->is_leaf){
		curr = load_page(curr->next_offset);
	}
	while(1){
		printf("printing leaf..");
		for(int i = 0; i < curr->num_of_keys; i++){
			printf("%ld ", curr->records[i].key);
		}
		printf("\n");
		if(curr->next_offset == 0){
			break;
		}
		curr = load_page(curr->next_offset);
	}
}


H_P * load_header(off_t off) { //return header page structure pointer
    H_P * newhp = (H_P*)calloc(1, sizeof(H_P));
    if (sizeof(H_P) > pread(fd, newhp, sizeof(H_P), 0)) {

        return NULL;
    }
    return newhp;
}


page * load_page(off_t off) { //return page structure pointer
    page* load = (page*)calloc(1, sizeof(page));
    //if (off % sizeof(page) != 0) printf("load fail : page offset error\n");
    if (sizeof(page) > pread(fd, load, sizeof(page), off)) {

        return NULL;
    }
    return load;
}

int open_table(char * pathname) {//if file exists, open existing file, else make new file
    fd = open(pathname, O_RDWR | O_CREAT | O_EXCL | O_SYNC  , 0775);
    hp = (H_P *)calloc(1, sizeof(H_P));
    if (fd > 0) { //make new file
        //printf("New File created\n");
        hp->fpo = 0;
        hp->num_of_pages = 1;
        hp->rpo = 0;
        pwrite(fd, hp, sizeof(H_P), 0);
        free(hp);
        hp = load_header(0);
        return 0;
    }
    fd = open(pathname, O_RDWR|O_SYNC);
    if (fd > 0) { //existing file
        //printf("Read Existed File\n");
        if (sizeof(H_P) > pread(fd, hp, sizeof(H_P), 0)) {
            return -1;
        }
        off_t r_o = hp->rpo;
        rt = load_page(r_o);
        return 0;
    }
    else return -1;
}

void reset(off_t off) {//if free pages don't exist any more
    page * reset;
    reset = (page*)calloc(1, sizeof(page)); //memory allocation of new page
    reset->parent_page_offset = 0;
    reset->is_leaf = 0;
    reset->num_of_keys = 0;
    reset->next_offset = 0;
    pwrite(fd, reset, sizeof(page), off);
    free(reset);
    return;
}

void freetouse(off_t fpo) { //initialize page before using this page
    page * reset;
    reset = load_page(fpo);
    reset->parent_page_offset = 0;
    reset->is_leaf = 0;
    reset->num_of_keys = 0;
    reset->next_offset = 0;
    pwrite(fd, reset, sizeof(page), fpo);
    free(reset);
    return;
}

void usetofree(off_t wbf) {
    page * utf = load_page(wbf);
    utf->parent_page_offset = hp->fpo;
    utf->is_leaf = 0;
    utf->num_of_keys = 0;
    utf->next_offset = 0;
    pwrite(fd, utf, sizeof(page), wbf);
    free(utf);
    hp->fpo = wbf;
    pwrite(fd, hp, sizeof(hp), 0);
    free(hp);
    hp = load_header(0);
    return;
}

off_t new_page() {
    off_t newp;
    page * np;
    off_t prev;
    if (hp->fpo != 0) { //if there is remaining free page
        newp = hp->fpo;
        np = load_page(newp); 
        hp->fpo = np->parent_page_offset;
        pwrite(fd, hp, sizeof(hp), 0);
        free(hp);
        hp = load_header(0);
        free(np);
        freetouse(newp); //initialize page before using
        return newp;
    }
    //change previous offset to 0 is needed
    newp = lseek(fd, 0, SEEK_END);
    //if (newp % sizeof(page) != 0) printf("new page made error : file size error\n");
    reset(newp);
    hp->num_of_pages++;
    pwrite(fd, hp, sizeof(H_P), 0);
    free(hp);
    hp = load_header(0);
    return newp;
}

off_t find_leaf(int64_t key) {
    int i = 0;
    page * p;
    off_t loc = hp->rpo;

    //printf("left = %ld, key = %ld, right = %ld, is_leaf = %d, now_root = %ld\n", rt->next_offset, 
    //  rt->b_f[0].key, rt->b_f[0].p_offset, rt->is_leaf, hp->rpo);

    if (rt == NULL) {
        //printf("Empty tree.\n");
        return 0;
    }
    p = load_page(loc);

    while (!p->is_leaf) { //goes down until leaf page appear
        i = 0;

        i = binary_search_for_internal(p, i, p->num_of_keys-1, key);

        if (i == -1) loc = p->next_offset; //one more page number
        else
            loc = p->b_f[i].p_offset;
        //if (loc == 0)
        // return NULL;

        free(p);
        p = load_page(loc); //goes down one level

    }

    free(p);
    return loc; //return leaf page offset

}

char * db_find(int64_t key) {
    char * value = (char*)malloc(sizeof(char) * 120);
    int i = 0;
    off_t fin = find_leaf(key);
    if (fin == 0) { //case of empty tree
        return NULL;
    }
    page * p = load_page(fin); //load leaf page

    for (; i < p->num_of_keys; i++) {
        if (p->records[i].key == key) break; //found
    }
    if (i == p->num_of_keys) { //not found
        free(p);
        return NULL;
    }
    else { //i is the index of leaf page where the key exists
        strcpy(value, p->records[i].value);
        free(p);
        return value;
    }
}

int cut(int length) { //splitting
    if (length % 2 == 0)
        return length / 2;
    else
        return length / 2 + 1;
}



void start_new_file(record rec) {

    page * root;
    off_t ro; //root offset
    ro = new_page(); //root page's offset
    rt = load_page(ro); //bring root page
    hp->rpo = ro;
    pwrite(fd, hp, sizeof(H_P), 0);
    free(hp);
    hp = load_header(0);
    rt->num_of_keys = 1;
    rt->is_leaf = 1;
    rt->records[0] = rec;
    pwrite(fd, rt, sizeof(page), hp->rpo);
    free(rt);
    rt = load_page(hp->rpo);
    //printf("new file is made\n");
}

int db_insert(int64_t key, char * value) {

    record nr;
    nr.key = key;
    strcpy(nr.value, value);
    if (rt == NULL) {
        start_new_file(nr);
        return 0;
    }

    int dupcheck = -1;

	off_t leaf = find_leaf(key); //leaf page offset of the key
	page * leafp = load_page(leaf); //load leaf page

	int i = 0;
	if(leaf == 0){ //the case of empty tree
		dupcheck = -1;
	}else{ //if it is not empty tree
		for(; i < leafp->num_of_keys; i++){
			if(leafp->records[i].key == key){
				dupcheck = 1;
				break; //found
			}
		}
	}

    if (dupcheck == 1) { //if it exists
        return -1;
    }

	//if it doesn't exist
    if (leafp->num_of_keys < LEAF_MAX) { 
        insert_into_leaf(leaf, nr); //just insert
        free(leafp);
        return 0;
    }
	
	int check = key_rotation_for_insert(leaf, nr);//if key rotation happens return 1 else return 0

	if(check == 0){//if there is no space for both sides
		insert_into_leaf_as(leaf, nr); 
	}
    free(leafp);
    //why double free?
    return 0;

}

int key_rotation_for_insert(off_t leaf, record inst){ //key rotation when full
	record * temp;
	int insertion_index, i, j, k, k_prime_index;
	page * currleaf = load_page(leaf); //leaf page
	page * parpage = load_page(currleaf->parent_page_offset); //parent page
	page * nborpage; //neighbor page
	off_t nboroff;

	if(leaf == hp->rpo){ //if leaf is root, split has to occur so just return
		return 0;
	}
	//have to move reocrds to temp
	temp = (record *)calloc(LEAF_MAX + 1, sizeof(record));
	if(temp == NULL){
		perror("Temporary records array");
		exit(EXIT_FAILURE);
	}
	insertion_index = 0;

	while(insertion_index < LEAF_MAX && currleaf->records[insertion_index].key < inst.key){
		insertion_index++; //index for insertion
	}
	for(i = 0, j = 0; i < currleaf->num_of_keys; i++, j++){
		if(j == insertion_index) j++; //has to move records to temp, jump insertion_index 
		temp[j] = currleaf->records[i];
	}
	temp[insertion_index] = inst; //insert inst to insertion_index

	//selecting k for k_prim_index
	if(parpage->next_offset == leaf){ //if leaf is leftmost
		k = -1;
	}else{
		for(k = 0; k < parpage->num_of_keys; k++){
			if(parpage->b_f[k].p_offset == leaf) break;
		}
	}

	if(parpage->next_offset != leaf){ //key rotation to left(it is not leftmost leaf)
		k_prime_index = k;

		if(k_prime_index == 0){ //if currpage is right page to the leftmost page
			nboroff = parpage->next_offset;
		}else{
			nboroff = parpage->b_f[k_prime_index-1].p_offset;
		}

		nborpage = load_page(nboroff);

		if(nborpage->num_of_keys < LEAF_MAX){ //if there is space for left neighbor
			//printf("\nnborpage num of keys %d\n", nborpage->num_of_keys);
			//printf("\ntemp[0] = %d, temp[1] = %d \n", temp[0].key, temp[1].key);
			parpage->b_f[k_prime_index].key = temp[1].key; //update parent's k_prime
			nborpage->records[nborpage->num_of_keys] = temp[0];//give its left neighbor the smallest record
			nborpage->num_of_keys++;
			for(i = currleaf->num_of_keys; i > 0; i--){
				currleaf->records[i-1] = temp[i];//update records
			}
			for(i = currleaf->num_of_keys; i < LEAF_MAX; i++){
				currleaf->records[i].key = 0; //initialize index after num_of_keys
			}
			pwrite(fd, nborpage, sizeof(page), nboroff);
			pwrite(fd, currleaf, sizeof(page), leaf);
			pwrite(fd, parpage, sizeof(page), currleaf->parent_page_offset);
			if(currleaf->parent_page_offset == hp->rpo){ //if parent is root
				free(rt);
				rt = load_page(currleaf->parent_page_offset);
			}
			//printf("\nkey rotation to left\n");
			free(temp);
			free(currleaf);
			free(parpage);
			free(nborpage);
			return 1; //return 1 when successfully key rotated
		}
	}
	if(currleaf->next_offset != 0){ //key rotation to right(it is not right most leaf)
		k_prime_index = k + 1;

		nboroff = parpage->b_f[k_prime_index].p_offset; //right neighbor
		nborpage = load_page(nboroff);

		if(nborpage->num_of_keys < LEAF_MAX){ //if there is space for right neighbor
			for(i = nborpage->num_of_keys; i>0; i --){
				nborpage->records[i] = nborpage->records[i-1]; //push right
			}
			nborpage->records[0] = temp[currleaf->num_of_keys];//move largest value in currleaf to smallest value for right neighbor
			nborpage->num_of_keys++;
			parpage->b_f[k_prime_index].key = nborpage->records[0].key;//update parent key
			for(i = 0; i < currleaf->num_of_keys; i++){
				currleaf->records[i] = temp[i];//update records with sorted temp
			}
			for(i = currleaf->num_of_keys; i < LEAF_MAX; i++){
				currleaf->records[i].key = 0; //initialize index after num_of_keys
			}
			pwrite(fd, nborpage, sizeof(page), nboroff);
			pwrite(fd, currleaf, sizeof(page), leaf);
			pwrite(fd, parpage, sizeof(page), currleaf->parent_page_offset);
			if(currleaf->parent_page_offset == hp->rpo){ //if parent is root
				free(rt);
				rt = load_page(currleaf->parent_page_offset);
			}

			//printf("\nkey rotation to right\n");
			free(temp);
			free(currleaf);
			free(parpage);
			free(nborpage);
			return 1;//return 1 when successfully key rotated
		}
	}
	free(temp);
	free(currleaf);
	free(parpage);
	free(nborpage);
	return 0; //key rotation didn't occur
}

off_t insert_into_leaf(off_t leaf, record inst) { //just inserting into leaf

    page * p = load_page(leaf); //load leaf page
    //if (p->is_leaf == 0) printf("iil error : it is not leaf page\n");
    int i, insertion_point;
    insertion_point = 0;
    while (insertion_point < p->num_of_keys && p->records[insertion_point].key < inst.key) {
        insertion_point++; //index for record to be inserted
    }
    for (i = p->num_of_keys; i > insertion_point; i--) {
        p->records[i] = p->records[i - 1]; //push right
    }
    p->records[insertion_point] = inst; //insert record
    p->num_of_keys++; 
    pwrite(fd, p, sizeof(page), leaf);
    //printf("insertion %ld is complete %d, %ld\n", inst.key, p->num_of_keys, leaf);
    free(p);
    return leaf;
}


off_t insert_into_leaf_as(off_t leaf, record inst) { //split
    off_t new_leaf; 
    record * temp;
    int insertion_index, split, i, j;
    int64_t new_key;
    new_leaf = new_page();  //new leaf page
    //printf("\n%ld is new_leaf offset\n\n", new_leaf);
    page * nl = load_page(new_leaf);
    nl->is_leaf = 1;
    temp = (record *)calloc(LEAF_MAX + 1, sizeof(record));
    if (temp == NULL) {
        perror("Temporary records array");
        exit(EXIT_FAILURE);
    }
    insertion_index = 0;
    page * ol = load_page(leaf); //old leaf
    while (insertion_index < LEAF_MAX && ol->records[insertion_index].key < inst.key) {
        insertion_index++;//index for record to be inserted
    }
    for (i = 0, j = 0; i < ol->num_of_keys; i++, j++) {
        if (j == insertion_index) j++; //move records to temp, jump insertion_index
        temp[j] = ol->records[i];
    }
    temp[insertion_index] = inst; //insert record
    ol->num_of_keys = 0; //set zero and increase as you move each record
    split = cut(LEAF_MAX); //split

    for (i = 0; i < split; i++) { //index below split goes to old leaf
        ol->records[i] = temp[i];
        ol->num_of_keys++;
    }

    for (i = split, j = 0; i < LEAF_MAX + 1; i++, j++) {//index after or equal to split oges to new leaf
        nl->records[j] = temp[i];
        nl->num_of_keys++;
    }

    free(temp);

    nl->next_offset = ol->next_offset; //update right sibling page number
    ol->next_offset = new_leaf;

    for (i = ol->num_of_keys; i < LEAF_MAX; i++) {
        ol->records[i].key = 0; //after splitting, initialize index after num_of_keys
        //strcpy(ol->records[i].value, NULL);
    }

    for (i = nl->num_of_keys; i < LEAF_MAX; i++) {
        nl->records[i].key = 0;//after splitting, initialize index after num_of_keys
        //strcpy(nl->records[i].value, NULL);
    }

    nl->parent_page_offset = ol->parent_page_offset;//parent page offset is equal
    new_key = nl->records[0].key;//parent page key is updated to smallest record of new leaf

    pwrite(fd, nl, sizeof(page), new_leaf);
    pwrite(fd, ol, sizeof(page), leaf);
    free(ol);
    free(nl);
    //printf("split_leaf is complete\n");

    return insert_into_parent(leaf, new_key, new_leaf);

}

off_t insert_into_parent(off_t old, int64_t key, off_t newp) {

    int left_index;
    off_t bumo;
    page * left;
    left = load_page(old);

    bumo = left->parent_page_offset;
    free(left);

    if (bumo == 0) //parent page offset is 0 so it is root
        return insert_into_new_root(old, key, newp); //make new root

    left_index = get_left_index(old);

    page * parent = load_page(bumo);
    //printf("\nbumo is %ld\n", bumo);
    if (parent->num_of_keys < INTERNAL_MAX) { //parent's split didn't occur
        free(parent);
        //printf("\nuntil here is ok\n");
        return insert_into_internal(bumo, left_index, key, newp);
    }
    free(parent);
    return insert_into_internal_as(bumo, left_index, key, newp);//parent's split occur
}

int get_left_index(off_t left) {
    page * child = load_page(left);
    off_t po = child->parent_page_offset;
    free(child);
    page * parent = load_page(po);
    int i = 0;
    if (left == parent->next_offset) return -1; //if it is one more page number
    for (; i < parent->num_of_keys; i++) {
        if (parent->b_f[i].p_offset == left) break; //find index i for left child
    }

    if (i == parent->num_of_keys) { //not found, error
        free(parent);
        return -10;
    }
    free(parent);
    return i; //return index for left child
}

off_t insert_into_new_root(off_t old, int64_t key, off_t newp) {

    off_t new_root;
    new_root = new_page(); //new root
    page * nr = load_page(new_root);
    nr->b_f[0].key = key;
    nr->next_offset = old; //one more page number(left)
    nr->b_f[0].p_offset = newp; //right
    nr->num_of_keys++;
    //printf("key = %ld, old = %ld, new = %ld, nok = %d, nr = %ld\n", key, old, newp, 
    //  nr->num_of_keys, new_root);
    page * left = load_page(old);
    page * right = load_page(newp);
    left->parent_page_offset = new_root; //update parent_page_offset to new root
    right->parent_page_offset = new_root;
    pwrite(fd, nr, sizeof(page), new_root);
    pwrite(fd, left, sizeof(page), old);
    pwrite(fd, right, sizeof(page), newp);
    free(nr);
    nr = load_page(new_root);
    rt = nr; //change root
    hp->rpo = new_root; //change header page's root pointer
    pwrite(fd, hp, sizeof(H_P), 0);
    free(hp);
    hp = load_header(0);
    free(left);
    free(right);
    return new_root;

}

off_t insert_into_internal(off_t bumo, int left_index, int64_t key, off_t newp) {

    page * parent = load_page(bumo);
    int i;

    for (i = parent->num_of_keys; i > left_index + 1; i--) {
        parent->b_f[i] = parent->b_f[i - 1];//push records
    }
    parent->b_f[left_index + 1].key = key; //key that came from child
    parent->b_f[left_index + 1].p_offset = newp; //splitted child
    parent->num_of_keys++;
    pwrite(fd, parent, sizeof(page), bumo);
    free(parent);
    if (bumo == hp->rpo) { //if parent is root
        free(rt);
        rt = load_page(bumo);
        //printf("\nrt->numofkeys%lld\n", rt->num_of_keys);

    }
    return hp->rpo;
}

off_t insert_into_internal_as(off_t bumo, int left_index, int64_t key, off_t newp) { //split in parent page

    int i, j, split;
    int64_t k_prime;
    off_t new_p, child;
    I_R * temp;

    temp = (I_R *)calloc(INTERNAL_MAX + 1, sizeof(I_R));

    page * old_parent = load_page(bumo);

    for (i = 0, j = 0; i < old_parent->num_of_keys; i++, j++) {
        if (j == left_index + 1) j++;
        temp[j] = old_parent->b_f[i];
    }

    temp[left_index + 1].key = key; //new key that came from child
    temp[left_index + 1].p_offset = newp; //splitted child

    split = cut(INTERNAL_MAX);
    new_p = new_page();
    page * new_parent = load_page(new_p);
    old_parent->num_of_keys = 0;
    for (i = 0; i < split; i++) {
        old_parent->b_f[i] = temp[i];
        old_parent->num_of_keys++;
    }
    k_prime = temp[i].key; //key for grandparent
    new_parent->next_offset = temp[i].p_offset; //splitted parent's one more page number
    for (++i, j = 0; i < INTERNAL_MAX + 1; i++, j++) {//update splitted parent
        new_parent->b_f[j] = temp[i];
        new_parent->num_of_keys++;
    }

    new_parent->parent_page_offset = old_parent->parent_page_offset;
    page * nn;
    nn = load_page(new_parent->next_offset);
    nn->parent_page_offset = new_p;//update parent for first child
    pwrite(fd, nn, sizeof(page), new_parent->next_offset);
    free(nn);
    for (i = 0; i < new_parent->num_of_keys; i++) { //update rest of the child
        child = new_parent->b_f[i].p_offset;
        page * ch = load_page(child);
        ch->parent_page_offset = new_p;
        pwrite(fd, ch, sizeof(page), child);
        free(ch);
    }

    pwrite(fd, old_parent, sizeof(page), bumo);
    pwrite(fd, new_parent, sizeof(page), new_p);
    free(old_parent);
    free(new_parent);
    free(temp);
    //printf("split internal is complete\n");
    return insert_into_parent(bumo, k_prime, new_p);
}

int db_delete(int64_t key) {
	int check = -1;
	off_t deloff = find_leaf(key); //get offset where the key exists
	page * leafp = load_page(deloff);
	
    if (rt == NULL || rt->num_of_keys == 0) {  
        //printf("root is empty\n");
        return -1;
    }

	int i = 0;
	for(; i < leafp->num_of_keys; i++){
		if(leafp->records[i].key == key){
			check = 1;
			break; //found
		}
	}
    

    if (check== -1) { //if key doesn't exist
        //printf("There are no key to delete\n");
        return -1;
    }
    
    delete_entry(key, deloff);
    return 0;

}//fin

void delete_entry(int64_t key, off_t deloff) {

    remove_entry_from_page(key, deloff); //remove key

    if (deloff == hp->rpo) { //if it is root
        adjust_root(deloff);
        return;
    }
    page * not_enough = load_page(deloff);
    //int check = not_enough->is_leaf ? cut(LEAF_MAX) : cut(INTERNAL_MAX);
    if (not_enough->num_of_keys > 0){ //if there exists at least one key
      free(not_enough);
      //printf("just delete\n");
      return;  
    } 

	//merge or key rotation
    int neighbor_index, k_prime_index;
    off_t neighbor_offset, parent_offset;
    int64_t k_prime;
    parent_offset = not_enough->parent_page_offset;
    page * parent = load_page(parent_offset);

    if (parent->next_offset == deloff) { //if it is parent's one more page number(leftmost)
        neighbor_index = -2;
        neighbor_offset = parent->b_f[0].p_offset; //right neighbor offset
        k_prime = parent->b_f[0].key;
        k_prime_index = 0;
    }
    else if(parent->b_f[0].p_offset == deloff) { //if it is second left child 
        neighbor_index = -1;
        neighbor_offset = parent->next_offset; //left most page is neighbor
        k_prime_index = 0;
        k_prime = parent->b_f[0].key;
    }
    else {
        int i;

        for (i = 0; i <= parent->num_of_keys; i++) //it starts from i = 1 since i = 0 is handled 
            if (parent->b_f[i].p_offset == deloff) break;
        neighbor_index = i - 1;
        neighbor_offset = parent->b_f[i - 1].p_offset; //left page offset
        k_prime_index = i;
        k_prime = parent->b_f[i].key;
    }

    page * neighbor = load_page(neighbor_offset);
    int max = not_enough->is_leaf ? LEAF_MAX : INTERNAL_MAX - 1;

    //printf("%d %d\n",why, max);
    if (neighbor->num_of_keys <= max) { //if neighbor->num_of_keys is less or equal to max
        free(not_enough);
        free(parent);
        free(neighbor);
        coalesce_pages(deloff, neighbor_index, neighbor_offset, parent_offset, k_prime);//merging with empty page(just freeing empty page and reconstruct)
    }else {
        free(not_enough);
        free(parent);
        free(neighbor);
        redistribute_pages(deloff, neighbor_index, neighbor_offset, parent_offset, k_prime, k_prime_index);//may occur in 'internal page' only to postpone merging, but this happens when neighbor has Internal Max value of keys so may not happen very often. I left this part out just to maintain the overall structure of the original code.

    }

    return;

}
void redistribute_pages(off_t need_more, int nbor_index, off_t nbor_off, off_t par_off, int64_t k_prime, int k_prime_index) {
    
    page *need, *nbor, *parent;
    int i;
    need = load_page(need_more); //page with not enough keys
    nbor = load_page(nbor_off); //neighbor page
    parent = load_page(par_off); //parent page
    if (nbor_index != -2) { //need is not leftmost page
        
        if (!need->is_leaf) { 
            //printf("redis average interal\n");
            for (i = need->num_of_keys; i > 0; i--)
                need->b_f[i] = need->b_f[i - 1]; //push need
            
            need->b_f[0].key = k_prime; //bring parent key
            need->b_f[0].p_offset = need->next_offset; //move left most offset to right
            need->next_offset = nbor->b_f[nbor->num_of_keys - 1].p_offset;
            page * child = load_page(need->next_offset);
            child->parent_page_offset = need_more;//update parent for moved child
            pwrite(fd, child, sizeof(page), need->next_offset);
            free(child);
            parent->b_f[k_prime_index].key = nbor->b_f[nbor->num_of_keys - 1].key;
            
        }
        else { //modified code don't need the part for leaf since redistribution may only occur in internal page
            //printf("redis average leaf\n");
            for (i = need->num_of_keys; i > 0; i--){
                need->records[i] = need->records[i - 1];
            }
            need->records[0] = nbor->records[nbor->num_of_keys - 1];
            nbor->records[nbor->num_of_keys - 1].key = 0;
            parent->b_f[k_prime_index].key = need->records[0].key;
        }

    }
    else { //need is leftmost page 
        //
        if (need->is_leaf) { //modified code don't need the part for leaf since redistribution may only occur in internal page
            //printf("redis leftmost leaf\n");
            need->records[need->num_of_keys] = nbor->records[0];
            for (i = 0; i < nbor->num_of_keys - 1; i++)
                nbor->records[i] = nbor->records[i + 1];
            parent->b_f[k_prime_index].key = nbor->records[0].key;
            
           
        }
        else {
            //printf("redis leftmost internal\n");
            need->b_f[need->num_of_keys].key = k_prime; //bring parent's key to need
            need->b_f[need->num_of_keys].p_offset = nbor->next_offset;//move nbor's one more page number to need
            page * child = load_page(need->b_f[need->num_of_keys].p_offset);
            child->parent_page_offset = need_more; //update child's parent
            pwrite(fd, child, sizeof(page), need->b_f[need->num_of_keys].p_offset);
            free(child);
            
            parent->b_f[k_prime_index].key = nbor->b_f[0].key; //neighbor's key goes up
            nbor->next_offset = nbor->b_f[0].p_offset; //update neighbor's one more page number
            for (i = 0; i < nbor->num_of_keys - 1 ; i++)
                nbor->b_f[i] = nbor->b_f[i + 1]; //pull neighbor's records
            
        }
    }
    nbor->num_of_keys--;
    need->num_of_keys++;
    pwrite(fd, parent, sizeof(page), par_off);
    pwrite(fd, nbor, sizeof(page), nbor_off);
    pwrite(fd, need, sizeof(page), need_more);
    free(parent); free(nbor); free(need);
    return;
}

void coalesce_pages(off_t will_be_coal, int nbor_index, off_t nbor_off, off_t par_off, int64_t k_prime) { //merging with empty page
    
    page *wbc, *nbor, *parent;
    off_t newp, wbf;

    if (nbor_index == -2) { //if it is one more page number
        //printf("leftmost\n");
        wbc = load_page(nbor_off); nbor = load_page(will_be_coal); parent = load_page(par_off);
        newp = will_be_coal; wbf = nbor_off;
    }
    else { //if it is not one more page number
        wbc = load_page(will_be_coal); nbor = load_page(nbor_off); parent = load_page(par_off);
        newp = nbor_off; wbf = will_be_coal;
    }

    int point = nbor->num_of_keys; //start index when merging
    int le = wbc->num_of_keys;
    int i, j;
    if (!wbc->is_leaf) { //if it is not leaf
        //printf("coal internal\n");
        nbor->b_f[point].key = k_prime;//bring parent's key
        nbor->b_f[point].p_offset = wbc->next_offset; //bring wbc's one more page number
        nbor->num_of_keys++;

        for (i = point + 1, j = 0; j < le; i++, j++) {
            nbor->b_f[i] = wbc->b_f[j]; //move records
            nbor->num_of_keys++;
            wbc->num_of_keys--;
        }

        for (i = point; i < nbor->num_of_keys; i++) {
            page * child = load_page(nbor->b_f[i].p_offset);
            child->parent_page_offset = newp; //update parent for children
            pwrite(fd, child, sizeof(page), nbor->b_f[i].p_offset);
            free(child);
        }

    }
    else {//if it is leaf
        //printf("coal leaf\n");
        int range = wbc->num_of_keys; 
        for (i = point, j = 0; j < range; i++, j++) {
            
            nbor->records[i] = wbc->records[j];
            nbor->num_of_keys++;
            wbc->num_of_keys--;
        }
        nbor->next_offset = wbc->next_offset;
    }
    pwrite(fd, nbor, sizeof(page), newp);
    
    delete_entry(k_prime, par_off); //recursively goes up by one level
    free(wbc);
    usetofree(wbf); //reset page(make it free page)
    free(nbor);
    free(parent);
    return;

}//fin

void adjust_root(off_t deloff) {

    if (rt->num_of_keys > 0)  //root is not empty
        return;
    if (!rt->is_leaf) { //if root is empty and is not leaf
        off_t nr = rt->next_offset;
        page * nroot = load_page(nr);
        nroot->parent_page_offset = 0;
        usetofree(hp->rpo); //make original root to free page
        hp->rpo = nr; //make first child root
        pwrite(fd, hp, sizeof(H_P), 0);
        free(hp);
        hp = load_header(0);
        
        pwrite(fd, nroot, sizeof(page), nr);
        free(nroot);
        free(rt);
        rt = load_page(nr);

        return;
    }
    else { //root is leaf and is empty so root has to be deleted
        free(rt);
        rt = NULL;
        usetofree(hp->rpo); //delete root and make it free page
        hp->rpo = 0;
        pwrite(fd, hp, sizeof(hp), 0); //update header page
        free(hp);
        hp = load_header(0);
        return;
    }
}//fin

void remove_entry_from_page(int64_t key, off_t deloff) {
    
    int i = 0;
    page * lp = load_page(deloff); //load page with key that has to be deleted
    if (lp->is_leaf) { //if it is leaf page
        //printf("remove leaf key %ld\n", key);
        while (lp->records[i].key != key)
            i++; //index of key that has to be deleted 

        for (++i; i < lp->num_of_keys; i++)
            lp->records[i - 1] = lp->records[i]; //pull index
        lp->num_of_keys--;
        pwrite(fd, lp, sizeof(page), deloff);
        if (deloff == hp->rpo) { //if it is leaf and root
            free(lp);
            free(rt);
            rt = load_page(deloff); //update root
            return;
        }
        
        free(lp);
        return;
    }
    else { //if it is not leaf page
        //printf("remove interanl key %ld\n", key);
        while (lp->b_f[i].key != key)
            i++; //index of key to be deleted
        for (++i; i < lp->num_of_keys; i++)
            lp->b_f[i - 1] = lp->b_f[i]; //pull index
        lp->num_of_keys--;
        pwrite(fd, lp, sizeof(page), deloff);
        if (deloff == hp->rpo) { //if it is root page
            free(lp);
            free(rt);
            rt = load_page(deloff); //update root
            return;
        }
        
        free(lp);
        return;
    }
    
}//fin






